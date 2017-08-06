/*
 * WaterSchedule.h
 *
 *  Created on: 6 авг. 2017 г.
 *      Author: dennis
 */

#ifndef WATERSCHEDULE_H_
#define WATERSCHEDULE_H_

#include <Arduino.h>
#include <RTClib.h>
#include "RainSensor.h"

enum EventType { None, WaterOut, WaterIn };
enum EventFlags {
	SkipIfRecentRain = 1
};

struct ScheduleEvent {
	EventType type;	// Event type
	uint32_t checkTime;	// Time to check for this event
	int duration;	// Minimal duration
	int liters;		// Minimal liters to pour
	uint8_t minTemperature;	// Minimal temperature to engage this event
	uint8_t minLevel; 	// Minimal allowed water level
	uint8_t maxLevel; 	// Max allowed water level
	uint8_t flags;	// Bit flags

	ScheduleEvent() {}

	ScheduleEvent(EventType _type, DateTime _checkTime, int _duration, int _liters
			, uint8_t _minTemperature, uint8_t _minLevel
			, uint8_t _maxLevel, uint8_t _flags) :
		type(type)
		, checkTime(_checkTime.unixtime())
		, duration(_duration)
		, liters(_liters)
		, minLevel(_minLevel)
		, maxLevel(_maxLevel)
		, minTemperature(_minTemperature)
		, flags(_flags) {}
};

struct ShcheduleHeader {
	uint32_t startDate;	// Date to start operation
	uint32_t stopDate;	// Date to stop operation
	uint8_t minTemperature; // minimal operation temperature
	uint8_t numRecords;	// Number of schedule records

	boolean isValid() const {
		return startDate > 0 && startDate < 0xFFFFFFFF &&
				stopDate > 0 && stopDate < 0xFFFFFFFF;
	}

	ShcheduleHeader() {}

	ShcheduleHeader(DateTime _startDate, DateTime _stopDate, uint8_t _minTemperature, uint8_t _numRecords) :
		startDate(_startDate.unixtime())
		, stopDate(_stopDate.unixtime())
		, minTemperature(_minTemperature)
		, numRecords(numRecords) {}
};

class WaterSchedule {
private:
	const int memAddr;

	RTC_DS1307& rtc;
	RainSensor& rainSensor;

	ShcheduleHeader header;
	ScheduleEvent currentEvent;
public:
	WaterSchedule(const int _memAddr, RTC_DS1307& _rtc, RainSensor& _rainSensor);
	virtual ~WaterSchedule();
};

#endif /* WATERSCHEDULE_H_ */
