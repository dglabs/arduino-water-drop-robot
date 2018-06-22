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
	boolean openState;
public:
	WaterInValve(const uint8_t _valvePin);
	virtual ~WaterInValve();

	boolean setValvePosition(Position position) {
		switch (position) {
		case VALVE_OPEN: {
			openState = true;
		} break;
		case VALVE_CLOSED: {
			openState = false;
		} break;
		}
		return Valve::setValvePosition(position);
	}

	virtual boolean openValve();
	virtual boolean closeValve();

	virtual boolean isOpen() { return openState; };
	virtual boolean isClosed() { return !openState; };
};

#endif /* WATERINVALVE_H_ */
