/*
 * EEPROMUtils.cpp
 *
 *  Created on: 6 авг. 2017 г.
 *      Author: dennis
 */

#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include "EEPROMUtils.h"

static void EEPROMUtils::save(int addr, uint8_t byte) {
	EEPROM.write(addr, byte);
}

static uint8_t EEPROMUtils::read(int addr) {
	return EEPROM.read(addr);
}


static void EEPROMUtils::save_bytes(int addr, uint8_t *raw, uint16_t size) {
	for (uint16_t i = 0; i < size; i++)
		EEPROM.write(addr + i, raw[i]);
}

static void EEPROMUtils::read_bytes(int addr, uint8_t *raw, uint16_t size) {
	for (uint16_t i = 0; i < size; i++)
		raw[i] = EEPROM.read(addr + i);
}

static void EEPROMUtils::saveUInt(int addr, uint16_t& value) {
	EEPROMUtils::save_bytes(addr, (uint8_t*)&value, sizeof(uint16_t));
}

static uint16_t EEPROMUtils::readUInt(int addr) {
	uint16_t value;
	EEPROMUtils::read_bytes(addr, (uint8_t*)&value, sizeof(uint16_t));
	return value;
}

static void EEPROMUtils::saveULong(int addr, unsigned long& value) {
	EEPROMUtils::save_bytes(addr, (uint8_t*)&value, sizeof(unsigned long));
}

static unsigned long EEPROMUtils::readULong(int addr) {
	unsigned long value;
	EEPROMUtils::read_bytes(addr, (uint8_t*)&value, sizeof(unsigned long));
	return value;
}

void writeFlash(int deviceaddress, unsigned int eeaddress, uint8_t data )
{
	Wire.beginTransmission(deviceaddress);
	Wire.write((int)(eeaddress >> 8));   // MSB
	Wire.write((int)(eeaddress & 0xFF)); // LSB
	Wire.write(data);
	Wire.endTransmission();

	delay(5);
}

uint8_t readFlash(int deviceaddress, unsigned int eeaddress )
{
	uint8_t rdata = 0xFF;

	Wire.beginTransmission(deviceaddress);
	Wire.write((int)(eeaddress >> 8));   // MSB
	Wire.write((int)(eeaddress & 0xFF)); // LSB
	Wire.endTransmission();

	Wire.requestFrom(deviceaddress,1);

	if (Wire.available()) rdata = Wire.read();

	return rdata;
}

void writeFlash(int deviceaddress, unsigned int eeaddress, uint8_t* data, uint16_t size) {
	for (uint16_t i = 0; i < size; i++, eeaddress++)
	{
		Wire.beginTransmission(deviceaddress);
		Wire.write((int)(eeaddress >> 8));   // MSB
		Wire.write((int)(eeaddress & 0xFF)); // LSB
		Wire.write(data[i]);
		Wire.endTransmission();
		delay(5);
	}
}

void readFlash(int deviceaddress, unsigned int eeaddress, uint8_t* data, uint16_t size) {
	for (uint16_t i = 0; i < size; i++, eeaddress++)
	{
		Wire.beginTransmission(deviceaddress);
		Wire.write((int)(eeaddress >> 8));   // MSB
		Wire.write((int)(eeaddress & 0xFF)); // LSB
		Wire.endTransmission();

		Wire.requestFrom(deviceaddress,1);

		if (Wire.available()) data[i] = Wire.read();
	}
}

