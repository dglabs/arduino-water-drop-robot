/*
 * WaterMotorizedValve.cpp
 *
 *  Created on: 7 июн. 2017 г.
 *      Author: dennis
 */

#include "WaterMotorizedValve.h"
#include "EEPROMUtils.h"

const int MOTOR_POWER_OFF = 0;
const int MOTOR_POWER_LOW = 100;
const int MOTOR_POWER_FULL = 160;

const int DIRECTION_OPEN = LOW;
const int DIRECTION_CLOSE = HIGH;

const uint8_t MASK_OPEN = 0xAA;
const uint8_t MASK_CLOSED = 0x55;

WaterMotorizedValve::WaterMotorizedValve(const int _memAddress, const uint8_t _motorOpenPin, const uint8_t _motorClosePin
		, const uint8_t _signalPinOpen /*= 0*/, const uint8_t _signalPinClosed /*= 0*/) :
	Valve()
	, memAddress(_memAddress)
	, motorOpenPin(_motorOpenPin)
	, motorClosePin(_motorClosePin)
	, signalPinOpen( _signalPinOpen)
	, signalPinClosed( _signalPinClosed)
	, valveTransitChrono(Chrono::MILLIS)
	, currentState(MASK_OPEN)
{
	pinMode(motorOpenPin, OUTPUT); analogWrite(motorOpenPin, 0);
	pinMode(motorClosePin, OUTPUT); analogWrite(motorClosePin, 0);
	if (signalPinOpen > 0)
		pinMode(signalPinOpen, INPUT_PULLUP);
	if (signalPinClosed > 0)
		pinMode(signalPinClosed, INPUT_PULLUP);
}

void WaterMotorizedValve::setup() {
	currentState = EEPROMUtils::read(memAddress);
	//Serial.print("Current state: "); Serial.println(currentState, HEX);

	switch (currentState) {
	case MASK_OPEN: case MASK_CLOSED: break;
	default:
		currentState = MASK_OPEN;
		EEPROMUtils::save(memAddress, currentState);
		break;
	}
}


WaterMotorizedValve::~WaterMotorizedValve() {}

void WaterMotorizedValve::processValve(Position position, int signalPin) {
	int activePin = 0;

	switch (position) {
	case Position::VALVE_OPEN:
		activePin = motorOpenPin;
		analogWrite(motorClosePin, 0);
		break;
	case Position::VALVE_CLOSED:
		activePin = motorClosePin;
		analogWrite(motorOpenPin, 0);
		break;
	}
	analogWrite(activePin, MOTOR_POWER_FULL);

	valveTransitChrono.restart(0);
	int initialSignalPin = (signalPin > 0) ?  digitalRead(signalPin) : HIGH;
	//boolean signalPinChanged = false;
	do {
		delay(250);

		if (signalPin > 0) {
			//if (initialSignalPin != digitalRead(signalPin)) signalPinChanged = true;
			if (valveTransitChrono.elapsed() > VALVE_TRANSIT_TIMEOUT_MILLIS_MIN /*&& signalPinChanged*/ && digitalRead(signalPin) == LOW) break;
		}
	} while (valveTransitChrono.elapsed() < VALVE_TRANSIT_TIMEOUT_MILLIS_MAX);
	analogWrite(motorOpenPin, 0);
	analogWrite(motorClosePin, 0);
}

boolean WaterMotorizedValve::openValve() {
	if (isOpen()) return false;

	processValve(Position::VALVE_OPEN, signalPinOpen);

	currentState = MASK_OPEN;
	EEPROMUtils::save(memAddress, currentState);
	setValvePosition(Position::VALVE_OPEN);
	return true;
}

boolean WaterMotorizedValve::closeValve() {
	if (isClosed()) return false;

	processValve(Position::VALVE_CLOSED, signalPinClosed);

	currentState = MASK_CLOSED;
	EEPROMUtils::save(memAddress, currentState);
	setValvePosition(Position::VALVE_CLOSED);
	return true;
}

boolean WaterMotorizedValve::isOpen() {
	if (signalPinOpen > 0)
		if ((digitalRead(signalPinOpen) ^ digitalRead(signalPinClosed)))
			return digitalRead(signalPinOpen) == LOW;
		else return true;	// This is undefined state and treat it as open
	else return currentState != MASK_CLOSED;
}

boolean WaterMotorizedValve::isClosed() {
	if (signalPinClosed > 0) {
		if ((digitalRead(signalPinOpen) ^ digitalRead(signalPinClosed)))
			return digitalRead(signalPinClosed) == LOW;
		else return false;	// This is undefined state and treat it as open
	}
	else return currentState != MASK_OPEN;
}

