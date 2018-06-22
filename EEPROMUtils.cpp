/*
 * EEPROMUtils.cpp
 *
 *  Created on: 6 авг. 2017 г.
 *      Author: dennis
 */

#define USE_AT24C1024

#ifdef USE_AT24C1024

	#include <inttypes.h>
	#include <AT24C1024.h>
	AT24C1024 eprom;

#else

	#include <EEPROM.h>

#endif
#include "EEPROMUtils.h"

static void EEPROMUtils::save(int addr, uint8_t byte) {
#ifdef USE_AT24C1024
	eprom.write(addr, byte);
#else
	EEPROM.write(addr, byte);
#endif
}

static uint8_t EEPROMUtils::read(int addr) {
#ifdef USE_AT24C1024
	return eprom.read(addr);
#else
	return EEPROM.read(addr);
#endif
}


static void EEPROMUtils::save_bytes(int addr, uint8_t *raw, uint16_t size) {
#ifdef USE_AT24C1024
	for (uint16_t i = 0; i < size; i++)
		eprom.write(addr + i, raw[i]);
#else
	for (uint16_t i = 0; i < size; i++)
		EEPROM.write(addr + i, raw[i]);
#endif
}

static void EEPROMUtils::read_bytes(int addr, uint8_t *raw, uint16_t size) {
#ifdef USE_AT24C1024
	for (uint16_t i = 0; i < size; i++)
		raw[i] = eprom.read(addr + i);
#else
	for (uint16_t i = 0; i < size; i++)
		raw[i] = EEPROM.read(addr + i);
#endif
}

static void EEPROMUtils::saveULong(int addr, unsigned long& value) {
	EEPROMUtils::save_bytes(addr, (uint8_t*)&value, sizeof(unsigned long));
}

static unsigned long EEPROMUtils::readULong(int addr) {
	unsigned long value;
	EEPROMUtils::read_bytes(addr, (uint8_t*)&value, sizeof(unsigned long));
	return value;
}
