/*
 * RobotDisplay.h
 *
 *  Created on: 10 июн. 2017 г.
 *      Author: dennis
 */

#ifndef ROBOTDISPLAY_H_
#define ROBOTDISPLAY_H_

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Chrono.h>

#include "WaterMotorizedValve.h"
#include "WaterInValve.h"
#include "WaterLevelMeter.h"
#include "KeyboardWithISR.h"
#include "WaterFlowMeter.h"
#include "RainSensor.h"
#include "RainCoverHandler.h"
#include "BatteryMonitor.h"
#include "DS3232RTC.h"

const int LCD_COLS = 16;
const int LCD_ROWS = 4;

// Date and time functions using a DS1307 RTC connected via I2C and Wire lib

const long BACKLIGHT_TIME_SECONDS = 30;

class RobotDisplay {
public:
	enum State { Dashboard, OutValve, InValve, RainControl, Power, FullTime, Statistics, ITEM_COUNT };
protected:
	Chrono backligtOnChrono;
	State currentState;
	boolean backlightOn;

	LiquidCrystal_I2C& lcd;
	RTC_DS3231& rtc;
	DS3232RTC& rtcDS3232;

	WaterLevelMeter& waterLevelMeter;
	WaterMotorizedValve& waterOutValve;
	WaterInValve& waterInValve;
	WaterFlowMeter& waterFlowMeter;
	RainSensor& rainSensor;
	RainCoverHandler& rainCoverHandler;
	BatteryMonitor& batteryMonitor;

	void printNumber(unsigned long n, uint8_t base);

public:
	RobotDisplay(LiquidCrystal_I2C& _lcd, RTC_DS3231& _rtc, WaterLevelMeter& _waterLevelMeter
			, WaterMotorizedValve& _waterOutValve, WaterInValve& _waterInValve, WaterFlowMeter& _waterFlowMeter
			, RainSensor& _rainSensor, RainCoverHandler& _rainCoverHandler, BatteryMonitor& _batteryMonitor, DS3232RTC& _rtcDS3232);
	virtual ~RobotDisplay();

	void initialize();
	void turnOnBacklight();
	void turnOffBacklight();
	void powerDown();
	boolean isPowerSaveMode() { return !backlightOn; }

	State getState() { return currentState; }
	void setState(State newState);
	RobotDisplay::State switchNextState();
	RobotDisplay::State switchPrevState();

	void update(DateTime& now);

	void print(String str, int line = 0);
	void print(unsigned long n, int line = 0);
};

#endif /* ROBOTDISPLAY_H_ */
