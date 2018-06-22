/*
 * WaterInValve.cpp
 *
 *  Created on: 10 июн. 2017 г.
 *      Author: dennis
 */

#include "WaterInValve.h"

WaterInValve::WaterInValve(const uint8_t _valvePin) :
	Valve()
	, valvePin(_valvePin)
	, openState(false)
{
	closeValveChrono.restart();
	openValveChrono.stop();

	pinMode(valvePin, OUTPUT);
	digitalWrite(valvePin, LOW);
}

WaterInValve::~WaterInValve() {
	digitalWrite(valvePin, LOW);
}

boolean WaterInValve::openValve() {
	pinMode(valvePin, OUTPUT);
	digitalWrite(valvePin, HIGH);
	return setValvePosition(Position::VALVE_OPEN);
}

boolean WaterInValve::closeValve() {
	pinMode(valvePin, OUTPUT);
	digitalWrite(valvePin, LOW);
	return setValvePosition(Position::VALVE_CLOSED);
}



