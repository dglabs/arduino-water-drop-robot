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

const unsigned long VALVE_TRANSIT_SLOW_MILLIS = 500;
const unsigned long VALVE_TRANSIT_TIMEOUT_MILLIS_MIN = 5000;
const unsigned long VALVE_TRANSIT_TIMEOUT_MILLIS_MAX = 8000;
const int STORAGE_SIZE_WATER_VALVE = 1;

class WaterMotorizedValve : public virtual Valve {
protected:
	uint8_t currentState;
	const uint8_t motorOpenPin;
	const uint8_t motorClosePin;
	const uint8_t signalPinOpen;
	const uint8_t signalPinClosed;
	const int memAddress;

	Chrono valveTransitChrono;

	void processValve(Position position, int signalPin);

public:
	WaterMotorizedValve(const int _memAddress, const uint8_t _motorOpenPin, const uint8_t _motorClosePin, const uint8_t _signalPinOpen = 0, const uint8_t _signalPinClosed = 0);
	virtual ~WaterMotorizedValve();

	virtual void setup();

	virtual boolean openValve();
	virtual boolean closeValve();

	virtual boolean isOpen();
	virtual boolean isClosed();

};

#endif /* WATERMOTORIZEDVALVE_H_ */
