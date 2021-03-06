/*
 * Playback.cpp
 *
 *  Created on: Mar 20, 2018
 *      Author: jochenalt
 */


#include <assert.h>
#include <iostream>
#include <string.h>
#include <audio/Playback.h>
#include <audio/SoundCardUtils.h>

using namespace std;

Playback::Playback() {
}

Playback::~Playback() {
	if (pulseAudioConnection != NULL) {
		 pa_simple_free(pulseAudioConnection);
		 pulseAudioConnection = NULL;
	}
}


void Playback::setup(int newSampleRate) {
    if (sampleRate == newSampleRate)
    	return;

    sampleRate = newSampleRate;
    /* The Sample format to use */
    ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = (uint32_t)sampleRate,
        .channels = 1
    };

    int error = 0;

    if (deviceName.empty())
    	deviceName = SoundCardUtils::getInstance().getDefaultOutputDevice().name;
    if (pulseAudioConnection != NULL) {
		 pa_simple_free(pulseAudioConnection);
		 pulseAudioConnection = NULL;
    }
    pulseAudioConnection = pa_simple_new(NULL, "Donna", PA_STREAM_PLAYBACK, deviceName.c_str(), "playback", &ss, NULL, NULL, &error);
    if (pulseAudioConnection == NULL) {
        cerr << "ERR: pa_simple_new on " << deviceName << " failed: " <<  pa_strerror(error);
        exit(1);
    }

   	// cout << "using device " << deviceName << " for audio playback " << sampleRate << "Hz for audio output " << endl;
}

void Playback::play(double volume /* 0..1 */ ,double outputBuffer[], int outputBufferSize) {
	if (!playback)
		return;

	char playBuffer[outputBufferSize*2];
	int outputBufferCount = 0;
    int bits = 16;
	for (int i = 0;i<outputBufferSize;i++) {
		double sample = outputBuffer[i];
		int sampleValue = sample*volume*(float)(1<<(bits-1));
		if (sampleValue < 0)
			sampleValue = (1<<bits) + sampleValue;

		// set frame value into output buffer to be played later on
		// use unsigned 16bits, little endian (U16LE)
		assert (outputBufferCount  < outputBufferSize*2);
		playBuffer[outputBufferCount++] = (uint8_t)(sampleValue & 0xFF);
		assert (outputBufferCount < outputBufferSize*2);
		playBuffer[outputBufferCount++] = (uint8_t)(sampleValue >> 8);
	}

	int error = 0;
    int bytesWritten = pa_simple_write(pulseAudioConnection, playBuffer, (size_t) outputBufferCount, &error);
    if (bytesWritten < 0) {
        cerr << "ERR: pa_simple_write failed: " << pa_strerror(error);
        exit(1);
    }
}

