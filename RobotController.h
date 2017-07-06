/*
 * RobotController.h
 *
 *  Created on: 10 июн. 2017 г.
 *      Author: dennis
 */

#ifndef ROBOTCONTROLLER_H_
#define ROBOTCONTROLLER_H_

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Chrono.h>

#include "WaterMotorizedValve.h"
#include "WaterInValve.h"
#include "WaterLevelMeter.h"
#include "KeyboardWithISR.h"
#include "RobotDisplay.h"

const long ACTIVE_STATE_TIME_SECONDS = 60;
const long POWER_SAVE_SYCLES = 4;

class RobotController {
public:
	enum RobotState { Active, PowerSave };

protected:

	RobotState currentState;
	Chrono activeStateChrono;
	int powerSaveCyclesCount;
	DateTime now;

	const uint8_t mainPowerPin;
	RTC_DS1307& rtc;
	KeyboardWithISR& keyboard;
	WaterLevelMeter& waterLevelMeter;
	WaterMotorizedValve& waterOutValve;
	WaterInValve& waterInValve;
	RobotDisplay& display;

	void setCurrentState(RobotState _state);
public:
	RobotController(const uint8_t _mainPowerPin,
			RTC_DS1307& _rtc,
			KeyboardWithISR& _keyboard,
			WaterLevelMeter& _waterLevelMeter,
			WaterMotorizedValve& _waterOutValve,
			WaterInValve& _waterInValve,
			RobotDisplay& _display);
	virtual ~RobotController();

	void loop();

	void startWaterOut();
	void startWaterIn();
	boolean checkWaterLevel();
	boolean checkSchedule();
};

#endif /* ROBOTCONTROLLER_H_ */
