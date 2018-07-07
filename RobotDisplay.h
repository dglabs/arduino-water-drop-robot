/*
 * RobotDisplay.h
 *
 *  Created on: 10 июн. 2017 г.
 *      Author: dennis
 */

#ifndef ROBOTDISPLAY_H_
#define ROBOTDISPLAY_H_

#include <Arduino.h>
#include <Chrono.h>
#include <RTClib.h>

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

	boolean innerMenuState;

	void printNumber(unsigned long n, uint8_t base);

public:
	RobotDisplay();

	void initialize();
	void turnOnBacklight();
	void turnOffBacklight();
	void powerDown();
	boolean isPowerSaveMode() { return !backlightOn; }

	State getState() { return currentState; }
	void setState(State newState);

	void update(DateTime& now);

	void print(String str, int line = 0);
	void print(unsigned long n, int line = 0);

	void setInnerMenuState(boolean state) { innerMenuState = state; }
	boolean isInnerMenuState() { return innerMenuState; }
};

extern RobotDisplay display;

#endif /* ROBOTDISPLAY_H_ */
