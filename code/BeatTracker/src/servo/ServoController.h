/*
 * ServoController.h
 *
 *  Created on: Mar 4, 2018
 *      Author: jochenalt
 */

#ifndef SRC_SERVO_SERVOCONTROLLER_H_
#define SRC_SERVO_SERVOCONTROLLER_H_

#ifdef __linux__
#include "PCA9685.h"
#include "Servo.h"


class ServoController {
public:
	ServoController();
	virtual ~ServoController();

	// initialize PCA9685 and all servos
	void setup();
	static ServoController& getInstance() {
		static ServoController instance;
		return instance;
	}

	void calibrateViaKeyBoard();

	double  getAngle(int servoNo);
	void setAngle(int servoNo, double angle);
	void setAngle_rad(int servoNo, double newAngle_rad);


	double getMinAngle(int servoNo);
	double getMaxAngle(int servoNo);
	double getNullAngle(int servoNo);

	void setMinAngle(int servoNo, double newValue);
	void setMaxAngle(int servoNo, double newValue);
	void setNullAngle(int servoNo, double newValue);

private:
	PCA9685 pca9685;

	const static int numServos = 12;
	Servo servo[numServos];
};

#endif
#endif /* SRC_SERVO_SERVOCONTROLLER_H_ */
