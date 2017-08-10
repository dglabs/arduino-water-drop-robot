/*
 * WaterFlowMeter.cpp
 *
 *  Created on: 27 июл. 2017 г.
 *      Author: dennis
 */

#include <Arduino.h>
#include "WaterFlowMeter.h"
#include <EEPROM.h>
#include "EEPROMUtils.h"

volatile unsigned long flowCounter = 0;

const int ISR_PIN = 3;
const long TICKS_PER_LITER = 410;

void waterFlow_ISR() {
	flowCounter++;
}

WaterFlowMeter::WaterFlowMeter(const int _memAddress):
	memAddress(_memAddress)
	, totalVolume(0)
	, startedVolume(0)
	, startedChrono(Chrono::SECONDS){

	pinMode(ISR_PIN, INPUT);
	digitalWrite(ISR_PIN, LOW);

	totalVolume = startedVolume = EEPROMUtils::readULong(memAddress);
	if (totalVolume > 0xEFFFFFFF) {
		totalVolume = startedVolume = 0;
		EEPROMUtils::saveULong(memAddress, totalVolume);
	}

	flowCounter = totalVolume;

	attachInterrupt(digitalPinToInterrupt(ISR_PIN), waterFlow_ISR, RISING);
}

WaterFlowMeter::~WaterFlowMeter() {
}

void WaterFlowMeter::startWaterOut() {
	totalVolume = startedVolume = flowCounter;
	startedChrono.restart();
}

unsigned long WaterFlowMeter::stopWaterOut() {
	unsigned long volume = getVolumeFromStart();
	totalVolume = startedVolume = flowCounter;
	EEPROMUtils::saveULong(memAddress, totalVolume);
	startedChrono.stop();
	return volume;
}

unsigned long WaterFlowMeter::getVolumeFromStart() const {
	return (flowCounter - startedVolume) / TICKS_PER_LITER;
}
unsigned long WaterFlowMeter::getTotalVolume(){
	totalVolume = flowCounter;
	return totalVolume / TICKS_PER_LITER;
}

unsigned long WaterFlowMeter::getVolumePerMinute() const {
	if (startedChrono.elapsed() > 0)
		return getVolumeFromStart() / startedChrono.elapsed() / 60;
	return 0;
}


