/*
 * RainCoverHandler.cpp
 *
 *  Created on: 6 авг. 2017 г.
 *      Author: dennis
 */

#include "RainCoverHandler.h"
#include "EEPROMUtils.h"
#include "WaterDropRobot.h"

const uint8_t MASK_OPEN = 0xAA;
const uint8_t MASK_CLOSED = 0x55;

const int MOTOR_POWER_OFF = 0;
const int MOTOR_POWER_LOW = 100;
const int MOTOR_POWER_FULL = 220;

const int TILT_SENSOR_OPEN = LOW;
const int TILT_SENSOR_CLOSED = HIGH;

RainCoverHandler::RainCoverHandler(const uint8_t _valveMask
		, const uint8_t _pwm0Pin
		, const uint8_t _pwm1Pin
		, const uint8_t _motorENPin
		, const uint8_t _tiltSensorPin
		, int _memAddress) :
#ifdef BOARD_V2
	Valve(_valveMask)
#else
	Valve(_valveMask)
#endif
	, pwm0Pin(_pwm0Pin)
	, pwm1Pin(_pwm1Pin)
	, motorENPin(_motorENPin)
	, tiltSensorPin(_tiltSensorPin)
	, memAddress(_memAddress)
	, timeToOpenCover(0)
	, operationChrono(Chrono::SECONDS)
{
}

void RainCoverHandler::setup() {
#ifdef BOARD_V2
	//portExtender.pinMode(tiltSensorPin, INPUT_PULLUP);
	pinMode(motorENPin, OUTPUT); digitalWrite(motorENPin, LOW);
#else
	pinMode(pwm1Pin, OUTPUT); analogWrite(pwm1Pin, 0);
	pinMode(pwm0Pin, OUTPUT); analogWrite(pwm0Pin, 0);
	pinMode(tiltSensorPin, INPUT_PULLUP);
#endif

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

boolean RainCoverHandler::isClosed() {
	switch (state) {
	case State::Closed:
#ifdef BOARD_V2
		if (portExtender.digitalRead(tiltSensorPin) != TILT_SENSOR_CLOSED) {
#else
		if (digitalRead(tiltSensorPin) != TILT_SENSOR_CLOSED) {
#endif
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
#ifdef BOARD_V2
		if (portExtender.digitalRead(tiltSensorPin) != TILT_SENSOR_CLOSED) {
#else
		if (digitalRead(tiltSensorPin) != TILT_SENSOR_CLOSED) {
#endif
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
				!(operationChrono.elapsed() >= TIME_TO_OPEN_COVER_SEC_MIN &&
#ifdef BOARD_V2
					portExtender.digitalRead(tiltSensorPin) == TILT_SENSOR_OPEN)
#else
					digitalRead(tiltSensorPin) == TILT_SENSOR_OPEN)
#endif
		) {
			delay(50);
		}
		else {
			digitalWrite(motorENPin, LOW);
			analogWrite(pwm0Pin, 0);
			analogWrite(pwm1Pin, 0);

			if (operationChrono.elapsed() >= TIME_TO_OPEN_COVER_SEC_MIN && operationChrono.elapsed() < MAX_TIME_TO_OPEN_COVER_SEC &&
#ifdef BOARD_V2
					portExtender.digitalRead(tiltSensorPin) == TILT_SENSOR_OPEN
#else
					digitalRead(tiltSensorPin) == TILT_SENSOR_OPEN
#endif
				) {
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
			digitalWrite(motorENPin, LOW);
			analogWrite(pwm0Pin, 0);
			analogWrite(pwm1Pin, 0);
			EEPROMUtils::save(memAddress, state);
			setValvePosition(State::Closed);
		}
	} break;
	}
}

boolean RainCoverHandler::openValve(const uint8_t _valveMask /*= 0xFF*/, boolean manual /*=false*/) {
	Valve::openValve(_valveMask, manual);

	operationChrono.restart();
	analogWrite(pwm0Pin, MOTOR_POWER_FULL);
	analogWrite(pwm1Pin, 0);
	digitalWrite(motorENPin, HIGH);

	return setValvePosition(State::Opening);
}

boolean  RainCoverHandler::closeValve() {
	operationChrono.restart();
	analogWrite(pwm0Pin, 0);
	analogWrite(pwm1Pin, MOTOR_POWER_FULL);
	digitalWrite(motorENPin, HIGH);

	return setValvePosition(State::Closing);
}

