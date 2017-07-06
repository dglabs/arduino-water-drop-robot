/*
 * RobotDisplay.cpp
 *
 *  Created on: 10 июн. 2017 г.
 *      Author: dennis
 */

#include <RTClib.h>
#include "RobotDisplay.h"

RobotDisplay::RobotDisplay(LiquidCrystal_I2C& _lcd, RTC_DS1307& _rtc, WaterLevelMeter& _waterLevelMeter
		, WaterMotorizedValve& _waterOutValve, WaterInValve& _waterInValve) :
	backligtOnChrono(Chrono::SECONDS),
	currentState(State::Dashboard),
	lcd(_lcd),
	rtc(_rtc),
	waterLevelMeter(_waterLevelMeter),
	waterOutValve(_waterOutValve),
	waterInValve(_waterInValve)
{

}

void RobotDisplay::initialize() {
	lcd.init();
	lcd.cursor_off();
	lcd.backlight();
	backlightOn = true;
	backligtOnChrono.restart(0);
}

RobotDisplay::~RobotDisplay() {
	lcd.clear();
	lcd.noBacklight();
	lcd.noDisplay();
}

void RobotDisplay::turnOnBacklight() {
	if (!backligtOnChrono.isRunning())
		lcd.backlight();
	backligtOnChrono.restart();
	backlightOn=true;
}

void RobotDisplay::powerDown() {
	lcd.noBacklight();
	backligtOnChrono.stop();
	backlightOn = false;
}

void RobotDisplay::setState(State newState) {
	currentState = newState;
	lcd.init();
}

RobotDisplay::State RobotDisplay::switchNextState() {
	RobotDisplay::State next = currentState;
	switch (currentState) {
	case Dashboard: next = OutValve; break;
	case OutValve: next = InValve; break;
	case InValve: next = WaterLevel; break;
	case WaterLevel: next = FullTime; break;
	case FullTime: next = Dashboard; break;
	}
	setState(next);
	return next;
}


void RobotDisplay::update(DateTime& now) {
	switch (currentState) {
	case Dashboard: {
	    lcd.setCursor(0, 0);
	    lcd.print("LVL:");
	    lcd.print(waterLevelMeter.readLevel());
	    lcd.print("% ");

	    lcd.setCursor(9, 0);
	    lcd.print("OUT:");
	    lcd.print(waterOutValve.isOpen() ? "ON " : "OFF");

	    lcd.setCursor(9, 1);
	    lcd.print("IN :");
	    lcd.print(waterInValve.isOpen() ? "ON " : "OFF");

	    lcd.setCursor(0, 1);
	    lcd.print(now.hour(), DEC);
	    lcd.print(':');
	    lcd.print(now.minute(), DEC);
	    lcd.print(':');
	    lcd.print(now.second(), DEC);
	    lcd.print(" ");
	} break;
	case OutValve: {
	    lcd.setCursor(0, 0);
	    lcd.print("OUT WTR:");
	    lcd.print(waterOutValve.isOpen() ? "ON " : "OFF");
	    if (waterOutValve.isOpen()) {
	    	lcd.print(waterOutValve.valveOpenSeconds());
	    	lcd.print("s");
	    }
	    else
	    	lcd.print(" PRESS");

	    lcd.setCursor(0, 1);
	    lcd.print("WtrLvl:");
	    lcd.print(waterLevelMeter.readLevel());
	    lcd.print("% ");
	} break;
	case InValve: {
	    lcd.setCursor(0, 0);
	    lcd.print("IN WTR:");
	    lcd.print(waterInValve.isOpen() ? "ON " : "OFF");
	    if (waterInValve.isOpen()) {
	    	lcd.print(waterInValve.valveOpenSeconds());
	    	lcd.print("s");
	    }
	    else lcd.print(" PRESS");

	    lcd.setCursor(0, 1);
	    lcd.print("WtrLvl:");
	    lcd.print(waterLevelMeter.readLevel());
	    lcd.print("% ");
	} break;
	case WaterLevel: {
		waterLevelMeter.readLevel();
	    lcd.setCursor(0, 0);
	    lcd.print('C');
	    for (int i = 0; i < LEVEL_SENSOR_COUNT; i++) {
	    	lcd.print(waterLevelMeter.getCurrentValues()[i]);
	    	if (i < LEVEL_SENSOR_COUNT - 1) lcd.print(';');
	    }
	    lcd.setCursor(0, 1);
	    lcd.print('A');
	    for (int i = 0; i < LEVEL_SENSOR_COUNT; i++) {
	    	lcd.print(waterLevelMeter.getAverageValues()[i]);
	    	if (i < LEVEL_SENSOR_COUNT - 1) lcd.print(';');
	    }
	} break;
	case FullTime: {
	    lcd.setCursor(0, 0);
	    lcd.print(now.day(), DEC);
	    lcd.print(':');
	    lcd.print(now.month(), DEC);
	    lcd.print(':');
	    lcd.print(now.year(), DEC);
	    lcd.print("     ");

	    lcd.setCursor(0, 1);
	    lcd.print(now.hour(), DEC);
	    lcd.print(':');
	    lcd.print(now.minute(), DEC);
	    lcd.print(':');
	    lcd.print(now.second(), DEC);
	    lcd.print("    ");
	} break;
	}

	if (backligtOnChrono.isRunning() && backligtOnChrono.hasPassed(BACKLIGHT_TIME_SECONDS)) {
		powerDown();
	}
}



