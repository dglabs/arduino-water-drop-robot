/*
 * WaterSchedule.cpp
 *
 *  Created on: 6 авг. 2017 г.
 *      Author: dennis
 */

#include <RTClib.h>
#include "WaterSchedule.h"
#include "EEPROMUtils.h"

WaterSchedule::WaterSchedule(const int _memAddr, RTC_DS3231& _rtc, RainSensor& _rainSensor):
	memAddr(_memAddr)
	, rtc(_rtc)
	, rainSensor(_rainSensor)
	, header(DateTime(2018, 04, 01, 0, 0, 0)	// April 1st start time
			, DateTime(2018, 11, 01, 0, 0, 0)	// November 1st stop time
			, 5 // 5C minimal operating temperature
			, EVENTS_SIZE)

	{
		// Default every day event
		DefaultEvents[0] = ScheduleEvent(1, EventType::WaterOut, DateTime(2018, 06, 10, 20, 20, 0), 800 /*duration*/, 60 /*liters*/, 5 /*minTemperature*/
				, 10 /*minLevel*/, 100 /*maxLevel*/, EventFlags::Active | EventFlags::SkipIfRecentRain, VALVE_OUT0 /*valves*/, 80 /*max humidity*/, 0 /*period flags*/, 1 /*repeat period days*/, 0 /*last action time*/);
		// Additional event when it's very hot
		DefaultEvents[1] = ScheduleEvent(2, EventType::WaterOut, DateTime(2018, 06, 10, 13, 0, 0), 300 /*duration*/, 30 /*liters*/, 40 /*minTemperature*/
				, 10 /*minLevel*/, 100 /*maxLevel*/, EventFlags::Active | EventFlags::SkipIfRecentRain, VALVE_OUT0, 60/*max humidity*/, 0 /*period flags*/, 1 /*repeat period days*/, 0 /*last action time*/);
		// Additional backup event if it was raining
		DefaultEvents[2] = ScheduleEvent(3, EventType::WaterOut, DateTime(2018, 06, 10, 22, 0, 0), 300 /*duration*/, 30 /*liters*/, 15 /*minTemperature*/
				, 10 /*minLevel*/, 100 /*maxLevel*/, /*EventFlags::Active |*/ EventFlags::OnlyAfterRecentRain, VALVE_OUT0, 100/*max humidity*/, 0 /*period flags*/, 1 /*repeat period days*/, 0 /*last action time*/);
		// Event to fill-in the tank
		DefaultEvents[3] = ScheduleEvent(4, EventType::WaterIn, DateTime(2018, 06, 10, 10, 30, 0), 600 /*duration*/, 300 /*liters*/, 5 /*minTemperature*/
				, 51 /*minLevel*/, 100 /*maxLevel*/, EventFlags::Active, VALVE_IN, 100/*max humidity*/, 0 /*period flags*/, 1 /*repeat period days*/, 0 /*last action time*/);
		DefaultEvents[4] = ScheduleEvent();

		currentEvent = DefaultEvents[0];

}

void WaterSchedule::setup() {
	ShcheduleHeader bufHeader;
	EEPROMUtils::read_bytes(memAddr, (uint8_t*)&bufHeader, sizeof(ShcheduleHeader));
	// If header is not valid, then initialize EEPROM with default schedule
	if (!header.isValid()) {
		EEPROMUtils::save_bytes(memAddr, (uint8_t*)&header, sizeof(ShcheduleHeader));
		EEPROMUtils::save_bytes(memAddr + sizeof(ShcheduleHeader), (uint8_t*)&DefaultEvents, sizeof(DefaultEvents));
	}
	/*else {
		header = bufHeader;
		EEPROMUtils::read_bytes(memAddr + sizeof(ShcheduleHeader), (uint8_t*)&DefaultEvents, sizeof(DefaultEvents));
		EEPROMUtils::read_bytes(memAddr + sizeof(ShcheduleHeader), (uint8_t*)&DefaultEvents, sizeof(DefaultEvents));
	}*/
}

WaterSchedule::~WaterSchedule() {
	// TODO Auto-generated destructor stub
}

void printDateTime(String title, DateTime& dt);

boolean WaterSchedule::isInActiveDateRange(int temperature) {
	if (temperature < header.minTemperature) return false;

	DateTime bufDateStart(header.startDate);
	DateTime bufDateEnd(header.stopDate);

	now = rtc.now();

	DateTime startDate(now.year(), bufDateStart.month(), bufDateStart.day(), bufDateStart.hour(), bufDateStart.minute(), bufDateStart.second());
	DateTime endDate(now.year(), bufDateEnd.month(), bufDateEnd.day(), bufDateEnd.hour(), bufDateEnd.minute(), bufDateEnd.second());

	boolean result = now.secondstime() >= startDate.secondstime() && now.secondstime() <= endDate.secondstime();;
	return result;
}


boolean WaterSchedule::isEventAppropriate(const ScheduleEvent& event, int temperature) {
	boolean result = false;
	if (event.type == EventType::None) return false;

	/*Serial.print("Event: "); Serial.println(event.type);
	Serial.print("Temp: "); Serial.println(temperature);*/

	now = rtc.now();
	DateTime eventTime(event.checkTime);
	DateTime actionTime(now.year(), now.month(), now.day(), eventTime.hour(), eventTime.minute(), eventTime.second());

	if (now.secondstime() >= actionTime.secondstime()) {
		long eventSpan = now.secondstime() - actionTime.secondstime();

		if (eventSpan < event.duration) {
			result = true;

			if (temperature < event.minTemperature)
				result = false;

			// Check recent rain flag
			LastRainInfo rainInfo;
			rainSensor.getLastRainInfo(rainInfo);

			if (rainInfo.startTime > 0 && rainInfo.startTime < 0xFFFFFFFF) {
				DateTime lastRaintTime(rainInfo.startTime);
				TimeSpan rainSpan = now - lastRaintTime;

				if (rainSpan.days() <= 1 && rainInfo.enoughRainPoured() && (event.flags & EventFlags::SkipIfRecentRain) != 0)
					result = false;

				if (!(rainSpan.days() <= 1 && rainInfo.enoughRainPoured()) && (event.flags & EventFlags::OnlyAfterRecentRain) != 0)
					result = false;
			}
		}
	}

	//Serial.println(result ? "TRUE" : "FALSE");
	return result;
}

boolean WaterSchedule::scanEvents(int temperature) {
	if (!isInActiveDateRange(temperature)) return false;

	if (isEventAppropriate(currentEvent, temperature)) return true;

	boolean result = false;
	for (int i = 0; i < header.numRecords && result == false; i++) {
		ScheduleEvent event = DefaultEvents[i];
		if (event.type == EventType::None) break;	// This is terminating event
		result = isEventAppropriate(event, temperature);
		if (result) { currentEvent = event; break; }
	}

	if (!result) currentEvent.type = EventType::None;

	return result;
}
