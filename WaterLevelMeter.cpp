/*
 * WaterLevelMeter.cpp
 *
 *  Created on: 7 июн. 2017 г.
 *      Author: dennis
 */

#include "WaterLevelMeter.h"
#include <EEPROM.h>
#include "EEPROMUtils.h"

//#define _SERIAL_DEBUG

const int BOTTOM_SENSOR = 1;
const int TOP_SENSOR = 0;

WaterLevelMeter::WaterLevelMeter(const uint8_t* _levelPins
#ifdef BOARD_V2
		, const PCF8574& _portExtender
#endif

		):
	levelPins(_levelPins)
#ifdef BOARD_V2
	, portExtender(_portExtender)
#endif
{
	// Pull-up all analog pins to detect disconnected wires
	for (int i = 0; i < LEVEL_SENSOR_COUNT; i++) {
		pinMode(levelPins[i], INPUT);           // input pin with pull-up resistor
		digitalWrite(levelPins[i], HIGH);
	}
}

WaterLevelMeter::~WaterLevelMeter() {
	// TODO Auto-generated destructor stub
}

uint8_t WaterLevelMeter::readLevel() {
	if (digitalRead(levelPins[BOTTOM_SENSOR]) == HIGH && digitalRead(levelPins[TOP_SENSOR]) == HIGH)	// Bottom sensor is disconnected(no water in tank)
		return 0;
	else if (digitalRead(levelPins[BOTTOM_SENSOR]) == LOW && digitalRead(levelPins[TOP_SENSOR]) == HIGH)	// Bottom sensor is up. Top sensor is off (level is between)
		return 50;
	else if (digitalRead(levelPins[BOTTOM_SENSOR]) == LOW && digitalRead(levelPins[TOP_SENSOR]) == LOW)	// Both sensors are up - full tank.
		return 100;
	else return 100; // Prevent water in if levels have undefined values
}


