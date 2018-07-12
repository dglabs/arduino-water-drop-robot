/*
 * RobotDisplay.cpp
 *
 *  Created on: 10 июн. 2017 г.
 *      Author: dennis
 */

#include "WaterDropRobot.h"

RobotDisplay::RobotDisplay() :
	backligtOnChrono(Chrono::SECONDS)
	, currentState(State::Dashboard)
	, backlightOn(false)
	, innerMenuState(false)
{

}

void RobotDisplay::initialize() {
	lcd.init();
	lcd.cursor_off();
	lcd.backlight();

	keyboard.setMaxPos(State::ITEM_COUNT);
	backlightOn = true;
	backligtOnChrono.restart(0);
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
	keyboard.setPos(currentState);
	lcd.init();
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
	    else lcd.print(F("LOW POWER!"));

	    lcd.setCursor(9, 0);
	    lcd.print(F("OUT:"));
	    lcd.print(waterOutValve.isOpen() ? F("ON ") : F("OFF"));

	    lcd.setCursor(9, 1);
	    lcd.print(F("IN :"));
	    lcd.print(waterInValve.isOpen() ? F("ON ") : F("OFF"));

	    lcd.setCursor(0, 1);
	    lcd.print(now.hour(), DEC);
	    lcd.print(':');
	    lcd.print(now.minute(), DEC);
	    lcd.print(':');
	    lcd.print(now.second(), DEC);
	    lcd.print(" ");

		if (LCD_ROWS > 2) {
			lcd.setCursor(0, 2);
			lcd.print(F("TODAY: ")); lcd.print(waterFlowMeter.getStatistics().today.litres, DEC); lcd.print(F("L"));
		    lcd.setCursor(0, 3);
			lcd.print(F("BATT: ")); lcd.print(batteryMonitor.getVdd()); lcd.print(F("V "));
			lcd.print(rtcDS3232.temperature() / 4); lcd.print("C  ");
		}
	} break;
	case OutValve: {
		if (!innerMenuState) {
			lcd.setCursor(0, 0);
			lcd.print(F("IRRIGATE:"));
			lcd.print(waterOutValve.getStateString());

			if (waterOutValve.isOpen()) {
				lcd.setCursor(0, 1);
				lcd.print(waterOutValve.valveOpenSeconds());
				lcd.print("s");

				if (LCD_ROWS > 2) {
					lcd.setCursor(0, 2);
					lcd.print(F("VOL:"));
					lcd.print(waterFlowMeter.getVolumeFromStart());
					lcd.print(F("L "));
					lcd.print(waterFlowMeter.getVolumePerMinute());
					lcd.print(F("L/m   "));

					lcd.setCursor(0, 3);
					lcd.print(F("LEVEL:"));
					lcd.print(waterLevelMeter.readLevel());
					lcd.print(F("% "));
				}
			}
			else {
				lcd.setCursor(0, 1);
				lcd.print(F("LEVEL:"));
				lcd.print(waterLevelMeter.readLevel());
				lcd.print(F("% "));
			}
		}
		else {
			lcd.setCursor(0, 0);
			lcd.print(F("POUR VOLUME:   "));
			lcd.setCursor(0, 1);
			lcd.print(waterOutValve.getSelectedVolume());
			lcd.print(F("L     "));
			if (LCD_ROWS > 2) {
				lcd.setCursor(0, 3);
				lcd.print(F("LEVEL:"));
				lcd.print(waterLevelMeter.readLevel());
				lcd.print(F("% "));
			}
		}
	} break;
	case InValve: {
	    lcd.setCursor(0, 0);
	    lcd.print(F("FILL IN:"));
	    lcd.print(waterInValve.isOpen() ? F("ON ") : F("OFF"));
	    if (waterInValve.isOpen()) {
	    	lcd.print(waterInValve.valveOpenSeconds());
	    	lcd.print("s");
	    }
	    else lcd.print("      ");

	    lcd.setCursor(0, 1);
	    lcd.print(F("LEVEL:"));
	    lcd.print(waterLevelMeter.readLevel());
	    lcd.print(F("% "));
	} break;
	case RainControl: {
	    lcd.setCursor(0, 0);
	    lcd.print(F("RAIN: ")); lcd.print(rainSensor.getIntensityString());
	    lcd.setCursor(0, 1);
	    lcd.print(F("COVER: ")); lcd.print(rainCoverHandler.getStateString());
		if (LCD_ROWS > 2) {
			lcd.setCursor(0, 2);
		    LastRainInfo info;
		    rainSensor.getLastRainInfo(info);
		    if (info.startTime > 0) {
		    	DateTime startTime(info.startTime);
		    	lcd.print(F("LAST:")); lcd.print(startTime.day()); lcd.print(F("/")); lcd.print(startTime.month()); lcd.print(F(" "));
			    lcd.print(startTime.hour()); lcd.print(":"); lcd.print(startTime.minute()); lcd.print(" ");
			    lcd.setCursor(0, 3);
			    lcd.print(info.duration / 60); lcd.print(F("m ")); lcd.print(RainSensor::getIntensityAsString(info.intensity));
		    }
		    else {
		    	lcd.print(F("NO LAST RAIN"));
		    	lcd.setCursor(0, 3);
			    lcd.print("              ");
		    }

		}
	} break;
	case Power: {
	    lcd.setCursor(0, 0);
	    lcd.print(F("BATT: ")); lcd.print(batteryMonitor.getVdd()); lcd.print(F("V"));
	    lcd.setCursor(0, 1);
	    lcd.print(F("VCC : ")); lcd.print(batteryMonitor.getVcc()); lcd.print(F("V"));
	} break;
	case FullTime: {
	    lcd.setCursor(0, 0);
	    lcd.print(now.day(), DEC);
	    lcd.print('/');
	    lcd.print(now.month(), DEC);
	    lcd.print('/');
	    lcd.print(now.year(), DEC);
	    lcd.print(" ");
	    lcd.print(weatherManager.curTemperature); lcd.print(F("C"));

	    lcd.setCursor(0, 1);
	    lcd.print(now.hour(), DEC);
	    lcd.print(':');
	    lcd.print(now.minute(), DEC);
	    lcd.print(':');
	    lcd.print(now.second(), DEC);
	    lcd.print("   ");

	    if (LCD_ROWS > 2) {
		    lcd.setCursor(0, 2);
		    lcd.print(F("MIN")); lcd.print(weatherManager.minTemperature); lcd.print(F("C "));
		    lcd.print(F("MAX")); lcd.print(weatherManager.maxTemperature); lcd.print(F("C  "));

#ifdef USE_BME280
		    lcd.setCursor(0, 2);
		    lcd.print(F("HUM:")); lcd.print(weatherManager.curHumidity); lcd.print(F("% P:"));
		    lcd.print(F("% P:")); lcd.print(weatherManager.curPressure); lcd.print(F("Hg/m  "));
#endif
	    }
	} break;
	case Statistics: {
	    lcd.setCursor(0, 0);
	    lcd.print(F("POUR TODAY: ")); lcd.print(waterFlowMeter.getStatistics().today.litres, DEC); lcd.print(F("L"));
	    lcd.setCursor(0, 1);
	    lcd.print(F("MONTH: ")); lcd.print(waterFlowMeter.getStatistics().lastMonth.litres, DEC); lcd.print(F("L"));
		if (LCD_ROWS > 2) {
			lcd.setCursor(0, 2);
		    lcd.print(F("YEAR: ")); lcd.print(waterFlowMeter.getStatistics().lastYear.litres, DEC); lcd.print(F("L"));
		    lcd.setCursor(0, 3);
		    lcd.print(F("TOTAL: ")); lcd.print(waterFlowMeter.getStatistics().total.litres, DEC); lcd.print(F("L"));
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

#ifdef _DEBUG
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

void printDateTime(const char* title, DateTime& dt) {
	Serial.print(title);
    Serial.print(dt.day(), DEC);
    Serial.print('/');
    Serial.print(dt.month(), DEC);
    Serial.print('/');
    Serial.print(dt.year(), DEC);
    Serial.print(" ");

    Serial.print(dt.hour(), DEC);
    Serial.print(':');
    Serial.print(dt.minute(), DEC);
    Serial.print(':');
    Serial.println(dt.second(), DEC);
}
#endif

// Display class
RobotDisplay display;
