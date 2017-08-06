/*
 * WaterSchedule.cpp
 *
 *  Created on: 6 авг. 2017 г.
 *      Author: dennis
 */

#include "WaterSchedule.h"
#include "EEPROMUtils.h"

const ScheduleEvent DefaultEvents[] = {
		// Default every day event
		ScheduleEvent(EventType::WaterOut, DateTime(2017, 06, 10, 21, 0, 0), 300 /*duration*/, 100 /*liters*/, 10 /*minTemperature*/
				, 0 /*minLevel*/, 100 /*maxLevel*/, EventFlags::SkipIfRecentRain)
		// Additional event when it's very hot
		, ScheduleEvent(EventType::WaterOut, DateTime(2017, 06, 10, 16, 0, 0), 200 /*duration*/, 50 /*liters*/, 30 /*minTemperature*/
				, 0 /*minLevel*/, 100 /*maxLevel*/, EventFlags::SkipIfRecentRain)
		// Additional backup event if it was raining
		, ScheduleEvent(EventType::WaterOut, DateTime(2017, 06, 10, 22, 0, 0), 200 /*duration*/, 50 /*liters*/, 15 /*minTemperature*/
				, 0 /*minLevel*/, 100 /*maxLevel*/, 0)
		// Event to fill-in the tank
		, ScheduleEvent(EventType::WaterIn, DateTime(2017, 06, 10, 13, 0, 0), 500 /*duration*/, 300 /*liters*/, 10 /*minTemperature*/
				, 30 /*minLevel*/, 80 /*maxLevel*/, 0)
};

const ShcheduleHeader DefaultHeader(
		DateTime(2018, 04, 01, 0, 0, 0).unixtime()	// April 1st start time
		, DateTime(2018, 11, 01, 0, 0, 0).unixtime()	// November 1st stop time
		, 10 // 10C minimal operating temperature
		, sizeof(DefaultEvents) / sizeof(ScheduleEvent));

WaterSchedule::WaterSchedule(const int _memAddr, RTC_DS1307& _rtc, RainSensor& _rainSensor):
	memAddr(_memAddr)
	, rtc(_rtc)
	, rainSensor(_rainSensor)

	, header()
	, currentEvent() {

	EEPROMUtils::read_bytes(memAddr, (uint8_t*)&header, sizeof(ShcheduleHeader));
	// If header is not valid, then initialize EEPROM with default schedule
	if (!header.isValid()) {
		EEPROMUtils::save_bytes(memAddr, (uint8_t*)&DefaultHeader, sizeof(ShcheduleHeader));
		EEPROMUtils::save_bytes(memAddr + sizeof(ShcheduleHeader), (uint8_t*)&DefaultEvents, sizeof(DefaultEvents));

		EEPROMUtils::read_bytes(memAddr, (uint8_t*)&header, sizeof(ShcheduleHeader));
	}
}

WaterSchedule::~WaterSchedule() {
	// TODO Auto-generated destructor stub
}

