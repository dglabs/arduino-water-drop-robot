/*
 * WaterFlowMeter.cpp
 *
 *  Created on: 27 июл. 2017 г.
 *      Author: dennis
 */

#include <Arduino.h>
#include <RTClib.h>
#include "WaterFlowMeter.h"
#include "WaterDropRobot.h"
#include "EEPROMUtils.h"

volatile unsigned long flowCounter = 0;

const int ISR_PIN = 3;
const long TICKS_PER_LITER = 410;

void waterFlow_ISR() {
	flowCounter++;
}

WaterFlowMeter::WaterFlowMeter(const int _memAddress):
	memAddress(_memAddress)
	, startedChrono(Chrono::SECONDS)
	, totalVolume(0)
{
	pinMode(ISR_PIN, INPUT);
	digitalWrite(ISR_PIN, LOW);

	flowCounter = 0;
	attachInterrupt(digitalPinToInterrupt(ISR_PIN), waterFlow_ISR, RISING);
}

void WaterFlowMeter::initStatistics() {
	totalVolume = EEPROMUtils::readULong(memAddress);
	if (totalVolume > 0xEFFFFFFF) {
		totalVolume = 0;
		EEPROMUtils::saveULong(memAddress, totalVolume);
	}

	EEPROMUtils::read_bytes(memAddress + sizeof(totalVolume), (uint8_t *)&statistics, sizeof(PourStatistics));
	if (statistics.signature != FLOW_SIGNATURE) {
		DateTime now = rtc.now();
		DateTime startOfDay(now.year(), now.month(), now.day(), 0, 0, 0);

		statistics.init(startOfDay.unixtime());
		EEPROMUtils::save_bytes(memAddress + sizeof(totalVolume), (uint8_t *)&statistics, sizeof(PourStatistics));
	}
	else adjustStatictcs();
}

WaterFlowMeter::~WaterFlowMeter() {
	detachInterrupt(digitalPinToInterrupt(ISR_PIN));
}

void WaterFlowMeter::startWaterOut() {
	flowCounter = 0;
	startedChrono.restart();
}

unsigned long WaterFlowMeter::stopWaterOut() {
	unsigned long volume = getVolumeFromStart();
	totalVolume += volume;
	flowCounter = 0;
	EEPROMUtils::saveULong(memAddress, totalVolume);

	statistics.total.litres += volume;
	statistics.lastYear.litres += volume;
	statistics.lastMonth.litres += volume;
	statistics.today.litres += volume;
	if (!adjustStatictcs())
		EEPROMUtils::save_bytes(memAddress + sizeof(totalVolume), (uint8_t *)&statistics, sizeof(PourStatistics));

	startedChrono.stop();
	return volume;
}

unsigned long WaterFlowMeter::getVolumeFromStart() const {
	return flowCounter / TICKS_PER_LITER;
}

unsigned long WaterFlowMeter::getTotalVolume() {
	totalVolume = flowCounter;
	return totalVolume / TICKS_PER_LITER;
}

unsigned long WaterFlowMeter::getVolumePerMinute() const {
	if (startedChrono.elapsed() > 0 && getVolumeFromStart() > 0) {
		long result = (getVolumeFromStart() * 100) / (startedChrono.elapsed() / 60) / 100;
		return (result < 0 || result > 2000000) ? 0 : result;
	}
	return 0;
}

boolean WaterFlowMeter::adjustStatictcs() {
	boolean adjusted = false;

	DateTime now = rtc.now();
	DateTime startOfDay = DateTime(now.year(), now.month(), now.day(), 0, 0, 0);

	DateTime todayStart(statistics.today.startTime);
	DateTime monthStart(statistics.lastMonth.startTime);
	DateTime yearStart(statistics.lastYear.startTime);

	if (startOfDay.day() != todayStart.day()) {
		statistics.today.startTime = startOfDay.unixtime();
		statistics.today.litres = 0;
		adjusted = true;
	}

	if (startOfDay.month() != monthStart.month()) {
		statistics.lastMonth.startTime = startOfDay.unixtime();
		statistics.lastMonth.litres = 0;
	}

	if (startOfDay.year() != monthStart.year()) {
		statistics.lastYear.startTime = startOfDay.unixtime();
		statistics.lastYear.litres = 0;
	}

	if (adjusted)
		EEPROMUtils::save_bytes(memAddress + sizeof(totalVolume), (uint8_t *)&statistics, sizeof(PourStatistics));

	return adjusted;
}
