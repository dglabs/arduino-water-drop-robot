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

#include "CommonDefs.h"
#include "Valve.h"

const long MAX_OUT_VALVE_OPEN_TIME_SECONDS = 600;

const unsigned long VALVE_TRANSIT_SLOW_MILLIS = 500;
const unsigned long VALVE_TRANSIT_TIMEOUT_MILLIS_MIN = 5000;
const unsigned long VALVE_TRANSIT_TIMEOUT_MILLIS_MAX = 8000;
const int STORAGE_SIZE_WATER_VALVE = 1;

class WaterMotorizedValve : public virtual Valve {
protected:
	const uint8_t& motorOpenPin;
	const uint8_t& motorClosePin;
#ifndef BOARD_V2
	const uint8_t& signalPinOpen;
	const uint8_t& signalPinClosed;
#endif
	const int& memAddress;

	Chrono valveTransitChrono;
#ifndef BOARD_V2
	int signalPin;
	boolean initialSignalPin;
	boolean signalPinChanged;
#endif

	void processValve(State position
#ifndef BOARD_V2
			, int _signalPin
#endif
	);

public:
	WaterMotorizedValve(const uint8_t _valveMask
#ifdef BOARD_V2
		, PCF8574& _portExtender
#endif
		, const int _memAddress
		, const uint8_t _motorOpenPin
		, const uint8_t _motorClosePin
#ifndef BOARD_V2
		, const uint8_t _signalPinOpen = 0
		, const uint8_t _signalPinClosed = 0
#endif
	);
	virtual ~WaterMotorizedValve();

	virtual void setup();
	virtual void loop();

	virtual boolean openValve(const uint8_t _valvesMask = 0xFF, boolean manual = false);
	virtual boolean closeValve();

	virtual boolean isOpen();
	virtual boolean isClosed();

};

#endif /* WATERMOTORIZEDVALVE_H_ */
