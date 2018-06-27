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

RainCoverHandler::RainCoverHandler(const uint8_t _valveMask, const uint8_t _motorOpenPin, const uint8_t _motorClosePin, const uint8_t _tiltSensorPin, int _memAddress) :
	Valve(_valveMask)
	, motorOpenPin(_motorOpenPin)
	, motorClosePin(_motorClosePin)
	, tiltSensorPin(_tiltSensorPin)
	, memAddress(_memAddress)
	, timeToOpenCover(0)
	, operationChrono(Chrono::SECONDS)
{
	pinMode(motorClosePin, OUTPUT); analogWrite(motorClosePin, 0);
	pinMode(motorOpenPin, OUTPUT); analogWrite(motorOpenPin, 0);
	pinMode(tiltSensorPin, INPUT);
	digitalWrite(tiltSensorPin, HIGH);

}

void RainCoverHandler::setup() {
	state = (State)(EEPROMUtils::read(memAddress));
	switch (state) {
	case State::Open: case State::Opening: case State::Closing: case State::Closed: break;
	default:
		state = State::Open;
		EEPROMUtils::save(memAddress, state);
		break;
	}

	timeToOpenCover = EEPROMUtils::readULong(memAddress + sizeof(state));
	if (timeToOpenCover < TIME_TO_OPEN_COVER_SEC || timeToOpenCover > MAX_TIME_TO_OPEN_COVER_SEC) {
		timeToOpenCover = TIME_TO_OPEN_COVER_SEC;
		EEPROMUtils::saveULong(memAddress + sizeof(state), timeToOpenCover);
	}
}


RainCoverHandler::~RainCoverHandler() {
	// TODO Auto-generated destructor stub
}

boolean RainCoverHandler::isClosed() {
	switch (state) {
	case State::Closed:
		if (digitalRead(tiltSensorPin) != TILT_SENSOR_CLOSED) {
			state = State::Open;
			EEPROMUtils::save(memAddress, state);
			return false;
		}
		else return true;
	default: return false;
	}
}


boolean RainCoverHandler::isOpen() {
	switch (state) {
	case State::Open:
		if (digitalRead(tiltSensorPin) == TILT_SENSOR_CLOSED) {
			state = State::Closed;
			EEPROMUtils::save(memAddress, state);
			return false;
		}
		else return true;
	default: return false;
	}
}

void RainCoverHandler::loop() {
	switch (state)
	{
	case Opening: {
		if (operationChrono.elapsed() < MAX_TIME_TO_OPEN_COVER_SEC &&
				!(operationChrono.elapsed() >= TIME_TO_OPEN_COVER_SEC_MIN && digitalRead(tiltSensorPin) == TILT_SENSOR_OPEN)) {
			delay(50);
		}
		else {
			analogWrite(motorOpenPin, 0);
			analogWrite(motorClosePin, 0);

			if (operationChrono.elapsed() >= TIME_TO_OPEN_COVER_SEC_MIN && operationChrono.elapsed() < MAX_TIME_TO_OPEN_COVER_SEC && digitalRead(tiltSensorPin) == TILT_SENSOR_OPEN) {
				timeToOpenCover = operationChrono.elapsed();
				EEPROMUtils::saveULong(memAddress + sizeof(state), timeToOpenCover);
			}

			EEPROMUtils::save(memAddress, state);
			setValvePosition(State::Open);
		}
	} break;
	case Closing: {
		if (operationChrono.elapsed() < ((timeToOpenCover * 100) / 110)) {
			delay(50);
		}
		else {
			analogWrite(motorOpenPin, 0);
			analogWrite(motorClosePin, 0);
			EEPROMUtils::save(memAddress, state);
			setValvePosition(State::Closed);
		}
	} break;
	}
}

boolean RainCoverHandler::openValve(const uint8_t _valveMask /*= 0xFF*/, boolean manual /*=false*/) {
	Valve::openValve(_valveMask, manual);

	operationChrono.restart();
	analogWrite(motorOpenPin, MOTOR_POWER_FULL);
	analogWrite(motorClosePin, 0);

	return setValvePosition(State::Opening);

	/*while (operationChrono.elapsed() < MAX_TIME_TO_OPEN_COVER_SEC) {
		if (operationChrono.elapsed() >= TIME_TO_OPEN_COVER_SEC_MIN && digitalRead(tiltSensorPin) == TILT_SENSOR_OPEN) {
			break;
		}

		delay(250);
	}


	analogWrite(motorOpenPin, 0);
	analogWrite(motorClosePin, 0);

	if (operationChrono.elapsed() >= TIME_TO_OPEN_COVER_SEC_MIN && operationChrono.elapsed() < MAX_TIME_TO_OPEN_COVER_SEC && digitalRead(tiltSensorPin) == TILT_SENSOR_OPEN) {
		timeToOpenCover = operationChrono.elapsed();
		EEPROMUtils::saveULong(memAddress + sizeof(state), timeToOpenCover);
	}

	EEPROMUtils::save(memAddress, state);
	return setValvePosition(State::Open);*/
}

boolean  RainCoverHandler::closeValve() {
	operationChrono.restart();
	analogWrite(motorClosePin, MOTOR_POWER_FULL);
	analogWrite(motorOpenPin, 0);

	return setValvePosition(State::Closing);

	/*while (operationChrono.elapsed() < timeToOpenCover) {
		delay(250);
	}
	analogWrite(motorOpenPin, 0);
	analogWrite(motorClosePin, 0);
	EEPROMUtils::save(memAddress, state);
	return setValvePosition(State::Closed);*/
}

