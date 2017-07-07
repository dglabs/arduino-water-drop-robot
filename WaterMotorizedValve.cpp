/*
 * WaterMotorizedValve.cpp
 *
 *  Created on: 7 июн. 2017 г.
 *      Author: dennis
 */

#include "WaterMotorizedValve.h"
#include <EEPROM.h>

const uint8_t MASK_OPEN = 0xAA;
const uint8_t MASK_CLOSED = 0x55;

WaterMotorizedValve::WaterMotorizedValve(const int _memAddress, const uint8_t _waterPolarPin, const uint8_t _waterPowerPin, const uint8_t _signalPin /*= -1*/) :
	Valve()
	, memAddress(_memAddress)
	, waterPolarPin(_waterPolarPin)
	, waterPowerPin(_waterPowerPin)
	, signalPin( _signalPin)
	, valveTransitChrono(Chrono::MILLIS)
{
	pinMode(waterPolarPin, OUTPUT); digitalWrite(waterPolarPin, HIGH);
	pinMode(waterPowerPin, OUTPUT); digitalWrite(waterPowerPin, HIGH);
	if (signalPin > 0)
		pinMode(signalPin, INPUT_PULLUP);

	currentState = EEPROM.read(memAddress);
	switch (currentState) {
	case MASK_OPEN: case MASK_CLOSED: break;
	default:
		currentState = MASK_OPEN;
		EEPROM.write(memAddress, currentState);
		break;
	}
}

WaterMotorizedValve::~WaterMotorizedValve() {}

boolean WaterMotorizedValve::setValvePosition(Position position) {
	if (isSamePosition(position) && Position::VALVE_CLOSED == position) return false;

	digitalWrite(waterPolarPin, Position::VALVE_OPEN == position ? HIGH : LOW);
	valveTransitChrono.restart(0);
	digitalWrite(waterPowerPin, LOW);
	int initialSignalPin = (signalPin > 0) ?  digitalRead(signalPin) : HIGH;
	boolean signalPinChanged = false;
	do {
		delay(500);
		if (signalPin > 0) {
			if (initialSignalPin != digitalRead(signalPin)) signalPinChanged = true;
			if (valveTransitChrono.elapsed() > VALVE_TRANSIT_TIMEOUT_MILLIS_MIN && signalPinChanged && digitalRead(signalPin) == LOW) break;
		}
	} while (valveTransitChrono.elapsed() < VALVE_TRANSIT_TIMEOUT_MILLIS_MAX);
	digitalWrite(waterPowerPin, HIGH);
	digitalWrite(waterPolarPin, HIGH);
	switch (position) {
	case VALVE_OPEN:
		currentState = MASK_OPEN;
		break;
	case VALVE_CLOSED:
		currentState = MASK_CLOSED;
		break;
	}
	EEPROM.write(memAddress, currentState);

	return Valve::setValvePosition(position);
}

boolean WaterMotorizedValve::isOpen() { return currentState != MASK_CLOSED; }


