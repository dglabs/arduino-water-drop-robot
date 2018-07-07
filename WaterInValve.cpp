/*
 * WaterInValve.cpp
 *
 *  Created on: 10 июн. 2017 г.
 *      Author: dennis
 */


#include "WaterInValve.h"
#include "WaterDropRobot.h"

WaterInValve::WaterInValve(const uint8_t _valveMask, const uint8_t _valvePin) :
	Valve(_valveMask)
	, valvePin(_valvePin)
{
	closeValveChrono.restart();
	openValveChrono.stop();
	setValvePosition(State::Closed);
}

void WaterInValve::setup() {
#ifdef BOARD_V2
	portExtender.digitalWrite(valvePin, LOW);
#else
	pinMode(valvePin, OUTPUT);
	digitalWrite(valvePin, LOW);
#endif
}

boolean WaterInValve::openValve(const uint8_t _valveMask /*= 0xFF*/, boolean manual /*= false*/) {
	Valve::openValve(_valveMask, manual);
#ifdef BOARD_V2
	portExtender.digitalWrite(valvePin, HIGH);
#else
	digitalWrite(valvePin, HIGH);
#endif
	return setValvePosition(State::Open);
}

boolean WaterInValve::closeValve() {
#ifdef BOARD_V2
	portExtender.digitalWrite(valvePin, LOW);
#else
	digitalWrite(valvePin, LOW);
#endif
	return setValvePosition(State::Closed);
}


