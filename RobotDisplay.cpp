/*
 * RobotDisplay.cpp
 *
 *  Created on: 10 июн. 2017 г.
 *      Author: dennis
 */

#include <RTClib.h>
#include "RobotDisplay.h"
#include <LiquidCrystal_I2C.h>

RobotDisplay::RobotDisplay(LiquidCrystal_I2C& _lcd, RTC_DS3231& _rtc, WaterLevelMeter& _waterLevelMeter
		, WaterMotorizedValve& _waterOutValve, WaterInValve& _waterInValve, WaterFlowMeter& _waterFlowMeter
		, RainSensor& _rainSensor, RainCoverHandler& _rainCoverHandler, BatteryMonitor& _batteryMonitor, DS3232RTC& _rtcDS3232) :
	backligtOnChrono(Chrono::SECONDS)
	, currentState(State::Dashboard)
	, backlightOn(false)
	, lcd(_lcd)
	, rtc(_rtc)
	, waterLevelMeter(_waterLevelMeter)
	, waterOutValve(_waterOutValve)
	, waterInValve(_waterInValve)
	, waterFlowMeter(_waterFlowMeter)
	, rainSensor(_rainSensor)
	, rainCoverHandler(_rainCoverHandler)
	, batteryMonitor(_batteryMonitor)
	, rtcDS3232(_rtcDS3232)
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

void RobotDisplay::turnOffBacklight() {
	lcd.noBacklight();
	backlightOn=false;
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
	RobotDisplay::State next = (RobotDisplay::State)(((int)currentState + 1) % (int)RobotDisplay::State::ITEM_COUNT);
	setState(next);
	return next;
}

// Dashboard, OutValve, InValve, WaterLevel, RainControl, Power

RobotDisplay::State RobotDisplay::switchPrevState() {
	RobotDisplay::State next = (int)currentState == 0 ? ((RobotDisplay::State)((int)RobotDisplay::State::ITEM_COUNT - 1)) :
		(RobotDisplay::State)((int)currentState - 1);
	setState(next);
	return next;
}


void RobotDisplay::update(DateTime& now) {
	switch (currentState) {
	case Dashboard: {
	    lcd.setCursor(0, 0);
	    if (!batteryMonitor.isPowerLow()) {
			lcd.print("LVL:");
			lcd.print(waterLevelMeter.readLevel());
			lcd.print("% ");
	    }
	    else lcd.print("LOW POWER!");

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

		if (LCD_ROWS > 2) {
			lcd.setCursor(16, 0); // lcd.setCursor(0, 2);
			lcd.print("TODAY: "); lcd.print(waterFlowMeter.getStatistics().today.litres, DEC); lcd.print("L");
		    lcd.setCursor(16, 1);  // lcd.setCursor(0, 3);
			lcd.print("BATT: "); lcd.print(batteryMonitor.getVdd()); lcd.print("V ");
			lcd.print(rtcDS3232.temperature() / 4); lcd.print("C  ");
		}
	} break;
	case OutValve: {
	    lcd.setCursor(0, 0);
	    lcd.print("OUT WTR:");
	    lcd.print(waterOutValve.getStateString());
    	lcd.print("    ");

	    if (waterOutValve.isOpen()) {
	    	lcd.print(waterOutValve.valveOpenSeconds());
	    	lcd.print("s");

		    lcd.setCursor(0, 1);
		    lcd.print("VOL:");
		    lcd.print(waterFlowMeter.getVolumeFromStart());
		    lcd.print("L ");
		    lcd.print(waterFlowMeter.getVolumePerMinute());
		    lcd.print("L/m");
	    	lcd.print("  ");

			if (LCD_ROWS > 2) {
			    lcd.setCursor(16, 1);
			    lcd.print("LEVEL:");
			    lcd.print(waterLevelMeter.readLevel());
			    lcd.print("% ");
			}
	    }
	    else {
		    lcd.setCursor(0, 1);
		    lcd.print("L:");
		    lcd.print(waterLevelMeter.readLevel());
		    lcd.print("% ");
	    }

	} break;
	case InValve: {
	    lcd.setCursor(0, 0);
	    lcd.print("IN WTR:");
	    lcd.print(waterInValve.isOpen() ? "ON " : "OFF");
	    if (waterInValve.isOpen()) {
	    	lcd.print(waterInValve.valveOpenSeconds());
	    	lcd.print("s");
	    }
	    else lcd.print("      ");

	    lcd.setCursor(0, 1);
	    lcd.print("WtrLvl:");
	    lcd.print(waterLevelMeter.readLevel());
	    lcd.print("% ");
	} break;
	case RainControl: {
	    lcd.setCursor(0, 0);
	    lcd.print("RAIN: "); lcd.print(rainSensor.getIntensityString());
	    lcd.setCursor(0, 1);
	    lcd.print("COVER: "); lcd.print(rainCoverHandler.getStateString());
		if (LCD_ROWS > 2) {
			lcd.setCursor(16, 0); //lcd.setCursor(0, 2);
		    LastRainInfo info;
		    rainSensor.getLastRainInfo(info);
		    if (info.startTime > 0) {
		    	DateTime startTime(info.startTime);
		    	lcd.print("LAST:"); lcd.print(startTime.day()); lcd.print("/"); lcd.print(startTime.month()); lcd.print(" ");
			    lcd.print(startTime.hour()); lcd.print(":"); lcd.print(startTime.minute()); lcd.print(" ");
			    lcd.setCursor(16, 1); // lcd.setCursor(0, 3);
			    lcd.print(info.duration / 60); lcd.print("m "); lcd.print(RainSensor::getIntensityAsString(info.intensity));
		    }
		    else {
		    	lcd.print("NO LAST RAIN");
		    	lcd.setCursor(16, 1); // lcd.setCursor(0, 3);
			    lcd.print("              ");
		    }

		}
	} break;
	case Power: {
	    lcd.setCursor(0, 0);
	    lcd.print("BATT: "); lcd.print(batteryMonitor.getVdd()); lcd.print("V");
	    lcd.setCursor(0, 1);
	    lcd.print("VCC : "); lcd.print(batteryMonitor.getVcc()); lcd.print("V");
	} break;
	case FullTime: {
	    lcd.setCursor(0, 0);
	    lcd.print(now.day(), DEC);
	    lcd.print('/');
	    lcd.print(now.month(), DEC);
	    lcd.print('/');
	    lcd.print(now.year(), DEC);
	    lcd.print(" ");
	    lcd.print(rtcDS3232.temperature() / 4); lcd.print("C");

	    lcd.setCursor(0, 1);
	    lcd.print(now.hour(), DEC);
	    lcd.print(':');
	    lcd.print(now.minute(), DEC);
	    lcd.print(':');
	    lcd.print(now.second(), DEC);
	    lcd.print("   ");
	} break;
	case Statistics: {
	    lcd.setCursor(0, 0);
	    lcd.print("POUR TODAY: "); lcd.print(waterFlowMeter.getStatistics().today.litres, DEC); lcd.print("L");
	    lcd.setCursor(0, 1);
	    lcd.print("MONTH: "); lcd.print(waterFlowMeter.getStatistics().lastMonth.litres, DEC); lcd.print("L");
		if (LCD_ROWS > 2) {
			lcd.setCursor(16, 0); // lcd.setCursor(0, 2);
		    lcd.print("YEAR: "); lcd.print(waterFlowMeter.getStatistics().lastYear.litres, DEC); lcd.print("L");
		    lcd.setCursor(16, 1); // lcd.setCursor(0, 3);
		    lcd.print("TOTAL: "); lcd.print(waterFlowMeter.getStatistics().total.litres, DEC); lcd.print("L");
		}
	} break;
	}

	if (backligtOnChrono.isRunning() && backligtOnChrono.hasPassed(BACKLIGHT_TIME_SECONDS)) {
		powerDown();
	}
}

void RobotDisplay::print(String str, int line /*=0*/) {
    lcd.setCursor(0, line);
    lcd.print(str);
}

void RobotDisplay::print(unsigned long n, int line /*=0*/) {
    lcd.setCursor(0, line);
    printNumber(n, 10);
}

void RobotDisplay::printNumber(unsigned long n, uint8_t base)
{
  char buf[8 * sizeof(long) + 1]; // Assumes 8-bit chars plus zero byte.
  char *str = &buf[sizeof(buf) - 1];

  *str = '\0';

  // prevent crash if called with base == 1
  if (base < 2) base = 10;

  do {
    char c = n % base;
    n /= base;

    *--str = c < 10 ? c + '0' : c + 'A' - 10;
  } while(n);

  lcd.print(str);
}

