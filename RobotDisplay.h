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

// Date and time functions using a DS1307 RTC connected via I2C and Wire lib

const long BACKLIGHT_TIME_SECONDS = 30;

class RobotDisplay {
public:
	enum State { Dashboard, OutValve, InValve, WaterLevel, FullTime };
protected:
	Chrono backligtOnChrono;
	State currentState;
	boolean backlightOn;

	LiquidCrystal_I2C& lcd;
	RTC_DS1307& rtc;

	WaterLevelMeter& waterLevelMeter;
	WaterMotorizedValve& waterOutValve;
	WaterInValve& waterInValve;
	WaterFlowMeter& waterFlowMeter;

public:
	RobotDisplay(LiquidCrystal_I2C& _lcd, RTC_DS1307& _rtc, WaterLevelMeter& _waterLevelMeter
			, WaterMotorizedValve& _waterOutValve, WaterInValve& _waterInValve, WaterFlowMeter& _waterFlowMeter);
	virtual ~RobotDisplay();

	void initialize();
	void turnOnBacklight();
	void powerDown();
	boolean isPowerSaveMode() { return !backlightOn; }

	State getState() { return currentState; }
	void setState(State newState);
	RobotDisplay::State switchNextState();

	void update(DateTime& now);
};

#endif /* ROBOTDISPLAY_H_ */
