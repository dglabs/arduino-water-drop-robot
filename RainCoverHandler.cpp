/*
 * RainCoverHandler.cpp
 *
 *  Created on: 6 авг. 2017 г.
 *      Author: dennis
 */

#include "RainCoverHandler.h"
#include <EEPROM.h>
#include "EEPROMUtils.h"

const uint8_t MASK_OPEN = 0xAA;
const uint8_t MASK_CLOSED = 0x55;

const int DIRECTION_OPEN = HIGH;
const int DIRECTION_CLOSE = LOW;

const int TILT_SENSOR_OPEN = LOW;
const int TILT_SENSOR_CLOSED = HIGH;

RainCoverHandler::RainCoverHandler(const uint8_t _motorPowerPin, const uint8_t _motorDirectionPin, const uint8_t _tiltSensorPin, int _memAddress) :
	motorPowerPin(_motorPowerPin)
	, motorDirectionPin(_motorDirectionPin)
	, tiltSensorPin(_tiltSensorPin)
	, memAddress(_memAddress)
	, currentState(MASK_CLOSED)
	, isOpenedManually(false)
	, timeToOpenCover(0)
	, operationChrono(Chrono::SECONDS)
{
	pinMode(motorPowerPin, OUTPUT); digitalWrite(motorPowerPin, HIGH);
	pinMode(motorDirectionPin, OUTPUT); digitalWrite(motorDirectionPin, HIGH);
	pinMode(tiltSensorPin, INPUT_PULLUP);

	currentState = EEPROM.read(memAddress);
	switch (currentState) {
	case MASK_OPEN: case MASK_CLOSED: break;
	default:
		currentState = MASK_CLOSED;
		EEPROM.write(memAddress, currentState);
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
	/*if (currentState == MASK_OPEN && digitalRead(tiltSensorPin) == TILT_SENSOR_CLOSED) {
		currentState == MASK_CLOSED;
		EEPROM.write(memAddress, currentState);
		return true;
	}*/
	return currentState == MASK_OPEN;
}

void RainCoverHandler::openCover(boolean manual) {
	isOpenedManually = manual;
	operationChrono.restart();
	digitalWrite(motorDirectionPin, DIRECTION_OPEN);
	digitalWrite(motorPowerPin, LOW);
	while (operationChrono.elapsed() < TIME_TO_OPEN_COVER_SEC/*timeToOpenCover && digitalRead(tiltSensorPin) == LOW*/) {
		delay(500);
	}
	digitalWrite(motorPowerPin, HIGH);
	digitalWrite(motorDirectionPin, HIGH);

	/*if (operationChrono.elapsed() >= TIME_TO_OPEN_COVER_SEC && operationChrono.elapsed() < MAX_TIME_TO_OPEN_COVER_SEC && ) {
		timeToOpenCover = operationChrono.elapsed();
		EEPROMUtils::saveULong(memAddress + sizeof(currentState), timeToOpenCover);
	}*/

	currentState = MASK_OPEN;
	EEPROM.write(memAddress, currentState);
}

void RainCoverHandler::closeCover() {
	digitalWrite(motorDirectionPin, DIRECTION_CLOSE);
	digitalWrite(motorPowerPin, LOW);
	operationChrono.restart();
	while (operationChrono.elapsed() < TIME_TO_OPEN_COVER_SEC/*timeToOpenCover*/) {
		delay(500);
	}
	digitalWrite(motorPowerPin, HIGH);
	digitalWrite(motorDirectionPin, HIGH);
	currentState = MASK_CLOSED;
	EEPROM.write(memAddress, currentState);
}

