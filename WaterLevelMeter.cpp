/*
 * WaterLevelMeter.cpp
 *
 *  Created on: 7 июн. 2017 г.
 *      Author: dennis
 */

#include "WaterLevelMeter.h"
#include <EEPROM.h>

//#define _SERIAL_DEBUG

WaterLevelMeter::WaterLevelMeter(const uint8_t* _levelPins, const int* _initialAverageValues, const int _memAddress) :
	levelPins(_levelPins)
	, memAddress(_memAddress)
	, initialAverageValues(_initialAverageValues)
{
	// Pull-up all analog pins to detect disconnected wires
	for (int i = 0; i < LEVEL_SENSOR_COUNT; i++)
		pinMode(levelPins[i], INPUT_PULLUP);

	// Initialize last saved current value
	currentLevel = EEPROM.read(memAddress);
	if (currentLevel < MIN_WATER_LEVEL || currentLevel > MAX_WATER_LEVEL) {
		currentLevel = MAX_WATER_LEVEL;
		//rtc.writenvram(sramAddress, currentLevel);
	}

	// Read average values from EEPROM
	read_averages(memAddress + sizeof(currentLevel));
	boolean invalidAverages = false;
	for (int i = 0; i < LEVEL_SENSOR_COUNT && !invalidAverages; i++)
		if (abs(averageValues[i] - initialAverageValues[i]) > 30) invalidAverages = true;

	// Check is averages in EEPROM are valid and initialize it if not
	if (invalidAverages) {
		for (int i = 0; i < LEVEL_SENSOR_COUNT; i++)
			averageValues[i] = initialAverageValues[i];
		save_averages(memAddress +  + sizeof(currentLevel));
	}

	// Initialize averages calculation in this new cycle
	scanCount = 1;
	for (int i = 0; i < LEVEL_SENSOR_COUNT; i++)
		sumValues[i] = averageValues[i];

}

WaterLevelMeter::~WaterLevelMeter() {
	// TODO Auto-generated destructor stub
}

uint8_t WaterLevelMeter::readLevel() {
	uint8_t level = 0;
	int diffs[LEVEL_SENSOR_COUNT];
	for (uint16_t i = 0; i < LEVEL_SENSOR_COUNT; i++) {
		currentValues[i] = analogRead(levelPins[i]);
		// Ignore disconnected lines
		if (currentValues[i] == 0 || (currentValues[i] > 900 || currentValues[i] < 100)) currentValues[i] = averageValues[i];

		diffs[i] = currentValues[i] - averageValues[i];
	}

	// Find the value with max difference
	int maxDiffIndex = -1;
	int maxDiff = 0;
	for (uint16_t i = 0; i < LEVEL_SENSOR_COUNT; i++)
		if (abs(diffs[i]) > abs(maxDiff)) {
			maxDiff = diffs[i];
			maxDiffIndex = i;
		}

	// Update sum values for averages
	for (int i = 0; i < LEVEL_SENSOR_COUNT; i++) {
		if (i != maxDiffIndex)
			sumValues[i] += currentValues[i];
		else
			sumValues[i] += averageValues[i];
	}

	if (maxDiffIndex >= 0 && abs(maxDiff) >= 12) {
		if (maxDiff > 0) {
			level = (uint8_t)(((float)((maxDiffIndex + 1) * 2 - 1)/(float)((LEVEL_SENSOR_COUNT + 1) * 2))*100.0);
#ifdef _SERIAL_DEBUG
			Serial.print("CUR > AVG, Level=");
			Serial.println(level);
#endif
		}
		else {
			level = (uint8_t)(((float)((maxDiffIndex + 1) * 2)/(float)((LEVEL_SENSOR_COUNT + 1) * 2))*100.0);
#ifdef _SERIAL_DEBUG
			Serial.print("CUR < AVG, Level=");
			Serial.println(level);
#endif
		}
	}

#ifdef _SERIAL_DEBUG
	Serial.print("CUR:");
	for (uint16_t i = 0; i < LEVEL_SENSOR_COUNT; i++) {
		Serial.print(currentValues[i]);
		Serial.print(';');
	}
	Serial.println();
	Serial.print("AVG:");
	for (uint16_t i = 0; i < LEVEL_SENSOR_COUNT; i++) {
		Serial.print(averageValues[i]);
		Serial.print(';');
	}
	Serial.println();
#endif

	scanCount++;

	// If level was detected (any sensor is active), then update currentLevel and save it
	if (level > 0 && level <= 100 && level != currentLevel) {
		currentLevel = level;
		EEPROM.write(memAddress, currentLevel);
	}

	// If sum average values are going to overflow, then calculate averages and save to EEPROM
	if (sumValues[0] > 10000) {
		for (int i = 0; i < LEVEL_SENSOR_COUNT; i++) {
			int avg = sumValues[i] / scanCount;
			if (abs(avg- averageValues[i]) < 30) averageValues[i] = avg;
			sumValues[i] = averageValues[i];
		}
		save_averages(memAddress + sizeof(currentLevel));
		scanCount = 1;
	}

	return currentLevel;
}

void WaterLevelMeter::forceCurrentValuesAsAverages() {
	for (int i = 0; i < LEVEL_SENSOR_COUNT; i++)
		averageValues[i] = currentValues[i];
	save_averages(memAddress + sizeof(currentLevel));
}

// Save averages to memory
void WaterLevelMeter::save_averages(int addr) {
	uint8_t *raw = (uint8_t *)&averageValues;
	for (uint16_t i = 0; i < sizeof(averageValues); i++)
		EEPROM.write(addr + i, raw[i]);
}

void WaterLevelMeter::read_averages(int addr) {
	uint8_t *raw = (uint8_t *)&averageValues;
	for (uint16_t i = 0; i < sizeof(averageValues); i++)
		raw[i] = EEPROM.read(addr + i);
}


