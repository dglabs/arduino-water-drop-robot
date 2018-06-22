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

const int MIN_LEVEL_DIFF = 15;

const int BOTTOM_SENSOR = 1;
const int TOP_SENSOR = 0;

WaterLevelMeter::WaterLevelMeter(const uint8_t* _levelPins/*, const int* _initialAverageValues, const int _memAddress*/) :
	levelPins(_levelPins)
	/*, memAddress(_memAddress)
	, initialAverageValues(_initialAverageValues)*/
{
	// Pull-up all analog pins to detect disconnected wires
	for (int i = 0; i < LEVEL_SENSOR_COUNT; i++) {
		//pinMode(levelPins[i], INPUT_PULLUP);
		pinMode(levelPins[i], INPUT);           // input pin with pull-up resistor
		digitalWrite(levelPins[i], HIGH);
	}

	// Initialize last saved current value
	/*currentLevel = EEPROM.read(memAddress);
	if (currentLevel < MIN_WATER_LEVEL || currentLevel > MAX_WATER_LEVEL) {
		currentLevel = MAX_WATER_LEVEL;*/
		//rtc.writenvram(sramAddress, currentLevel);
	//}

	// Read average values from EEPROM
	/*read_averages(memAddress + sizeof(currentLevel));
	boolean invalidAverages = false;
	for (int i = 0; i < LEVEL_SENSOR_COUNT && !invalidAverages; i++)
		if (abs(averageValues[i] - initialAverageValues[i]) > 30) invalidAverages = true;

	// Check is averages in EEPROM are valid and initialize it if not
	if (invalidAverages) {
		for (int i = 0; i < LEVEL_SENSOR_COUNT; i++)
			averageValues[i] = initialAverageValues[i];
		save_averages(memAddress + sizeof(currentLevel));
	}*/

	// Initialize averages calculation in this new cycle
	/*scanCount = 1;
	for (int i = 0; i < LEVEL_SENSOR_COUNT; i++)
		sumValues[i] = averageValues[i];*/

}

WaterLevelMeter::~WaterLevelMeter() {
	// TODO Auto-generated destructor stub
}


/*void WaterLevelMeter::sortAbsDiffs(uint8_t* sortedIndexes, int* sortedDiffs) {
	int buf[LEVEL_SENSOR_COUNT];
	for (uint8_t i = 0; i < LEVEL_SENSOR_COUNT; i++) buf[i] = sortedDiffs[i];
	
	for (uint8_t round = 0; round < LEVEL_SENSOR_COUNT; round++) {
		int idx = 0;
		int max = 0;
		for (uint8_t i = 0; i < LEVEL_SENSOR_COUNT; i++) {
			if (abs(buf[i]) > abs(max)) {
				idx = i;
				max = buf[i]; 
			}
		}
		buf[round] = 0;
		sortedDiffs[round] = max;
		sortedIndexes[round] = idx;		
	}	
}

uint8_t WaterLevelMeter::sensorDiffToLevel(int diff, uint8_t index) {
	int level = 0;
	if (diff > 0)
		level = (uint8_t)(((float)((index + 1) * 2 - 1)/(float)((LEVEL_SENSOR_COUNT + 1) * 2))*100.0);
	else
		level = (uint8_t)(((float)((index + 1) * 2)/(float)((LEVEL_SENSOR_COUNT + 1) * 2))*100.0);
	return level;
}*/

uint8_t WaterLevelMeter::readLevel() {
	if (digitalRead(levelPins[BOTTOM_SENSOR]) == HIGH && digitalRead(levelPins[TOP_SENSOR]) == HIGH)	// Bottom sensor is disconnected(no water in tank)
		return 0;
	else if (digitalRead(levelPins[BOTTOM_SENSOR]) == LOW && digitalRead(levelPins[TOP_SENSOR]) == HIGH)	// Bottom sensor is up. Top sensor is off (level is between)
		return 50;
	else if (digitalRead(levelPins[BOTTOM_SENSOR]) == LOW && digitalRead(levelPins[TOP_SENSOR]) == LOW)	// Both sensors are up - full tank.
		return 100;
	else return 100; // Prevent water in if levels have undefined values
}
/*	uint8_t level = 0;
	uint8_t sortedIndexes[LEVEL_SENSOR_COUNT];
	int diffs[LEVEL_SENSOR_COUNT];
	
	for (uint8_t i = 0; i < LEVEL_SENSOR_COUNT; i++) {
		currentValues[i] = analogRead(levelPins[i]);
		// Ignore disconnected lines
		if (currentValues[i] == 0 || (currentValues[i] > 900 || currentValues[i] < 100)) currentValues[i] = averageValues[i];

		diffs[i] = currentValues[i] - averageValues[i];
	}
	
	// Update sum values for averages
	for (uint8_t i = 0; i < LEVEL_SENSOR_COUNT; i++) {
		if (abs(diffs[i]) < HALL_DIFF_THRESHOLD)
			sumValues[i] += currentValues[i];
		else
			sumValues[i] += averageValues[i];
	}

	sortAbsDiffs(sortedIndexes, diffs);
	
	for (uint8_t i = 0; i < LEVEL_SENSOR_COUNT; i++) {
		if (abs(diffs[i]) >= HALL_DIFF_THRESHOLD)
			level = sensorDiffToLevel(diffs[i], sortedIndexes[i]);
		else break;
		
		// If level was detected (any sensor is active), then update currentLevel and save it
		// If max sensor diff produces fake level, then ignore it and take next one
		if (level > 0 && level <= 100 && level != currentLevel && abs(level - currentLevel) <= MIN_LEVEL_DIFF) {
			currentLevel = level;
			EEPROM.write(memAddress, currentLevel);
			break;
		}
	}
	
	// If all levels seem to be fake, then assume first level is right
	if (level == 0 && abs(diffs[0]) >= HALL_DIFF_THRESHOLD) {
		level = sensorDiffToLevel(diffs[0], sortedIndexes[0]);
		if (level > 0 && level <= 100 && level != currentLevel) {
			currentLevel = level;
			EEPROM.write(memAddress, currentLevel);
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
}*/

/*void WaterLevelMeter::forceCurrentValuesAsAverages() {
	for (int i = 0; i < LEVEL_SENSOR_COUNT; i++)
		averageValues[i] = currentValues[i];
	save_averages(memAddress + sizeof(currentLevel));
}

// Save averages to memory
void WaterLevelMeter::save_averages(int addr) {
	EEPROMUtils::save_bytes(addr, (uint8_t *)&averageValues, sizeof(averageValues));
}

void WaterLevelMeter::read_averages(int addr) {
	EEPROMUtils::read_bytes(addr, (uint8_t *)&averageValues, sizeof(averageValues));
}*/


