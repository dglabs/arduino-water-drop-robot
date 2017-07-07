/*
 * WaterMotorizedValve.h
 *
 *  Created on: 7 июн. 2017 г.
 *      Author: dennis
 */

#ifndef WATERMOTORIZEDVALVE_H_
#define WATERMOTORIZEDVALVE_H_

#include <Arduino.h>
#include <Chrono.h>
#include "Valve.h"

const long MAX_OUT_VALVE_OPEN_TIME_SECONDS = 600;

const unsigned long VALVE_TRANSIT_TIMEOUT_MILLIS_MIN = 5000;
const unsigned long VALVE_TRANSIT_TIMEOUT_MILLIS_MAX = 7000;
const int STORAGE_SIZE_WATER_VALVE = 1;

class WaterMotorizedValve : public Valve {
protected:
	uint8_t currentState;
	const uint8_t waterPolarPin;
	const uint8_t waterPowerPin;
	const uint8_t signalPin;
	const int memAddress;

	Chrono valveTransitChrono;
public:
	WaterMotorizedValve(const int _memAddress, const uint8_t _waterPolarPin, const uint8_t _waterPowerPin, const uint8_t _signalPin = -1);
	virtual ~WaterMotorizedValve();

	boolean setValvePosition(Position position);
	boolean isOpen();

};

#endif /* WATERMOTORIZEDVALVE_H_ */
