/*
 * EEPROMUtils.cpp
 *
 *  Created on: 6 авг. 2017 г.
 *      Author: dennis
 */

#include <EEPROM.h>
#include "EEPROMUtils.h"


static void EEPROMUtils::save_bytes(int addr, uint8_t *raw, uint16_t size) {
	for (uint16_t i = 0; i < size; i++)
		EEPROM.write(addr + i, raw[i]);
}

static void EEPROMUtils::read_bytes(int addr, uint8_t *raw, uint16_t size) {
	for (uint16_t i = 0; i < size; i++)
		raw[i] = EEPROM.read(addr + i);
}

static void EEPROMUtils::saveULong(int addr, unsigned long& value) {
	EEPROMUtils::save_bytes(addr, (uint8_t*)&value, sizeof(unsigned long));
}

static unsigned long EEPROMUtils::readULong(int addr) {
	unsigned long value;
	EEPROMUtils::read_bytes(addr, (uint8_t*)&value, sizeof(unsigned long));
	return value;
}
