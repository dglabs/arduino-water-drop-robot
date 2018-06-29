/*
 * WaterInValve.cpp
 *
 *  Created on: 10 июн. 2017 г.
 *      Author: dennis
 */

#include "WaterInValve.h"

WaterInValve::WaterInValve(const uint8_t _valveMask, const uint8_t _valvePin) :
	Valve(_valveMask)
	, valvePin(_valvePin)
{
	closeValveChrono.restart();
	openValveChrono.stop();

	pinMode(valvePin, OUTPUT);
	digitalWrite(valvePin, LOW);
	setValvePosition(State::Closed);
}

WaterInValve::~WaterInValve() {
	digitalWrite(valvePin, LOW);
}

boolean WaterInValve::openValve(const uint8_t _valveMask /*= 0xFF*/, boolean manual /*= false*/) {
	Valve::openValve(_valveMask, manual);
	pinMode(valvePin, OUTPUT);
	digitalWrite(valvePin, HIGH);
	return setValvePosition(State::Open);
}

boolean WaterInValve::closeValve() {
	pinMode(valvePin, OUTPUT);
	digitalWrite(valvePin, LOW);
	return setValvePosition(State::Closed);
}



