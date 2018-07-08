/*
 * WaterLevelMeter.cpp
 *
 *  Created on: 7 июн. 2017 г.
 *      Author: dennis
 */

#include "WaterLevelMeter.h"
#include "WaterDropRobot.h"
#include "EEPROMUtils.h"

//#define _SERIAL_DEBUG

#ifndef BOARD_V2
const int BOTTOM_SENSOR = 1;
const int TOP_SENSOR = 0;
#endif

WaterLevelMeter::WaterLevelMeter(const uint8_t* _levelPins):
	levelPins(_levelPins)
{
}

void WaterLevelMeter::setup() {
	// Pull-up all analog pins to detect disconnected wires
	for (int i = 0; i < LEVEL_SENSOR_COUNT; i++) {
#ifdef BOARD_V2
		//portExtender.pinMode(levelPins[i], INPUT_PULLUP);           // input pin with pull-up resistor
#else
		pinMode(levelPins[i], INPUT);           // input pin with pull-up resistor
		digitalWrite(levelPins[i], HIGH);
#endif
	}
}

uint8_t WaterLevelMeter::readLevel() {
#ifdef BOARD_V2
	return 70;

	/*uint8_t mask = 0;
	for (int i = 0; i < LEVEL_SENSOR_COUNT; i++) {
		mask = mask | (portExtender.digitalRead(levelPins[i]) == LOW ? 1 : 0 ) << i;
	}
	switch (mask) {
	case 0b0001: case 0b011: case 0b0111: return 100;
	case 0b0110: return 70;
	case 0b0100: return 30;
	case 0b0000: return 0;
	}*/

#else
	if (digitalRead(levelPins[BOTTOM_SENSOR]) == HIGH && digitalRead(levelPins[TOP_SENSOR]) == HIGH)	// Bottom sensor is disconnected(no water in tank)
		return 0;
	else if (digitalRead(levelPins[BOTTOM_SENSOR]) == LOW && digitalRead(levelPins[TOP_SENSOR]) == HIGH)	// Bottom sensor is up. Top sensor is off (level is between)
		return 50;
	else if (digitalRead(levelPins[BOTTOM_SENSOR]) == LOW && digitalRead(levelPins[TOP_SENSOR]) == LOW)	// Both sensors are up - full tank.
		return 100;
	else return 100; // Prevent water in if levels have undefined values
#endif
}

