/*
 * WaterInValve.cpp
 *
 *  Created on: 10 июн. 2017 г.
 *      Author: dennis
 */

#include "WaterInValve.h"

WaterInValve::WaterInValve(const uint8_t _valvePin) :
	Valve(),
	valvePin(_valvePin)
{
	pinMode(valvePin, OUTPUT);
	digitalWrite(valvePin, HIGH);

	openValveChrono.stop();
}

WaterInValve::~WaterInValve() {
	digitalWrite(valvePin, HIGH);
}

boolean WaterInValve::setValvePosition(Position position) {
	if (isSamePosition(position)) return false;
	switch (position) {
	case VALVE_OPEN: {
		digitalWrite(valvePin, LOW);
	} break;
	case VALVE_CLOSED: {
		digitalWrite(valvePin, HIGH);
	} break;
	}
	return Valve::setValvePosition(position);
}

boolean WaterInValve::isOpen() { return digitalRead(valvePin) == LOW; }


