/*
 * RainCoverHandler.cpp
 *
 *  Created on: 6 авг. 2017 г.
 *      Author: dennis
 */

#include "RainCoverHandler.h"
#include "EEPROMUtils.h"

const uint8_t MASK_OPEN = 0xAA;
const uint8_t MASK_CLOSED = 0x55;

const int MOTOR_POWER_OFF = 0;
const int MOTOR_POWER_LOW = 100;
const int MOTOR_POWER_FULL = 220;

const int TILT_SENSOR_OPEN = LOW;
const int TILT_SENSOR_CLOSED = HIGH;

RainCoverHandler::RainCoverHandler(const uint8_t _motorOpenPin, const uint8_t _motorClosePin, const uint8_t _tiltSensorPin, int _memAddress) :
	motorOpenPin(_motorOpenPin)
	, motorClosePin(_motorClosePin)
	, tiltSensorPin(_tiltSensorPin)
	, memAddress(_memAddress)
	, currentState(MASK_CLOSED)
	, isOpenedManually(false)
	, timeToOpenCover(0)
	, operationChrono(Chrono::SECONDS)
{
	pinMode(motorClosePin, OUTPUT); analogWrite(motorClosePin, 0);
	pinMode(motorOpenPin, OUTPUT); analogWrite(motorOpenPin, 0);
	pinMode(tiltSensorPin, INPUT);
	digitalWrite(tiltSensorPin, HIGH);

}

void RainCoverHandler::setup() {
	currentState = EEPROMUtils::read(memAddress);
	switch (currentState) {
	case MASK_OPEN: case MASK_CLOSED: break;
	default:
		currentState = MASK_CLOSED;
		EEPROMUtils::save(memAddress, currentState);
		break;
	}

	timeToOpenCover = EEPROMUtils::readULong(memAddress + sizeof(currentState));
	if (timeToOpenCover < TIME_TO_OPEN_COVER_SEC || timeToOpenCover > MAX_TIME_TO_OPEN_COVER_SEC) {
		timeToOpenCover = TIME_TO_OPEN_COVER_SEC;
		EEPROMUtils::saveULong(memAddress + sizeof(currentState), timeToOpenCover);
	}
}


RainCoverHandler::~RainCoverHandler() {
	// TODO Auto-generated destructor stub
}

boolean RainCoverHandler::isCoverOpen() {
	if (currentState == MASK_OPEN && digitalRead(tiltSensorPin) == TILT_SENSOR_CLOSED) {
		currentState == MASK_CLOSED;
		EEPROMUtils::save(memAddress, currentState);
		return true;
	}
	return currentState == MASK_OPEN;
}

void RainCoverHandler::openCover(boolean manual) {
	isOpenedManually = manual;
	operationChrono.restart();
	analogWrite(motorOpenPin, MOTOR_POWER_FULL);
	analogWrite(motorClosePin, 0);

	while (operationChrono.elapsed() < MAX_TIME_TO_OPEN_COVER_SEC) {
		if (operationChrono.elapsed() >= TIME_TO_OPEN_COVER_SEC_MIN && digitalRead(tiltSensorPin) == TILT_SENSOR_OPEN) {
			break;
		}

		delay(250);
	}
	analogWrite(motorOpenPin, 0);
	analogWrite(motorClosePin, 0);

	if (operationChrono.elapsed() >= TIME_TO_OPEN_COVER_SEC_MIN && operationChrono.elapsed() < MAX_TIME_TO_OPEN_COVER_SEC && digitalRead(tiltSensorPin) == TILT_SENSOR_OPEN) {
		timeToOpenCover = operationChrono.elapsed();
		EEPROMUtils::saveULong(memAddress + sizeof(currentState), timeToOpenCover);
	}

	currentState = MASK_OPEN;
	EEPROMUtils::save(memAddress, currentState);
}

void RainCoverHandler::closeCover() {
	operationChrono.restart();
	analogWrite(motorClosePin, MOTOR_POWER_FULL);
	analogWrite(motorOpenPin, 0);
	while (operationChrono.elapsed() < timeToOpenCover) {
		delay(250);
	}
	analogWrite(motorOpenPin, 0);
	analogWrite(motorClosePin, 0);
	currentState = MASK_CLOSED;
	EEPROMUtils::save(memAddress, currentState);
}

