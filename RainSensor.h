/*
 * RainSensor.h
 *
 *  Created on: 28 июл. 2017 г.
 *      Author: dennis
 */

#ifndef RAINSENSOR_H_
#define RAINSENSOR_H_

#include <Arduino.h>
#include "RTClib.h"

const uint32_t MIN_RAIN_DURATION = 600l;	// In unixtime seconds
const uint32_t MIN_RAIN_TIME_TO_OPEN_COVER = 240l;	// In unixtime seconds

enum RainIntensity { none, mist, moderate, intense };
const RainIntensity MIN_RAIN_INTENSITY = RainIntensity::moderate;

const uint16_t RAIN_SIGNATURE = 0x7A93U;

struct LastRainInfo {
	uint16_t signature;
	uint32_t startTime;
	uint32_t duration;
	RainIntensity intensity;

	LastRainInfo() :
		signature(RAIN_SIGNATURE)
		, startTime(0)
		, duration(0)
		, intensity(RainIntensity::none) {}

	LastRainInfo(const uint32_t _startTime, const uint32_t _duration, const RainIntensity _intensity) :
		signature(RAIN_SIGNATURE)
		, startTime(_startTime)
		, duration(_duration)
		, intensity(_intensity) {}

	LastRainInfo(const LastRainInfo& src) :
		signature(RAIN_SIGNATURE)
		, startTime(src.startTime)
		, duration(src.duration)
		, intensity(src.intensity) {}

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
	RainIntensity maxIntensity;
	uint32_t rainStartedTime;	// unixtime seconds

	LastRainInfo lastRainInfo;

	RainIntensity valueToIntensity(int value) const;
public:

	RainSensor(const uint8_t _pin, const int _memAddress);

	void setup();

	RainIntensity getIntensity();
	const char* getIntensityString();
	unsigned long secondsFromRainStarted() const;

	void getLastRainInfo(LastRainInfo& info) const;

	static const char* getIntensityAsString(RainIntensity intensity);
};



// Rain handling
extern RainSensor rainSensor;


#endif /* RAINSENSOR_H_ */
