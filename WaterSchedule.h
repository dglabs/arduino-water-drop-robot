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
	, OnlyAfterRecentRain = 2
};

const int EVENTS_SIZE = 10;

struct ScheduleEvent {
	EventType type;	// Event type
	uint32_t checkTime;	// Time to check for this event
	int duration;	// Minimal duration
	int liters;		// Minimal liters to pour
	uint8_t minTemperature;	// Minimal temperature to engage this event
	uint8_t minLevel; 	// Minimal allowed water level
	uint8_t maxLevel; 	// Max allowed water level
	uint8_t flags;	// Bit flags
	uint8_t maxSoilHumidity; // Max allowed soil humidity

	ScheduleEvent() :type(EventType::None) {}

	ScheduleEvent(const ScheduleEvent& event) {
		type = event.type;
		checkTime = event.checkTime;
		duration = event.duration;
		liters = event.liters;
		minLevel = event.minLevel;
		maxLevel = event.maxLevel;
		minTemperature = event.minTemperature;
		flags = event.flags;
		maxSoilHumidity = event.maxSoilHumidity;
	}

	ScheduleEvent(EventType _type, DateTime _checkTime, int _duration, int _liters
			, uint8_t _minTemperature, uint8_t _minLevel
			, uint8_t _maxLevel = 100, uint8_t _flags = 0, uint8_t _maxSoilHumidity = 100) :
		type(_type)
		, checkTime(_checkTime.unixtime())
		, duration(_duration)
		, liters(_liters)
		, minTemperature(_minTemperature)
		, minLevel(_minLevel)
		, maxLevel(_maxLevel)
		, flags(_flags)
		, maxSoilHumidity(_maxSoilHumidity) {}
};

struct ShcheduleHeader {
	uint32_t startDate;	// Date to start operation
	uint32_t stopDate;	// Date to stop operation
	uint8_t minTemperature; // minimal operation temperature
	uint8_t numRecords;	// Number of schedule records

	boolean isValid() const {
		return startDate > 0 && startDate < 0xFFFFFFFF &&
				stopDate > 0 && stopDate < 0xFFFFFFFF && (minTemperature > 5 && minTemperature < 20) &&
				(numRecords > 0 && numRecords <= 10);
	}

	ShcheduleHeader() {}

	ShcheduleHeader(const ShcheduleHeader& h) {
		startDate = h.startDate;
		stopDate = h.stopDate;
		minTemperature = h.minTemperature;
		numRecords = h.numRecords;
	}

	ShcheduleHeader(DateTime _startDate, DateTime _stopDate, uint8_t _minTemperature, uint8_t _numRecords) :
		startDate(_startDate.unixtime())
		, stopDate(_stopDate.unixtime())
		, minTemperature(_minTemperature)
		, numRecords(_numRecords) {}
};

const int STORAGE_SIZE_SCHEDULE = sizeof(ShcheduleHeader) + sizeof(ScheduleEvent) * EVENTS_SIZE;

class WaterSchedule {
private:
	const int memAddr;

	RTC_DS3231& rtc;
	RainSensor& rainSensor;

	DateTime now;
	ShcheduleHeader header;
	ScheduleEvent currentEvent;

	ScheduleEvent DefaultEvents[EVENTS_SIZE];

public:
	WaterSchedule(const int _memAddr, RTC_DS3231& _rtc, RainSensor& _rainSensor);
	virtual ~WaterSchedule();

	void setup();
	boolean scanEvents(int temperature);

	ShcheduleHeader getHeader() const { return header; }
	ScheduleEvent& getCurrentEvent() { return currentEvent; }
	void setCurrentEvent(const ScheduleEvent& event) { currentEvent = event; }

	boolean isEventAppropriate(const ScheduleEvent& event, int temperature);
	void dismissCurrentEvent() { currentEvent.type = EventType::None; };
	boolean isInActiveDateRange(int temperature);
};

#endif /* WATERSCHEDULE_H_ */
