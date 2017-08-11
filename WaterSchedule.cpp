/*
 * WaterSchedule.cpp
 *
 *  Created on: 6 авг. 2017 г.
 *      Author: dennis
 */

#include <RTClib.h>
#include "WaterSchedule.h"
#include "EEPROMUtils.h"

const ScheduleEvent DefaultEvents[] = {
		// Default every day event
		ScheduleEvent(EventType::WaterOut, DateTime(2017, 06, 10, 21, 0, 0), 600 /*duration*/, 60 /*liters*/, 10 /*minTemperature*/
				, 0 /*minLevel*/, 100 /*maxLevel*/, EventFlags::SkipIfRecentRain)
		// Additional event when it's very hot
		, ScheduleEvent(EventType::WaterOut, DateTime(2017, 06, 10, 16, 0, 0), 300 /*duration*/, 30 /*liters*/, 30 /*minTemperature*/
				, 0 /*minLevel*/, 100 /*maxLevel*/, EventFlags::SkipIfRecentRain)
		// Additional backup event if it was raining
		, ScheduleEvent(EventType::WaterOut, DateTime(2017, 06, 10, 22, 0, 0), 300 /*duration*/, 30 /*liters*/, 15 /*minTemperature*/
				, 0 /*minLevel*/, 100 /*maxLevel*/, 0)
		// Event to fill-in the tank
		, ScheduleEvent(EventType::WaterIn, DateTime(2017, 06, 10, 13, 0, 0), 250 /*duration*/, 300 /*liters*/, 10 /*minTemperature*/
				, 30 /*minLevel*/, 70 /*maxLevel*/, 0)
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

boolean WaterSchedule::isInActiveDateRange() {
	DateTime bufDateStart(header.startDate);
	DateTime bufDateEnd(header.stopDate);

	DateTime startDate(now.year(), bufDateStart.month(), bufDateStart.day(), bufDateStart.hour(), bufDateStart.minute(), bufDateStart.second());
	DateTime endDate(now.year(), bufDateEnd.month(), bufDateEnd.day(), bufDateEnd.hour(), bufDateEnd.minute(), bufDateEnd.second());

	return now.unixtime() >= startDate.unixtime() && now.unixtime() <= endDate.unixtime();
}


boolean WaterSchedule::isEventAppropriate(ScheduleEvent& event) {
	boolean result = false;
	if (event.type == EventType::None) return false;

	DateTime eventTime(event.checkTime);
	DateTime actionTime(now.year(), now.month(), now.day(), eventTime.hour(), eventTime.minute(), eventTime.second());

	if (now.unixtime() >= actionTime.unixtime()) {
		TimeSpan eventSpan = now - actionTime;

		if (eventSpan.totalseconds() < event.duration) {
			result = true;

			// TODO: Check min temperature and set result = false if needed

			// Check recent rain flag
			if (event.flags & EventFlags::SkipIfRecentRain) {
				LastRainInfo rainInfo;
				rainSensor.getLastRainInfo(rainInfo);

				if (rainInfo.startTime > 0 && rainInfo.startTime < 0xFFFFFFFF) {
					DateTime lastRaintTime(rainInfo.startTime);
					TimeSpan rainSpan = now - lastRaintTime;

					if (rainSpan.days() <= 1 && rainInfo.enoughRainPoured())
						result = false;
				}
			}
		}
	}

	return result;
}

boolean WaterSchedule::scanEvents() {
	if (isEventAppropriate(currentEvent)) return true;

	if (!isInActiveDateRange()) return false;

	now = rtc.now();
	boolean result = false;
	for (int i = 0; i < header.numRecords && result == false; i++) {
		EEPROMUtils::read_bytes(memAddr + sizeof(ShcheduleHeader) + (i * sizeof(currentEvent)), (uint8_t*)&currentEvent, sizeof(currentEvent));
		if (currentEvent.type == EventType::None) break;	// This is terminating event
		result = isEventAppropriate(currentEvent);
	}

	if (!result) currentEvent.type = EventType::None;

	return result;
}
