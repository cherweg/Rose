/*
 * ServoController.cpp
 *
 *  Created on: Mar 4, 2018
 *      Author: jochenalt
 */

#ifdef __linux__

#include <assert.h>
#include <iostream>
#include "basics/util.h"
#include <servo/ServoController.h>
#include <stewart/BodyKinematics.h>

using namespace std;

ServoController::ServoController() {
}

ServoController::~ServoController() {
}


float ServoController::getServoFrequency() {
	// PCA9685 has descrete frequency values (s.a. datasheet)
	// take the one which is closest to servo frequency of 333Hz
	// (above 333Hz would be 338 Hz)
	return 333;
};

void ServoController::setup() {
	const int i2cBus = 1; 			// this is /dev/ic2-1
	const int i2cAddress = 0x40; 	// i2c address of PCA 9685 from datasheet
	const int servoFrequency = getServoFrequency(); // [Hz] working frequency of KST 215 MG V3 (from datasheet)
	// initialize PCA 9685 board via Adafruit library
	cout << "servo setup via /dev/i2c-" << i2cBus << " address=" << i2cAddress << " at " << servoFrequency << " Hz" << endl;
	pca9685.setup(i2cBus,i2cAddress,servoFrequency);

	BodyKinematics& bodyKin = BodyKinematics::getInstance();
	double bottomServoLimit = degrees(bodyKin.getStewartConfig().bottomServoLimit_rad);
	double topServoLimit = degrees(bodyKin.getStewartConfig().topServoLimit_rad);

	// set each servo of stewart platform with default null values and limits
	servo[ STEWART_SERVO0].setup(&pca9685,  STEWART_SERVO0, servoFrequency, false, bottomServoLimit, topServoLimit, 0-4);
	servo[ STEWART_SERVO1].setup(&pca9685,  STEWART_SERVO1, servoFrequency, true,  bottomServoLimit, topServoLimit, -30-3);
	servo[ STEWART_SERVO2].setup(&pca9685,  STEWART_SERVO2, servoFrequency, false, bottomServoLimit, topServoLimit, 0-7);
	servo[ STEWART_SERVO3].setup(&pca9685,  STEWART_SERVO3, servoFrequency, true,  bottomServoLimit, topServoLimit, -30-7);
	servo[ STEWART_SERVO4].setup(&pca9685,  STEWART_SERVO4, servoFrequency, false, bottomServoLimit, topServoLimit, -1);
	servo[ STEWART_SERVO5].setup(&pca9685,  STEWART_SERVO5, servoFrequency, true,  bottomServoLimit, topServoLimit, -30-3);

	// set each servo of of mouth platform with default null values and limits
	servo[ MOUTH_TURN_SERVO].setup(&pca9685,  MOUTH_TURN_SERVO, servoFrequency, true,  -35, +35, -8);
	servo[ MOUTH_OPEN_SERVO].setup(&pca9685,  MOUTH_OPEN_SERVO, servoFrequency, false, -35, 40, 6);
	servo[ MOUTH_TILT_SERVO].setup(&pca9685,  MOUTH_TILT_SERVO, servoFrequency, false,  -10, 70, 6);

}

double ServoController::getMinAngle(int servoNo) {
	assert ((servoNo>= 0) && (servoNo < NumOfServos));
	return servo[servoNo].minAngle;
}
double ServoController::getMaxAngle(int servoNo) {
	assert ((servoNo>= 0) && (servoNo < NumOfServos));

	return servo[servoNo].maxAngle;
}

double ServoController::getNullAngle(int servoNo) {
	assert ((servoNo>= 0) && (servoNo < NumOfServos));

	return servo[servoNo].nullAngle;
}

void ServoController::setMinAngle(int servoNo, double newValue) {
	assert ((servoNo>= 0) && (servoNo < NumOfServos));

	servo[servoNo].minAngle = newValue;
}
void ServoController::setMaxAngle(int servoNo, double newValue) {
	assert ((servoNo>= 0) && (servoNo < NumOfServos));

	servo[servoNo].maxAngle = newValue;
}

void ServoController::setNullAngle(int servoNo, double newValue) {
	assert ((servoNo>= 0) && (servoNo < NumOfServos));

	servo[servoNo].nullAngle = newValue;
}

double ServoController::getAngle(int servoNo) {
	return servo[servoNo].getAngle();
}


void ServoController::setAngle_rad(int servoNo, double newAngle_rad) {
	servo[servoNo].setAngle(degrees(newAngle_rad));
}

void ServoController::setAngle_deg(int servoNo, double angle) {
	servo[servoNo].setAngle(angle);
}


void ServoController::calibrateViaKeyBoard() {
	setup();
	int currentServoBlock = 0;
	int currentServo = 0;
	int currentAngle = 0;
	cout << "stewart patform/mouth : (a,b)" << endl
		 << "select servo          : (0,1,2,3,4,5)" << endl
	     << "set null value        : n" << endl
	     << "change angle          : +/-" << endl;

	changemode(1);
    while (true) {

    	if (kbhit()) {
    		char inp= getchar();

			switch (inp) {
			case 'a':
				currentServoBlock = 0;
				currentServo = 0;
				break;
			case 'b':
				currentServoBlock = 1;
				currentServo = 0;
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
  			    currentServo = inp - '0';
				break;
			case '+':
				currentAngle += 1;
				servo[currentServo + currentServoBlock*6].setAngle(currentAngle);
				break;
			case 'n': {
				setNullAngle(currentServo + currentServoBlock*6, currentAngle + getNullAngle(currentServo + currentServoBlock*6));
				setAngle_deg(currentServo + currentServoBlock*6, 0);
				break;
			}
			case '-':
				currentAngle -= 1;
				servo[currentServo + currentServoBlock*6].setAngle(currentAngle);
				break;
			}
			currentAngle = getAngle(currentServo + currentServoBlock*6);
			cout << ((currentServoBlock == 0)?"A":"B") << "/" << currentServo << " -> " << currentAngle << " deg null="  << getNullAngle(currentServo + currentServoBlock*6) << endl;
    	}
    }
	changemode(0);

}

#endif
