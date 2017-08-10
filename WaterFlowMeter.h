/*
 * WaterFlowMeter.h
 *
 *  Created on: 27 июл. 2017 г.
 *      Author: dennis
 */

#ifndef WATERFLOWMETER_H_
#define WATERFLOWMETER_H_
#include <Chrono.h>

const int STORAGE_SIZE_WATER_VOLUME = sizeof(unsigned long);

class WaterFlowMeter {
private:
	const int memAddress;
	unsigned long startedVolume;
	unsigned long totalVolume;
	Chrono startedChrono;

	void saveLong(int addr, unsigned long value);
	unsigned long readLong(int addr);

public:
	WaterFlowMeter(const int _memAddress);
	virtual ~WaterFlowMeter();

	void startWaterOut();
	unsigned long stopWaterOut();
	unsigned long getVolumeFromStart() const;
	unsigned long getTotalVolume();

	unsigned long getVolumePerMinute() const;
};

#endif /* WATERFLOWMETER_H_ */
