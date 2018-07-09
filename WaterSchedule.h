/*
 * WaterSchedule.h
 *
 *  Created on: 6 авг. 2017 г.
 *      Author: dennis
 */

#ifndef WATERSCHEDULE_H_
#define WATERSCHEDULE_H_

#include <Arduino.h>

enum EventType { None, WaterOut, WaterIn };
enum EventFlags {
	SkipIfRecentRain = 0x01
	, OnlyAfterRecentRain = 0x02
	, Active = 0x04
};

const int EVENTS_SIZE = 10;

// Valve flags values
const uint8_t VALVE_OUT0 = 0x01;
const uint8_t VALVE_OUT1 = 0x02;
const uint8_t VALVE_OUT2 = 0x04;
const uint8_t VALVE_ALL_OUT = 0x07;
const uint8_t VALVE_IN = 0x08;
const uint8_t VALVE_RAIN = 0x10;

const uint8_t NO_ID = 0xFF;

struct ScheduleEvent {
	uint8_t id;
	EventType type;	// Event type
	uint32_t checkTime;	// Time to check for this event
	unsigned int duration;	// Minimal duration
	unsigned int liters;	// Minimal liters to pour
	uint8_t minTemperature;	// Minimal temperature to engage this event
	uint8_t minLevel; 	// Minimal allowed water level
	uint8_t maxLevel; 	// Max allowed water level
	uint8_t flags;	// Bit flags
	uint8_t valves;	// Bit mask of valves to be open
	uint8_t maxSoilHumidity; // Max allowed soil humidity
	uint8_t periodFlags;		// Flags to setup type of period
	uint8_t repeatPeriodDays;	// repeat this event every repeatPeriodDays. 1 means daily,
	uint32_t lastActionTime;	// Time to check for this event

	ScheduleEvent() :  id(0), type(EventType::None) {}

	ScheduleEvent(const ScheduleEvent& event) :
		id(event.id)
		, type(event.type)
		, checkTime(event.checkTime)
		, duration(event.duration)
		, liters(event.liters)
		, minTemperature(event.minTemperature)
		, minLevel(event.minLevel)
		, maxLevel(event.maxLevel)
		, flags(event.flags)
		, valves(event.valves)
		, maxSoilHumidity(event.maxSoilHumidity)
		, periodFlags(event.periodFlags)
		, repeatPeriodDays(event.repeatPeriodDays)
		, lastActionTime(event.lastActionTime) {}


	ScheduleEvent(uint8_t _id, EventType _type, DateTime _checkTime, int _duration, int _liters
			, uint8_t _minTemperature, uint8_t _minLevel
			, uint8_t _maxLevel = 100, uint8_t _flags = 0
			, uint8_t _valves = VALVE_OUT0
			, uint8_t _maxSoilHumidity = 100
			, uint8_t _periodFlags = 9
			, uint8_t _repeatPeriodDays = 1
			, uint32_t _lastActionTime = 0) :
		id(_id)
		, type(_type)
		, checkTime(_checkTime.unixtime())
		, duration(_duration)
		, liters(_liters)
		, minTemperature(_minTemperature)
		, minLevel(_minLevel)
		, maxLevel(_maxLevel)
		, flags(_flags)
		, valves(_valves)
		, maxSoilHumidity(_maxSoilHumidity)
		, periodFlags(_periodFlags)
		, repeatPeriodDays(_repeatPeriodDays)
		, lastActionTime(_lastActionTime) {}
};

const uint16_t HEADER_SIGNATURE = 0x5A78U;

struct ShcheduleHeader {
	uint16_t signature;
	uint32_t startDate;	// Date to start operation
	uint32_t stopDate;	// Date to stop operation
	uint8_t minTemperature; // minimal operation temperature
	uint8_t numRecords;	// Number of schedule records

	boolean isValid() const {
		return  signature == HEADER_SIGNATURE && startDate > 0 && startDate < 0xFFFFFFFF &&
				stopDate > 0 && stopDate < 0xFFFFFFFF && (minTemperature > 5 && minTemperature < 20) &&
				(numRecords > 0 && numRecords <= 10);
	}

	ShcheduleHeader() :
		signature(HEADER_SIGNATURE) {}

	ShcheduleHeader(const ShcheduleHeader& h) :
		signature(HEADER_SIGNATURE)
		, startDate(h.startDate)
		, stopDate(h.stopDate)
		, minTemperature(h.minTemperature)
		, numRecords(h.numRecords) {}

	ShcheduleHeader(DateTime _startDate, DateTime _stopDate, uint8_t _minTemperature, uint8_t _numRecords) :
		signature(HEADER_SIGNATURE)
		, startDate(_startDate.unixtime())
		, stopDate(_stopDate.unixtime())
		, minTemperature(_minTemperature)
		, numRecords(_numRecords) {}
};

const int STORAGE_SIZE_SCHEDULE = sizeof(ShcheduleHeader) + sizeof(ScheduleEvent) * EVENTS_SIZE;

class WaterSchedule {
private:
	const int memAddr;

	DateTime now;
	ShcheduleHeader header;
	ScheduleEvent currentEvent;

	ScheduleEvent DefaultEvents[EVENTS_SIZE];
	Chrono lastScanTime;
	Chrono lastCheckTime;

public:
	WaterSchedule(const int _memAddr);

	void setup();
	boolean scanEvents();

	ShcheduleHeader getHeader() const { return header; }
	ScheduleEvent& getCurrentEvent() { return currentEvent; }
	void setCurrentEvent(const ScheduleEvent& event);

	boolean isEventAppropriate(const ScheduleEvent& event);
	void dismissCurrentEvent();
	boolean isInActiveDateRange();

	boolean checkEventSecondaryConditions(const ScheduleEvent& event);
	uint32_t getTodayMaxPouring(uint8_t valveFlags = 0xFF);

	int secondsFromLastScan() const { return lastScanTime.elapsed(); }
	int secondsFromLastCheck() const { return lastCheckTime.elapsed(); }
};

// Scheduler
extern WaterSchedule schedule;

#endif /* WATERSCHEDULE_H_ */
