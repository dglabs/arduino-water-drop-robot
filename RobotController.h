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

#include "AbstractController.h"
#include "WaterMotorizedValve.h"
#include "WaterInValve.h"
#include "WaterLevelMeter.h"
#include "KeyboardWithISR.h"
#include "RobotDisplay.h"
#include "WaterFlowMeter.h"
#include "RainSensor.h"
#include "RainCoverHandler.h"
#include "WaterSchedule.h"
#include "BatteryMonitor.h"
#include "DS3232RTC.h"
#include "WeatherManager.h"

const long ACTIVE_STATE_TIME_SECONDS = 60;
const int POWER_SAVE_SYCLES = 3;

class RobotController: public AbstractController {
public:
	enum RobotState { Active, PowerSave };

protected:
	Chrono activeStateChrono;
	int powerSaveCyclesCount;
	int temperature;
	RobotState currentState;

	void setCurrentState(RobotState _state);
	virtual boolean checkRainOut();
	virtual boolean processScheduleEvent();
	virtual void prepareWinterOperation();

public:
	RobotController(const uint8_t _mainPowerPin
			, uint8_t _wifi3VPowerPin
			, RTC_DS3231& _rtc
			, KeyboardWithISR& _keyboard
			, WaterLevelMeter& _waterLevelMeter
			, WaterMotorizedValve& _waterOutValve
			, WaterInValve& _waterInValve
			, RobotDisplay& _display
			, WaterFlowMeter& _waterFlowMeter
			, RainSensor& _rainSensor
			, RainCoverHandler& _rainCoverHandler
			, WaterSchedule& _schedule
			, BatteryMonitor& _batteryMonitor
#ifdef BOARD_V2
			, PCF8574& _portExtender
#endif
			, DS3232RTC& rtsDS3232
			, WeatherManager& _weatherManager);
	virtual ~RobotController();

	virtual void setup();

	virtual void loop();

	boolean checkSchedule();
};

#endif /* ROBOTCONTROLLER_H_ */
