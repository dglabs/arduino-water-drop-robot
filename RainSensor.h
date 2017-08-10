/*
 * RainSensor.h
 *
 *  Created on: 28 июл. 2017 г.
 *      Author: dennis
 */

#ifndef RAINSENSOR_H_
#define RAINSENSOR_H_

#include <Arduino.h>
#include <RTClib.h>

const uint32_t MIN_RAIN_DURATION = 600l;	// In unixtime seconds
const uint32_t MIN_RAIN_TIME_TO_OPEN_COVER = 60l;	// In unixtime seconds

enum RainIntensity { none, mist, moderate, intense };
const RainIntensity MIN_RAIN_INTENSITY = RainIntensity::moderate;

struct LastRainInfo {
	uint32_t startTime;
	uint32_t duration;
	RainIntensity intensity;

	LastRainInfo() :
		startTime(0)
		, duration(0)
		, intensity(none) {}

	LastRainInfo(const uint32_t _startTime, const uint32_t _duration, const RainIntensity _intensity) :
		startTime(_startTime)
		, duration(_duration)
		, intensity(_intensity) {}

	void copy(const LastRainInfo& src) {
		this->startTime = src.startTime;
		this->duration = src.duration;
		this->intensity = src.intensity;
	}

	// Calculate if enough water poured depending of rain duration and intensity
	boolean enoughRainPoured() {
		switch (intensity) {
		case RainIntensity::mist: return duration > 14400;	// four hours mist rain
		case RainIntensity::moderate: return duration > 3600;	// one hour moderate rain
		case RainIntensity::intense: return duration > 1200;	// half hour intense rain
		default: return false;
		}
	}
};

const int STORAGE_SIZE_RAIN_SENSOR = sizeof(LastRainInfo);

class RainSensor {
private:
	const uint8_t pin;
	const int memAddress;

	RainIntensity lastIntensity;
	uint32_t rainStartedTime;	// unixtime seconds

	RTC_DS1307& rtc;
	LastRainInfo lastRainInfo;

	RainIntensity valueToIntensity(int value) const;
public:

	RainSensor(const uint8_t _pin, const int _memAddress, RTC_DS1307& _rtc);
	virtual ~RainSensor();

	RainIntensity getIntensity();
	const char* getIntensityString();
	unsigned long secondsFromRainStarted() const { return rainStartedTime > 0 ? rtc.now().unixtime() - rainStartedTime : 0; }

	void getLastRainInfo(LastRainInfo& info) const;
};

#endif /* RAINSENSOR_H_ */
