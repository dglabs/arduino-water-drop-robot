#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <LowPower.h>
#include <Chrono.h>

#include "WaterMotorizedValve.h"
#include "WaterLevelMeter.h"
#include "KeyboardWithISR.h"
#include "RobotDisplay.h"
#include "WaterFlowMeter.h"
#include "AbstractController.h"
#include "RobotController.h"
#include "TestController.h"
#include "RainSensor.h"
#include "RainCoverHandler.h"
#include "WaterSchedule.h"
#include "BatteryMonitor.h"
#include "DS3232RTC.h"

// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <RTClib.h>

//#define TEST_MODE

// RTC and Display
RTC_DS3231 rtc;
DS3232RTC rtcDS3232(false);

LiquidCrystal_I2C lcd(0x3F,LCD_COLS,LCD_ROWS);

// EPROM addresses
const int WTR_OUT_ADDR = 0;
const int WTR_LEVEL_ADDR = WTR_OUT_ADDR + STORAGE_SIZE_WATER_VALVE;
const int WTR_VOLUME_ADDR = WTR_LEVEL_ADDR + STORAGE_SIZE_WATER_LEVEL;
const int COVER_STATE_ADDR = WTR_VOLUME_ADDR + STORAGE_SIZE_WATER_VOLUME;
const int RAIN_SENSOR_ADDR = COVER_STATE_ADDR + STORAGE_SIZE_COVER_HANDLER;

const int SCHEDULE_EEPROM_ADDR = 0x200;	// 512 bytes from start
const int SCHEDULE_EEPROM_SIZE = 0x200;	// 512 bytes length

const int LOG_EEPROM_ADDR = 0x400;	// 1024 bytes from start
const int LOG_EEPROM_SIZE = 0x8000 - LOG_EEPROM_ADDR - 1;	// 31K length

// Main power pin
const uint8_t MAIN_POWER = 13;

// Water out motorized valve
const uint8_t WTR_OUT_OPEN = 5;
const uint8_t WTR_OUT_CLOSE = 6;
const uint8_t WTR_OUT_SIGNAL_OPEN = 4;
const uint8_t WTR_OUT_SIGNAL_CLOSED = 11;
WaterMotorizedValve waterOutValve(VALVE_OUT0 | VALVE_OUT1 | VALVE_OUT2, WTR_OUT_ADDR, WTR_OUT_OPEN, WTR_OUT_CLOSE/*, WTR_OUT_SIGNAL_OPEN, WTR_OUT_SIGNAL_CLOSED*/);

// Water in controls
// Water in solenoid valve
const uint8_t WTR_IN_RELAY = 12;
WaterInValve waterInValve(VALVE_IN, WTR_IN_RELAY);

// Water level meter
const uint8_t WATER_LEVEL_INPUTS[] = { A2, A3 };
WaterLevelMeter waterLevelMeter(WATER_LEVEL_INPUTS/*, WATER_LEVEL_AVERAGES, WTR_LEVEL_ADDR*/);

// Keyboard controls
const uint8_t ENCODER_CLK = 2;
const uint8_t ENCODER_DT  = 8;
const uint8_t ENCODER_SW  = 7;
KeyboardWithISR keyboard(ENCODER_CLK, ENCODER_DT, ENCODER_SW, RobotDisplay::State::ITEM_COUNT);

// Water flow volume meter
WaterFlowMeter waterFlowMeter(WTR_VOLUME_ADDR, rtc);

// Rain handling
RainSensor rainSensor(A7, RAIN_SENSOR_ADDR, rtc);
const uint8_t COVER_MOTOR_DIRECTION_PIN = 9;
const uint8_t COVER_MOTOR_POWER_PIN = 10;
const uint8_t TILT_SENSOR_PIN = 15;	// A1 pin in digital mode
RainCoverHandler rainCoverHandler(VALVE_RAIN, COVER_MOTOR_POWER_PIN, COVER_MOTOR_DIRECTION_PIN, TILT_SENSOR_PIN, COVER_STATE_ADDR);

// Battery monitor
const uint8_t BATTERY_POWER_SENSOR_PIN = A0;
BatteryMonitor batteryMonitor(BATTERY_POWER_SENSOR_PIN);

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
		, rainCoverHandler
		, batteryMonitor
		, rtcDS3232);

#ifdef TEST_MODE
TestController controller = TestController(MAIN_POWER
		, rtc
		, keyboard
		, waterLevelMeter
		, waterOutValve
		, waterInValve
		, display
		, waterFlowMeter
		, rainSensor
		, rainCoverHandler
		, schedule
		, batteryMonitor
		, rtcDS3232);
#else
RobotController controller = RobotController(MAIN_POWER
		, rtc
		, keyboard
		, waterLevelMeter
		, waterOutValve
		, waterInValve
		, display
		, waterFlowMeter
		, rainSensor
		, rainCoverHandler
		, schedule
		, batteryMonitor
		, rtcDS3232);
#endif /*TEST_MODE*/

void setup () {
	//while (!Serial); // for Leonardo/Micro/Zero
	Serial.begin(57600);

	controller.setup();
}

void loop () {
	controller.loop();
	//delay(1000);
	
	/*keyboard.tick();
	if (keyboard.isRotated()) Serial.println("Rotated");
	if (keyboard.isPressed()) Serial.println("Pressed");*/

	/*digitalWrite(MAIN_POWER, HIGH);	// Turn off all peripherals
	delay(1000);
	digitalWrite(MAIN_POWER, LOW);	// Turn off all peripherals
	delay(1000);*/
	
}
