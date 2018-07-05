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

#ifdef BOARD_V2
#include "PCF8574.h"
#endif

const int MIN_WATER_LEVEL = 1;
const int MAX_WATER_LEVEL = 100;
const int LEVEL_SENSOR_COUNT = 2;
const int STORAGE_SIZE_WATER_LEVEL = 1 + (LEVEL_SENSOR_COUNT * sizeof(int));

class WaterLevelMeter {
protected:
#ifdef BOARD_V2
	const PCF8574& portExtender;
#endif
	const uint8_t* levelPins;
	uint8_t currentLevel;

public:

	WaterLevelMeter(const uint8_t* _levelPins
#ifdef BOARD_V2
		, const PCF8574& _portExtender
#endif
	);
	virtual ~WaterLevelMeter();

	uint8_t readLevel();
};

#endif /* WATERLEVELMETER_H_ */
