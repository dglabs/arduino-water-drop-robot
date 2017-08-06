/*
 * WaterFlowMeter.cpp
 *
 *  Created on: 27 июл. 2017 г.
 *      Author: dennis
 */

#include <Arduino.h>
#include "WaterFlowMeter.h"
#include <EEPROM.h>

volatile unsigned long flowCounter = 0;

const int ISR_PIN = 3;

void waterFlow_ISR() {
	flowCounter++;
	Serial.print(flowCounter);
	Serial.println();
}

WaterFlowMeter::WaterFlowMeter(const int _memAddress):
	memAddress(_memAddress)
	, totalVolume(0)
	, startedVolume(0)
	, startedChrono(Chrono::SECONDS){

	pinMode(ISR_PIN, INPUT);
	digitalWrite(ISR_PIN, LOW);

	totalVolume = startedVolume = readLong(memAddress);
	if (totalVolume > 0xEFFFFFFF) {
		totalVolume = startedVolume = 0;
		saveLong(memAddress, totalVolume);
	}

	flowCounter = totalVolume;

	attachInterrupt(ISR_PIN, waterFlow_ISR, RISING);
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
	saveLong(memAddress, totalVolume);
	startedChrono.stop();
	return volume;
}

unsigned long WaterFlowMeter::getVolumeFromStart() {
	return flowCounter - startedVolume;
}
unsigned long WaterFlowMeter::getTotalVolume() {
	totalVolume = flowCounter;
	return totalVolume;
}

void WaterFlowMeter::saveLong(int addr, unsigned long value) {
	uint8_t *raw = (uint8_t *)&value;
	for (uint16_t i = 0; i < sizeof(value); i++)
		EEPROM.write(addr + i, raw[i]);
}

unsigned long WaterFlowMeter::readLong(int addr) {
	unsigned long value = 0;
	uint8_t *raw = (uint8_t *)&value;
	for (uint16_t i = 0; i < sizeof(value); i++)
		raw[i] = EEPROM.read(addr + i);
	return value;
}

