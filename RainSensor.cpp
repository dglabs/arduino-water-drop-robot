/*
 * RainSensor.cpp
 *
 *  Created on: 28 июл. 2017 г.
 *      Author: dennis
 */

#include "RainSensor.h"
#include "WaterDropRobot.h"
#include "EEPROMUtils.h"

const char* INTENSITY_NAMES[] = {"No rain", "Mist", "Moderate", "Intense" };

RainSensor::RainSensor(const uint8_t _pin, const int _memAddress):
	pin(_pin)
	, memAddress(_memAddress)
	, lastIntensity(RainIntensity::none)
	, maxIntensity(RainIntensity::none)
	, rainStartedTime(0)
	, lastRainInfo()
{
	pinMode(pin, INPUT);
}

void RainSensor::setup() {
	EEPROMUtils::read_bytes(memAddress, (uint8_t *)&lastRainInfo, sizeof(LastRainInfo));
	if (lastRainInfo.signature != RAIN_SIGNATURE) {
		lastRainInfo = LastRainInfo();
		EEPROMUtils::save_bytes(memAddress, (uint8_t *)&lastRainInfo, sizeof(LastRainInfo));
	}
}

RainIntensity RainSensor::valueToIntensity(int value) const {
	if (value >= 700) return RainIntensity::none;
	else if (value < 700 && value >= 600) return RainIntensity::mist;
	else if (value < 600 && value >= 400) return RainIntensity::moderate;
	else return RainIntensity::intense;
}

RainIntensity RainSensor::getIntensity() {
	RainIntensity newValue = valueToIntensity(analogRead(pin));
	if (newValue != lastIntensity) {
		if (newValue > RainIntensity::none && rainStartedTime == 0)
			rainStartedTime = rtc.now().unixtime();

		if (newValue > maxIntensity) maxIntensity = newValue;

		if (newValue == RainIntensity::none && lastIntensity > RainIntensity::none) {
			// If rain is stopped, then save it as last rain into storage
			uint32_t duration;
			if (rainStartedTime > 0 && (duration = rtc.now().unixtime() - rainStartedTime) >= MIN_RAIN_DURATION) {
				lastRainInfo.startTime = rainStartedTime;
				lastRainInfo.duration = duration;
				lastRainInfo.intensity = maxIntensity;
				maxIntensity = RainIntensity::none;
				EEPROMUtils::save_bytes(memAddress, (uint8_t *)&lastRainInfo, sizeof(lastRainInfo));
			}
			rainStartedTime = 0;
		}
		lastIntensity = newValue;
	}
	return lastIntensity;
}

static const char* RainSensor::getIntensityAsString(RainIntensity intensity) {
	return INTENSITY_NAMES[intensity];
}

const char* RainSensor::getIntensityString() {
	return INTENSITY_NAMES[getIntensity()];
}

void RainSensor::getLastRainInfo(LastRainInfo& info) const {
	// If it's currently raining, then use current rain info
	uint32_t duration;
	if (lastIntensity > RainIntensity::mist && rainStartedTime > 0 &&
			(duration = rtc.now().unixtime() - rainStartedTime) >= MIN_RAIN_DURATION) {
		info.startTime = rainStartedTime;
		info.duration = rtc.now().unixtime() - rainStartedTime;
		info.intensity = lastIntensity;
	}
	// Else provide last rain info
	else info.copy(lastRainInfo);
}

unsigned long RainSensor::secondsFromRainStarted() const
	{ return rainStartedTime > 0 ? rtc.now().unixtime() - rainStartedTime : 0; }

