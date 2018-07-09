/*
 * WaterSchedule.cpp
 *
 *  Created on: 6 авг. 2017 г.
 *      Author: dennis
 */

#include "WaterDropRobot.h"
#include <RTClib.h>
#include "WaterSchedule.h"
#include "EEPROMUtils.h"
#include "RobotDisplay.h"

WaterSchedule::WaterSchedule(const int _memAddr):
	memAddr(_memAddr)
	, lastScanTime(Chrono::SECONDS)
	, lastCheckTime(Chrono::SECONDS)
	, header(DateTime(2018, 04, 01, 0, 0, 0)	// April 1st start time
			, DateTime(2018, 11, 01, 0, 0, 0)	// November 1st stop time
			, 5 // 5C minimal operating temperature
			, EVENTS_SIZE)
	, currentEvent()

	{
		lastScanTime.restart();
		lastCheckTime.restart();

		// Default every day event
		DefaultEvents[0] = ScheduleEvent(1, EventType::WaterOut, DateTime(2018, 06, 10, 20, 30, 0), 1200 /*duration*/, 60 /*liters*/, 5 /*minTemperature*/
				, 10 /*minLevel*/, 100 /*maxLevel*/, EventFlags::Active | EventFlags::SkipIfRecentRain, VALVE_OUT0 /*valves*/, 80 /*max humidity*/
				, 0 /*period flags*/, 1 /*repeat period days*/, 0 /*last action time*/);
		// Additional event when it's very hot
		DefaultEvents[1] = ScheduleEvent(2, EventType::WaterOut, DateTime(2018, 06, 10, 13, 0, 0), 300 /*duration*/, 30 /*liters*/, 35 /*minTemperature*/
				, 10 /*minLevel*/, 100 /*maxLevel*/, EventFlags::Active | EventFlags::SkipIfRecentRain, VALVE_OUT0, 60/*max humidity*/
				, 0 /*period flags*/, 1 /*repeat period days*/, 0 /*last action time*/);
		// Additional backup event if it was raining
		DefaultEvents[2] = ScheduleEvent(3, EventType::WaterOut, DateTime(2018, 06, 10, 22, 0, 0), 300 /*duration*/, 30 /*liters*/, 15 /*minTemperature*/
				, 10 /*minLevel*/, 100 /*maxLevel*/, EventFlags::Active | EventFlags::OnlyAfterRecentRain, VALVE_OUT0, 100/*max humidity*/
				, 0 /*period flags*/, 1 /*repeat period days*/, 0 /*last action time*/);
		// Event to fill-in the tank
		DefaultEvents[3] = ScheduleEvent(4, EventType::WaterIn, DateTime(2018, 06, 10, 10, 30, 0), 600 /*duration*/, 300 /*liters*/, 5 /*minTemperature*/
				, 51 /*minLevel*/, 100 /*maxLevel*/, EventFlags::Active, VALVE_IN, 100/*max humidity*/, 0 /*period flags*/
				, 1 /*repeat period days*/, 0 /*last action time*/);
		DefaultEvents[4] = ScheduleEvent();
}

void WaterSchedule::setup() {
	ShcheduleHeader bufHeader;
	EEPROMUtils::read_bytes(memAddr, (uint8_t*)&bufHeader, sizeof(ShcheduleHeader));
	// If header is not valid, then initialize EEPROM with default schedule
	if (!header.isValid()) {
		EEPROMUtils::save_bytes(memAddr, (uint8_t*)&header, sizeof(ShcheduleHeader));
		EEPROMUtils::save_bytes(memAddr + sizeof(ShcheduleHeader), (uint8_t*)&DefaultEvents, sizeof(DefaultEvents));
	}
	else {
		header = bufHeader;
		EEPROMUtils::read_bytes(memAddr + sizeof(ShcheduleHeader), (uint8_t*)&DefaultEvents, header.numRecords * sizeof(ScheduleEvent));
	}
}

void printDateTime(String title, DateTime& dt);

boolean WaterSchedule::isInActiveDateRange() {
	if (weatherManager.maxTemperature < header.minTemperature) return false;

	DateTime bufDateStart(header.startDate);
	DateTime bufDateEnd(header.stopDate);

	now = rtc.now();

	DateTime startDate(now.year(), bufDateStart.month(), bufDateStart.day(), bufDateStart.hour(), bufDateStart.minute(), bufDateStart.second());
	DateTime endDate(now.year(), bufDateEnd.month(), bufDateEnd.day(), bufDateEnd.hour(), bufDateEnd.minute(), bufDateEnd.second());

	boolean result = now.secondstime() >= startDate.secondstime() && now.secondstime() <= endDate.secondstime();;
	return result;
}


boolean WaterSchedule::checkEventSecondaryConditions(const ScheduleEvent& event) {
	boolean result = true;

	if (event.id == NO_ID) return true;

	if (weatherManager.maxTemperature < event.minTemperature) {
#ifdef _DEBUG
		Serial.println(F("Temperature is too low. Skipping event"));
#endif
		result = false;
	}

	// Check recent rain flag
	LastRainInfo rainInfo;
	rainSensor.getLastRainInfo(rainInfo);

	if (rainInfo.startTime > 0 && rainInfo.startTime < 0xFFFFFFFF) {
		DateTime lastRaintTime(rainInfo.startTime);
		TimeSpan rainSpan = now - lastRaintTime;

		if (rainSpan.days() <= 1 && rainInfo.enoughRainPoured() && (event.flags & EventFlags::SkipIfRecentRain) != 0) {
#ifdef _DEBUG
			Serial.println(F("Enough rain water already. Skipping event"));
#endif
			result = false;
		}

		if (!(rainSpan.days() <= 1 && rainInfo.enoughRainPoured()) && (event.flags & EventFlags::OnlyAfterRecentRain) != 0) {
#ifdef _DEBUG
			Serial.println(F("No recent rain for this event. Skipping event"));
#endif
			result = false;
		}
	}

	// Skip even if this is not in allowed period
	if (event.lastActionTime > 0) {
		DateTime lastActionTime(event.lastActionTime);
		TimeSpan span = (lastActionTime - now) + TimeSpan(event.duration * 2);
#ifdef _DEBUG
		printDateTime("Event last action time: ", lastActionTime);
		Serial.print(F("Hours passed: ")); Serial.println(span.hours());
#endif

		// Block already performed event for 12 hours
		/*if (span.hours() < 12) {
#ifdef _DEBUG
			Serial.println(F("Less than 12 hours passed from prior event. Skipping event."));
#endif
			result = false;
		}*/

		if (event.repeatPeriodDays > 1) {
			if (span.days() < event.repeatPeriodDays) {
#ifdef _DEBUG
			Serial.println(F("Not enough days passed from prior periodic event. Skipping event."));
#endif
				result = false;
			}
		}
	}

#ifdef _DEBUG
	Serial.print(F("checkEventSecondaryConditions result = ")); Serial.println(result ? F("TRUE") : F("FALSE"));
#endif
	return result;
}

boolean WaterSchedule::isEventAppropriate(const ScheduleEvent& event) {
	boolean result = false;
	if (event.type == EventType::None || (event.flags & EventFlags::Active) == 0) return false;
	if (event.id == NO_ID) return true;

	lastCheckTime.restart();

#ifdef _DEBUG
	Serial.print(F("Event type: ")); Serial.print(event.type);
	Serial.print(F(" Event ID: ")); Serial.println(event.id);
	Serial.print(F("Temperature : ")); Serial.println(weatherManager.maxTemperature);
#endif
	now = rtc.now();
	DateTime eventTime(event.checkTime);
	DateTime actionTime(now.year(), now.month(), now.day(), eventTime.hour(), eventTime.minute(), eventTime.second());

	if (now.secondstime() >= actionTime.secondstime()) {
		long eventSpan = now.secondstime() - actionTime.secondstime();

		if (eventSpan < event.duration) {
			result = checkEventSecondaryConditions(event);

			// Skip event if enough water already poured today
			if (result && event.type == EventType::WaterOut) {
				uint32_t maxTodayLiters = getTodayMaxPouring(event.valves);
				uint32_t alreadyPoured = waterFlowMeter.getStatistics().today.litres;

#ifdef _DEBUG
				Serial.print(F("maxTodayLiters: ")); Serial.println(maxTodayLiters);
				Serial.print(F("alreadyPoured: ")); Serial.println(alreadyPoured);
#endif
				if (alreadyPoured + (event.liters / 2) > maxTodayLiters) {
#ifdef _DEBUG
					Serial.println(F("Enough water already poured today"));
#endif
					result = false;
				}
			}
		}
	}

#ifdef _DEBUG
	Serial.print(F("Event appropriate: ")); Serial.println(result ? F("TRUE") : F("FALSE"));
#endif
	return result;
}

boolean WaterSchedule::scanEvents() {
	lastScanTime.restart();
#ifdef _DEBUG
	Serial.println(F("scanEvents()"));
#endif
	if (!isInActiveDateRange()) {
#ifdef _DEBUG
		Serial.println(F("Out of active date range"));
#endif
		return false;
	}

	if (isEventAppropriate(currentEvent)) {
#ifdef _DEBUG
		Serial.println(F("Current event still appropriate"));
		Serial.print(F("Event ID = ")); Serial.println(currentEvent.id);
#endif
		return true;
	}

	boolean result = false;
	for (int i = 0; i < header.numRecords && result == false; i++) {
		if (DefaultEvents[i].type == EventType::None) break;	// This is terminating event
#ifdef _DEBUG
		Serial.print(F("Event ID = ")); Serial.println(DefaultEvents[i].id);
#endif
		if ((DefaultEvents[i].flags & EventFlags::Active) != 0) {
			result = isEventAppropriate(DefaultEvents[i]);
			if (result) {
				currentEvent = DefaultEvents[i]; break;
			}
		}
	}

	if (!result) {
#ifdef _DEBUG
		Serial.println(F("No current event selected"));
#endif
		if (currentEvent.type != EventType::None) {
			dismissCurrentEvent();
		}
	}
	return result;
}

uint32_t WaterSchedule::getTodayMaxPouring(uint8_t valveFlags) {
	uint32_t result = 0;
	for (int i = 0; i < header.numRecords && DefaultEvents[i].type != EventType::None; i++) {
		ScheduleEvent& event = DefaultEvents[i];
		if (event.type == EventType::WaterOut && (event.flags & EventFlags::Active) != 0 && (event.valves & valveFlags) != 0) {
			result += event.liters;
		}
	}
	return result;
}

void WaterSchedule::setCurrentEvent(const ScheduleEvent& event) {
	if (currentEvent.type != EventType::None) {
		dismissCurrentEvent();
	}
	currentEvent = event;
}

void WaterSchedule::dismissCurrentEvent() {
	if (waterInValve.isOpen()) waterInValve.closeValve();
	if (waterOutValve.isOpen()) waterOutValve.closeValve();

	if (currentEvent.id != NO_ID) {
#ifdef _DEBUG
		Serial.println(F(">>> dismissCurrentEvent()"));
		Serial.print(F("Event ID = ")); Serial.println(currentEvent.id);
#endif
		for (int i = 0; i < header.numRecords && DefaultEvents[i].type != EventType::None; i++) {
			if (currentEvent.id == DefaultEvents[i].id) {
				DateTime now = rtc.now();
				DefaultEvents[i].lastActionTime = now.unixtime();
				EEPROMUtils::save_bytes(memAddr + sizeof(ShcheduleHeader) + (i * sizeof(ScheduleEvent)), (uint8_t*)&DefaultEvents[i], sizeof(ScheduleEvent));
				break;
			}
		}
	}
	// TODO: log completed event
	currentEvent.type = EventType::None;
}
