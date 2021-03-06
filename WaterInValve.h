/*
 * WaterInValve.h
 *
 *  Created on: 10 июн. 2017 г.
 *      Author: dennis
 */

#ifndef WATERINVALVE_H_
#define WATERINVALVE_H_

#include <Arduino.h>
#include <Chrono.h>
#include "Valve.h"

const long MAX_IN_VALVE_OPEN_TIME_SECONDS = 250;

class WaterInValve: public virtual Valve {
protected:
	const uint8_t valvePin;
public:
	WaterInValve(const uint8_t _valveMask, const uint8_t _valvePin);
	virtual ~WaterInValve();

	virtual boolean openValve(const uint8_t _valvesMask = 0xFF, boolean manual = false);
	virtual boolean closeValve();

	virtual boolean isOpen() { return state == State::Open; };
	virtual boolean isClosed() { return state == State::Closed; };

	virtual State getState() { return isOpen() ? State::Open : State::Closed; }
};

#endif /* WATERINVALVE_H_ */
