#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <LowPower.h>
#include <Chrono.h>

#include "WaterMotorizedValve.h"
#include "WaterLevelMeter.h"
#include "KeyboardWithISR.h"
#include "RobotDisplay.h"
#include "WaterFlowMeter.h"
#include "RobotController.h"
#include "RainSensor.h"
#include "RainCoverHandler.h"
#include "WaterSchedule.h"

// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <Wire.h>
#include <RTClib.h>

// RTC and Display
RTC_DS1307 rtc;
LiquidCrystal_I2C lcd(0x3F,16,2);

// RTC addresses
const int WTR_OUT_ADDR = 0;
const int WTR_LEVEL_ADDR = WTR_OUT_ADDR + STORAGE_SIZE_WATER_VALVE;
const int WTR_VOLUME_ADDR = WTR_LEVEL_ADDR + STORAGE_SIZE_WATER_LEVEL;
const int COVER_STATE_ADDR = WTR_VOLUME_ADDR + STORAGE_SIZE_WATER_VOLUME;
const int RAIN_SENSOR_ADDR = COVER_STATE_ADDR + STORAGE_SIZE_COVER_HANDLER;

// EPROM addresses
const int SCHEDULE_EEPROM_ADDR = RAIN_SENSOR_ADDR + STORAGE_SIZE_RAIN_SENSOR;

// Main power pin
const uint8_t MAIN_POWER = 13;

// Water in controls
const uint8_t WTR_IN_RELAY = 12;

// Water out motorized valve
const uint8_t WTR_OUT_ON__OFF_RELAY = 10;
const uint8_t WTR_OUT_POWER_RELAY = 11;
const uint8_t WTR_OUT_SIGNAL_IN = 4;
WaterMotorizedValve waterOutValve(WTR_OUT_ADDR, WTR_OUT_ON__OFF_RELAY, WTR_OUT_POWER_RELAY, WTR_OUT_SIGNAL_IN);

// Water in solenoid valve
const uint8_t WRT_IN_RELAY = 12;
WaterInValve waterInValve(WRT_IN_RELAY);

// Water level meter
const uint8_t WATER_LEVEL_INPUTS[] = { A0, A1, A2, A3 };
const int WATER_LEVEL_AVERAGES[] = { 538, 538, 538, 540 };
WaterLevelMeter waterLevelMeter(WATER_LEVEL_INPUTS, WATER_LEVEL_AVERAGES, WTR_LEVEL_ADDR);


// Keyboard controls
const uint8_t PIN_KEYS[] = { 2, 5 };
KeyboardWithISR keyboard(PIN_KEYS);

// Water flow volume meter
WaterFlowMeter waterFlowMeter(WTR_VOLUME_ADDR);


// Rain handling
RainSensor rainSensor(A7, RAIN_SENSOR_ADDR, rtc);
const uint8_t COVER_MOTOR_POWER_PIN = 8;
const uint8_t COVER_MOTOR_DIRECTION_PIN = 7;
const uint8_t TILT_SENSOR_PIN = 6;
RainCoverHandler rainCoverHandler(COVER_MOTOR_POWER_PIN, COVER_MOTOR_DIRECTION_PIN, TILT_SENSOR_PIN, COVER_STATE_ADDR);

// Scheduler
WaterSchedule schedule(SCHEDULE_EEPROM_ADDR, rtc, rainSensor);


// Display class
RobotDisplay display(lcd
		, rtc
		, waterLevelMeter
		, waterOutValve
		, waterInValve
		, waterFlowMeter
		, rainSensor
		, rainCoverHandler);
RobotController controller(MAIN_POWER
		, rtc
		, keyboard
		, waterLevelMeter
		, waterOutValve
		, waterInValve
		, display
		, waterFlowMeter
		, rainSensor
		, rainCoverHandler
		, schedule);

void setup () {
	while (!Serial); // for Leonardo/Micro/Zero
	Serial.begin(57600);
	
	delay(5000);
	if (! rtc.begin()) {
	Serial.println("Couldn't find RTC");
		while (1);
	}

	if (! rtc.isrunning()) {
	Serial.println("RTC is NOT running!");
		// following line sets the RTC to the date & time this sketch was compiled
		rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
		// This line sets the RTC with an explicit date & time, for example to set
		// January 21, 2014 at 3am you would call:
		// rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
	}

	//if (schedule.isInActiveDateRange())
		waterOutValve.closeValve();
	/*else {
		// Prepare system to winter operation
		if (!waterOutValve.isOpen())
			waterOutValve.openValve();
		waterInValve.closeValve();
		if (rainCoverHandler.isCoverOpen())
			rainCoverHandler.closeCover();
	}*/


	display.initialize();
	lcd.print("Test");
}

void loop () {
	controller.loop();
	//delay(1000);
}
