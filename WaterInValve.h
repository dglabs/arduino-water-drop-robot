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

const long MAX_IN_VALVE_OPEN_TIME_SECONDS = 500;

class WaterInValve: public Valve {
protected:
	const uint8_t valvePin;
public:
	WaterInValve(const uint8_t _valvePin);
	virtual ~WaterInValve();

	boolean setValvePosition(Position position);
	boolean isOpen();
};

#endif /* WATERINVALVE_H_ */
