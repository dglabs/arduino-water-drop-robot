/*
 * WaterFlowMeter.h
 *
 *  Created on: 27 июл. 2017 г.
 *      Author: dennis
 */

#ifndef WATERFLOWMETER_H_
#define WATERFLOWMETER_H_
#include <Chrono.h>
#include <RTClib.h>

struct PourSummary {
	uint32_t startTime;
	uint32_t litres;

	PourSummary(): startTime(0), litres(0) {};

	void init(uint32_t _startTime) { startTime = _startTime; litres = 0; }

};

const uint32_t FLOW_SIGNATURE = 0xAB57ED55;

struct PourStatistics {
	uint32_t signature;
	PourSummary total;
	PourSummary lastYear;
	PourSummary lastMonth;
	PourSummary today;

	PourStatistics() { init(0); };

	void init(uint32_t _startTime) { signature = FLOW_SIGNATURE; total.init(_startTime), lastYear.init(_startTime); lastMonth.init(_startTime); today.init(_startTime); }
};

const int STORAGE_SIZE_WATER_VOLUME = sizeof(unsigned long) /*Total volume*/ + sizeof(PourStatistics);

class WaterFlowMeter {
private:
	const int memAddress;
	unsigned long totalVolume;
	Chrono startedChrono;

	RTC_DS3231& rtc;
	PourStatistics statistics;

public:
	WaterFlowMeter(const int _memAddress, RTC_DS3231& _rtc);
	virtual ~WaterFlowMeter();

	void initStatistics();
	// Returns true and saves statistics to EEPROM if adjustment required. Else returns false
	boolean adjustStatictcs();

	void startWaterOut();
	unsigned long stopWaterOut();
	unsigned long getVolumeFromStart() const;
	unsigned long getTotalVolume();
	const PourStatistics& getStatistics() const { return statistics; };

	unsigned long getVolumePerMinute() const;
};

#endif /* WATERFLOWMETER_H_ */
