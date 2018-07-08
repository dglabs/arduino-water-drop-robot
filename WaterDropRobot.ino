
#include "CommonDefs.h"

#ifdef BOARD_V2
#include "PCF8574.h"
#endif

#include "RobotController.h"
#include "TestController.h"

// RTC and Display
RTC_DS3231 rtc;
DS3232RTC rtcDS3232(false);

LiquidCrystal_I2C lcd(I2C_ADDR_LCD ,LCD_COLS, LCD_ROWS);

#ifdef BOARD_V2
PCF8574 portExtender;
#endif

// Water level meter
WaterLevelMeter waterLevelMeter(WATER_LEVEL_INPUTS);

// Battery monitor
BatteryMonitor batteryMonitor(BATTERY_POWER_SENSOR_PIN);

// Keyboard controls
KeyboardWithISR keyboard(ENCODER_CLK, ENCODER_DT, ENCODER_SW, RobotDisplay::State::ITEM_COUNT);

// Rain handling
RainSensor rainSensor(RAIN_SENSOR_PIN, RAIN_SENSOR_ADDR);

// Water flow volume meter
WaterFlowMeter waterFlowMeter(WTR_VOLUME_ADDR);

// Water in controls
// Water in solenoid valve
WaterInValve waterInValve(VALVE_IN
#ifdef BOARD_V2
		, EX_PIN_WTR_IN_EN
#else
		, WTR_IN_RELAY
#endif
);


// Water out motorized valve
WaterMotorizedValve waterOutValve(VALVE_OUT0 | VALVE_OUT1 | VALVE_OUT2
#ifdef BOARD_V2
		, OUT_VALVE_PINS
		, OUT_VALVES_SIZE
#endif
		, WTR_OUT_ADDR
#ifdef BOARD_V2
		, PIN_MOTOR_PWM0
		, PIN_MOTOR_PWM1
#else
		, WTR_OUT_OPEN
		, WTR_OUT_CLOSE
#endif
);

RainCoverHandler rainCoverHandler(VALVE_RAIN
#ifdef BOARD_V2
		, PIN_MOTOR_PWM0, PIN_MOTOR_PWM1
		, PIN_RAIN_COVER_MOTOR_EN
		, EX_PIN_TILT
#else
		, COVER_MOTOR_POWER_PIN, COVER_MOTOR_DIRECTION_PIN
		, TILT_SENSOR_PIN
#endif
		, COVER_STATE_ADDR);


WeatherManager weatherManager
#ifdef USE_BME280
(
		WIFI_3V_POWER
		, LIGHT_SENSOR_PIN
)
#endif
;

// Scheduler
WaterSchedule schedule(SCHEDULE_EEPROM_ADDR);

#ifdef TEST_MODE
TestController controller = TestController(MAIN_POWER
#ifdef BOARD_V2
		, WIFI_3V_POWER
#endif
);
#else
RobotController controller = RobotController(MAIN_POWER
#ifdef BOARD_V2
		, WIFI_3V_POWER
#endif
		);
#endif /*TEST_MODE*/

void setup () {
	//while (!Serial); // for Leonardo/Micro/Zero
	Serial.begin(57600);

	/*pinMode(MAIN_POWER, OUTPUT);
	digitalWrite(MAIN_POWER, HIGH);

	portExtender.begin(0xFF);

	pinMode(PIN_RAIN_COVER_MOTOR_EN, OUTPUT);
	lcd.init();
	lcd.cursor_off();
	lcd.backlight();
	lcd.setCursor(0, 0);
	lcd.print("Hello LCD");*/
	controller.setup();
}

void loop () {
	controller.loop();
	/*digitalWrite(PIN_RAIN_COVER_MOTOR_EN, LOW);
	delay(1000);
	digitalWrite(PIN_RAIN_COVER_MOTOR_EN, HIGH);
	delay(1000);*/
}
