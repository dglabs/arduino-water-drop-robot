/*
 * WaterLevelMeter.h
 *
 *  Created on: 7 июн. 2017 г.
 *      Author: dennis
 */

#ifndef WATERLEVELMETER_H_
#define WATERLEVELMETER_H_

#include <Arduino.h>

const int MIN_WATER_LEVEL = 1;
const int MAX_WATER_LEVEL = 100;
const int LEVEL_SENSOR_COUNT = 2;
const int STORAGE_SIZE_WATER_LEVEL = 1 + (LEVEL_SENSOR_COUNT * sizeof(int));
//const int HALL_DIFF_THRESHOLD = 12;

class WaterLevelMeter {
protected:
	//int scanCount;
	const uint8_t* levelPins;
	//const int* initialAverageValues;
	//const int memAddress;
	uint8_t currentLevel;

	//int currentValues[LEVEL_SENSOR_COUNT];
	//int averageValues[LEVEL_SENSOR_COUNT];
	//int sumValues[LEVEL_SENSOR_COUNT];

	//void save_averages(int addr);
	//void read_averages(int addr);
	//void sortAbsDiffs(uint8_t* sortedIndexes, int* sortedDiffs);
	//uint8_t sensorDiffToLevel(int diff, uint8_t index);

public:

	WaterLevelMeter(const uint8_t* _levelPins/*, const int* _initialAverageValues, const int _memAddress*/);
	virtual ~WaterLevelMeter();

	uint8_t readLevel();

	//const int* getCurrentValues() const { return currentValues; }
	//const int* getAverageValues() const { return averageValues; }

	//void forceCurrentValuesAsAverages();
};

#endif /* WATERLEVELMETER_H_ */
