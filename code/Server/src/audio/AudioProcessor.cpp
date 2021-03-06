/*
 * AudioProcessor.cpp
 *
 *  Created on: Mar 18, 2018
 *      Author: Jochen Alt
 */

#include <audio/AudioFile.h>
#include <iostream>
#include <string.h>

#include <stdlib.h>
#include <signal.h>
#include <chrono>
#include <unistd.h>
#include <iomanip>
#include <thread>
#include <string>
#include <unistd.h>
#include <iomanip>
#include <algorithm>

#include <basics/stringhelper.h>
#include <basics/util.h>
#include <audio/AudioProcessor.h>
#include <audio/MicrophoneInput.h>
#include <audio/Playback.h>

#include <dance/RhythmDetector.h>
#include <dance/Dancer.h>

#include <beat/BTrack.h>
#include <Configuration.h>

using namespace std;

AudioProcessor::AudioProcessor() {
	beatCallback = NULL;

	currentBeatType = NO_BEAT;
	pendingBeatType = NO_BEAT;
}

AudioProcessor::~AudioProcessor() {
	if (beatDetector != NULL)
		delete beatDetector;
}

void AudioProcessor::setup(BeatCallbackFct newBeatCallback) {
    beatCallback = newBeatCallback;
	musicDetected = false;

	// start time used for delays and output
	audioSource.setup();

	playback.setup(Configuration::getInstance().microphoneSampleRate);
	globalPlayback = true;

	// initialize beat detector
	initializeBeatDetector();
}

void AudioProcessor::initializeBeatDetector() {
	// initialize beat detector
	beatDetector = new BTrack(numInputSamples, numInputSamples*2);

}

void generateSinusoidTone(double buffer[], int bufferSize, float sampleRate, int numOfFrequencies, float tonefrequency[]) {
    for(int i=0; i<bufferSize; ++i) {
    	buffer[i] = 0;
    	for (int j = 0;j<numOfFrequencies;j++)
    		buffer[i] += 1.0/numOfFrequencies*sin( (2.f*M_PI*tonefrequency[j])/sampleRate * (float)i );
    }
}

double AudioProcessor::calibrateLatency() {
	// calibration is done by sending a sinusoid tone via the loudspeaker and
	// receiving it via microphone. The latency in between is measured.
	float measuredLatency = 0;

	float microphoneSampleRate = Configuration::getInstance().microphoneSampleRate;
	MicrophoneInput microphone;
	playback.setup(microphoneSampleRate);
	microphone.setup(microphoneSampleRate);


	// measurement is repeated if not successful
	int tries = 0;

	const float length_s = 0.150; 					// [s], length of test tone
	int bufferSize = length_s*microphoneSampleRate; // length of test tone buffer
	double buffer[bufferSize];						// buffer containing the test tone
	const int numOfTestFrequencies = 4;				// test tone consists of several frequencies in case the microphone is really poor

	float testFrequency[numOfTestFrequencies];		// test frequencies are generated by thirds starting at 440Hz
	int fftBin[numOfTestFrequencies];				// index of the fft's bin per test frequency
	const int fftLength = 512;						// input buffer size of fft
	int inputBufferSize = fftLength;				// input buffer size of microphone
	double inputBuffer[inputBufferSize];			// buffer for microphone

	// generate the test frequencies
	cout << "calibrating microphone with test frequencies ";
	for (int i = 0;i< numOfTestFrequencies;i++) {
		if (i == 0)
			testFrequency[i] = 440;	// [Hz]
		else
			testFrequency[i] =2.0*sqrt(2)*testFrequency[i-1];
		assert (testFrequency[i]  <  microphoneSampleRate/2);

		fftBin[i] = testFrequency[i]/ (microphoneSampleRate/ fftLength);

		// adapt test frequency to hit the middle of an fft bin
		testFrequency[i]= ((float)fftBin[i]) * (microphoneSampleRate / fftLength);
		cout << testFrequency[i] << "Hz" << string((i<numOfTestFrequencies-1)?string(", "):string(""));
	}
	cout << ". Be quiet." << endl;

	do {
		// sum up all test frequencies
		generateSinusoidTone(buffer, bufferSize , microphoneSampleRate, numOfTestFrequencies, testFrequency);

		// empty the alsa microphone buffer by reading
		microphone.readMicrophoneInput(inputBuffer, inputBufferSize);

		// play the test tone asynchronously
		playback.play(1.0, buffer, bufferSize);
		uint32_t start_ms = millis();
		uint32_t elapsedTime = 0;

		// do an fft to check for test frequencies
		double testThreshold = 0.0;

		// dont wait longer than two seconds for the test tone
		while ((millis() - start_ms < 1000)){
			kiss_fft_cpx fftIn [fftLength];
			kiss_fft_cpx fftOut[fftLength];
			kiss_fft_cfg cfgForwards = kiss_fft_alloc (fftLength, 0, 0, 0);
			kiss_fft_cfg cfgBackwards = kiss_fft_alloc (fftLength, 1, 0, 0);

			// get input from microphone
			bool ok = microphone.readMicrophoneInput(inputBuffer, inputBufferSize);
			if (ok) {
				elapsedTime += 1000.0*inputBufferSize/microphoneSampleRate;

				// copy microphone buffer into complex array and zero padding as input for fft
				for (int i = 0;i < inputBufferSize;i++)
				{
					if (i<inputBufferSize)
						fftIn[i].r = inputBuffer[i];
					else
						fftIn[i].r = 0;
					fftIn[i].i = 0.0;
				}

				// execute fft
				kiss_fft (cfgForwards, fftIn, fftOut);

				// multiply by complex conjugate
				for (int i = 0;i < fftLength;i++) {
					fftOut[i].r = fftOut[i].r * fftOut[i].r + fftOut[i].i * fftOut[i].i;
					fftOut[i].i = 0.0;
				}

				// perform the inverse fft
				kiss_fft (cfgBackwards, fftOut, fftIn);

				// compute the energy of the bands of the test-frequencies
				double testFrequencies = 0;
				for (int i = 0;i<numOfTestFrequencies; i++) {
					testFrequencies += fftOut[fftBin[0]].r;
				}

				// first measurement is used to detect the background energy
				if (testThreshold == 0)
					testThreshold = testFrequencies;

				// once the energy of the test frequencies is rising by a magnitude, we are obviously hearing the test tone
				if (testFrequencies > testThreshold*8.) {
					// cout << "t=" << millis() - start_ms << " " << " elapsed=" << elapsedTime << " ms";;
					// cout << std::fixed << std::setprecision(3) << fftBin[0] << ":" << fftOut[fftBin[0]].r << " " << fftBin[1] << ":" << fftOut[fftBin[1]].r << endl;
					testThreshold = testFrequencies;
					measuredLatency = (float)elapsedTime/1000.0;
				} else
					if (testFrequencies > testThreshold)
						testThreshold = testFrequencies;
				}
				free (cfgForwards);
				free (cfgBackwards);
			}

	} while ((measuredLatency == 0) && (tries++ < 3));
    if (measuredLatency > 0)
    	cout << "measured latency = " << measuredLatency << "s" << endl;

    return measuredLatency;
}

void AudioProcessor::setVolume(double newVolume) {
	volume = newVolume;
}
double AudioProcessor::getVolume() {
	return volume;
}

void AudioProcessor::setGlobalPlayback(bool ok) {
	globalPlayback = ok;
}

bool AudioProcessor::getGlobalPlayback() {
	return globalPlayback;
}

void AudioProcessor::setWavContent(std::vector<uint8_t>& newWavData) {
	audioSource.setWavContent(newWavData);
	playback.setPlayback(globalPlayback);
	pendingBeatType  = BEAT_DETECTION;
}

void AudioProcessor::setMicrophoneInput() {
	audioSource.setMicrophoneInput();

	// switch off playback
	playback.setPlayback(false);
	pendingBeatType  = BEAT_DETECTION;
}

void AudioProcessor::processInput() {

	stopCurrProcessing = false;

	// buffer for audio coming from wav or microphone
	double inputBuffer[numInputSamples];

	while (!stopCurrProcessing) {
		// save preveous input type (wav or microphone)
		AudioSource::InputType beforeInputType = audioSource.getSourceType();

		// fetch samples from source
		audioSource.fetchInput(numInputSamples, inputBuffer);

		// check if source type has changed. If yes, switch to beat generation
		if ((beforeInputType == AudioSource::WAV_INPUT) &&
			(audioSource.getSourceType() == AudioSource::MICROPHONE_INPUT)) {
			pendingBeatType = BEAT_GENERATION;
		}

		// play the buffer asynchronously
		playback.play(volume, inputBuffer,numInputSamples);

		// detect beat and bpm of that hop size
		// pass samples to both beat detectors
		// in order to manage the transitions from one song to another smoothly
		beatDetector -> processAudioFrame(inputBuffer);


		bool beat = false;
		double bpm = 0;

		if (pendingBeatType != NO_BEAT) {
			musicDetected = beatDetector->musicDetected();
			if (((currentBeatType == NO_BEAT) || (currentBeatType == BEAT_GENERATION)) && (pendingBeatType == BEAT_DETECTION)) {
				// wait until beat has been detected before switching
				if (beatDetector->beatDueInCurrentFrame() && musicDetected) {
					if (currentBeatType == BEAT_GENERATION) {
						// reinitialize beat detector and re-scan current buffer
						initializeBeatDetector();
						beatDetector -> processAudioFrame(inputBuffer);
						cout << "re-initializing beat detection" << endl;
					}
					currentBeatType = BEAT_DETECTION;
					pendingBeatType = NO_BEAT;
					cout << "turn on beat detection" << endl;
				}
			}
			else if ((currentBeatType == BEAT_DETECTION) && (pendingBeatType == BEAT_GENERATION)) {
					beatGen.setup(audioSource.getProcessedTime(),lastBeatTime,beatDetector->getCurrentTempoEstimate(), rhythmInQuarters);
					currentBeatType = BEAT_GENERATION;
					pendingBeatType = NO_BEAT;
					cout << "switch to beat generation" << endl;
				}
			else if ((currentBeatType == BEAT_DETECTION) && (pendingBeatType == BEAT_DETECTION)) {
				if (beatDetector->beatDueInCurrentFrame() && musicDetected) {
					// this happens when a new wav file came in. Start with beat generation, but wait for new beat
					beatGen.setup(audioSource.getProcessedTime(),lastBeatTime,beatDetector->getCurrentTempoEstimate(), rhythmInQuarters);
					currentBeatType = BEAT_GENERATION;
					pendingBeatType = BEAT_DETECTION; // initiate change to beat detection once the next beat has been identified
					cout << "temporarily switch to beat generation" << endl;
				}
			}
		}

		switch (currentBeatType) {
			case BEAT_DETECTION:
				beat = beatDetector->beatDueInCurrentFrame();
				bpm = beatDetector->getCurrentTempoEstimate();
				break;
			case BEAT_GENERATION:
				beat = beatGen.getLatchedBeat(audioSource.getProcessedTime());
				bpm = beatGen.getBPM(audioSource.getProcessedTime());
				break;
			case NO_BEAT:
				break;
		}

		if (beat) {
			double processTime  =  audioSource.getProcessedTime();
			double timePerBeat  = (60.0/bpm); 					// [s]
			double timeSinceBeat = processTime - lastBeatTime; 	// [s]

			// detect 1/1 or 1/2 rhythm if we had at least two beats
			// Otherwise leave rhythmInQuarters as it is
			if (lastBeatTime != 0) {
				rhythmInQuarters = 1;
				if (abs(timePerBeat - timeSinceBeat) > abs(2.0*timePerBeat - timeSinceBeat))
					rhythmInQuarters = 2;
			}
			lastBeatTime = processTime;
		}

		// we need to check if there is music or only noise.
		musicDetected = beatDetector->musicDetected();

		beatCallback(audioSource.getProcessedTime(), beat, bpm, rhythmInQuarters);

		// prevent that processing time is running ahead of elapsed time. Otherwise the queue that synrchonises the
		// process time with the real time gets flooded. Existing latency takes care that process
		// does not stutter due to potenta
		double delayTime = audioSource.getProcessedTime() - audioSource.getElapsedTime();
		delay_ms(delayTime*1000.0);
	}
}


double AudioProcessor::getElapsedTime() {
	return audioSource.getElapsedTime();
}

double AudioProcessor::getProcessedTime() {
	return audioSource.getProcessedTime();
}

double AudioProcessor::getCurrentLatency() {
	return audioSource.getCurrentLatency();
}

double AudioProcessor::getMusicKurtosis() {
	return beatDetector->getKurtosis();
}



