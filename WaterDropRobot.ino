#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <LowPower.h>
#include <Chrono.h>

#include "CommonDefs.h"

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
#ifdef BOARD_V2
#include "PCF8574.h"
#endif
#include "DS3232RTC.h"
#include "WeatherManager.h"

// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <RTClib.h>

// RTC and Display
RTC_DS3231 rtc;
DS3232RTC rtcDS3232(false);

#ifdef BOARD_V2
PCF8574 portExtender;
#endif

LiquidCrystal_I2C lcd(0x3F,LCD_COLS,LCD_ROWS);

const uint8_t LIGHT_SENSOR_PIN = A1;
WeatherManager weatherManager(rtc
#ifdef USE_BME280
		, WIFI_3V_POWER
		, LIGHT_SENSOR_PIN
#else
		, rtcDS3232
#endif
);

// EPROM addresses
const int WTR_OUT_ADDR = 0;
const int WTR_LEVEL_ADDR = WTR_OUT_ADDR + STORAGE_SIZE_WATER_VALVE;
const int WTR_VOLUME_ADDR = WTR_LEVEL_ADDR + STORAGE_SIZE_WATER_LEVEL;
const int COVER_STATE_ADDR = WTR_VOLUME_ADDR + STORAGE_SIZE_WATER_VOLUME;
const int RAIN_SENSOR_ADDR = COVER_STATE_ADDR + STORAGE_SIZE_COVER_HANDLER;

const int SCHEDULE_EEPROM_ADDR = 0x200;	// 512 bytes from start
const int SCHEDULE_EEPROM_SIZE = 0x200;	// 512 bytes length

const int LOG_EEPROM_ADDR = 0x0;	// 0 bytes from start
const int LOG_EEPROM_SIZE = 0x1000 - LOG_EEPROM_ADDR - 1;	// 4K length

// Water out motorized valve
const uint8_t MOTOR_PWM0 = 5;
const uint8_t MOTOR_PWM1 = 6;
//const uint8_t WTR_OUT_SIGNAL_OPEN = 4;
//const uint8_t WTR_OUT_SIGNAL_CLOSED = 11;
WaterMotorizedValve waterOutValve(VALVE_OUT0 | VALVE_OUT1 | VALVE_OUT2
#ifdef BOARD_V2
		, portExtender
#endif
		, WTR_OUT_ADDR
		, MOTOR_PWM0
		, MOTOR_PWM1);

// Water in controls
// Water in solenoid valve
const uint8_t WTR_IN_RELAY = 12;
WaterInValve waterInValve(VALVE_IN
#ifdef BOARD_V2
		, portExtender
#endif
		, WTR_IN_RELAY);

// Water level meter
const uint8_t WATER_LEVEL_INPUTS[] = { A2, A3 };
WaterLevelMeter waterLevelMeter(WATER_LEVEL_INPUTS
#ifdef BOARD_V2
		, portExtender
#endif
		);

// Keyboard controls
const uint8_t ENCODER_CLK = 2;
const uint8_t ENCODER_DT  = 8;
const uint8_t ENCODER_SW  = 7;
KeyboardWithISR keyboard(ENCODER_CLK, ENCODER_DT, ENCODER_SW, RobotDisplay::State::ITEM_COUNT);

// Water flow volume meter
WaterFlowMeter waterFlowMeter(WTR_VOLUME_ADDR, rtc);

// Rain handling
RainSensor rainSensor(A7, RAIN_SENSOR_ADDR, rtc);
#ifndef BOARD_V2
const uint8_t COVER_MOTOR_DIRECTION_PIN = 9;
const uint8_t COVER_MOTOR_POWER_PIN = 10;
const uint8_t TILT_SENSOR_PIN = 15;	// A1 pin in digital mode
#else
const uint8_t TILT_SENSOR_PIN = 7;	// Port Extended P7
#endif
RainCoverHandler rainCoverHandler(VALVE_RAIN
#ifdef BOARD_V2
		, portExtender
		, MOTOR_PWM0, MOTOR_PWM1
#else
		, COVER_MOTOR_POWER_PIN, COVER_MOTOR_DIRECTION_PIN
#endif
		, TILT_SENSOR_PIN
		, COVER_STATE_ADDR);

// Battery monitor
const uint8_t BATTERY_POWER_SENSOR_PIN = A0;
BatteryMonitor batteryMonitor(BATTERY_POWER_SENSOR_PIN);

// Scheduler
WaterSchedule schedule(SCHEDULE_EEPROM_ADDR, rtc, rainSensor, waterFlowMeter);

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
		, WIFI_3V_POWER
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
#ifdef BOARD_V2
		, portExtender
#endif
		, rtcDS3232
		, weatherManager);
#else
RobotController controller = RobotController(MAIN_POWER
		, WIFI_3V_POWER
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
#ifdef BOARD_V2
		, portExtender
#endif
		, rtcDS3232
		, weatherManager);
#endif /*TEST_MODE*/

void setup () {
	//while (!Serial); // for Leonardo/Micro/Zero
	Serial.begin(57600);

	controller.setup();
}

void loop () {
	controller.loop();
}
