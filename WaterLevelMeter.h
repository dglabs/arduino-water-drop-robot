/*
 * WaterLevelMeter.h
 *
 *  Created on: 7 июн. 2017 г.
 *      Author: dennis
 */

#ifndef WATERLEVELMETER_H_
#define WATERLEVELMETER_H_

#include <Arduino.h>
#include "CommonDefs.h"

const int MIN_WATER_LEVEL = 1;
const int MAX_WATER_LEVEL = 100;
#ifdef BOARD_V2
const int LEVEL_SENSOR_COUNT = 3;
#else
const int LEVEL_SENSOR_COUNT = 2;
#endif
const int STORAGE_SIZE_WATER_LEVEL = 1 + (LEVEL_SENSOR_COUNT * sizeof(int));

class WaterLevelMeter {
protected:
	const uint8_t* levelPins;
	uint8_t currentLevel;

public:

	WaterLevelMeter(const uint8_t* _levelPins);
	void setup();

	uint8_t readLevel();
};

// Water level meter
extern WaterLevelMeter waterLevelMeter;

#endif /* WATERLEVELMETER_H_ */
