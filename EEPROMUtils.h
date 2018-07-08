/*
 * EEPROMUtils.h
 *
 *  Created on: 6 авг. 2017 г.
 *      Author: dennis
 */

#ifndef EEPROMUTILS_H_
#define EEPROMUTILS_H_


class EEPROMUtils {
public:
	static void save(int addr, uint8_t byte);
	static uint8_t read(int addr);

	static void save_bytes(int addr, uint8_t *raw, uint16_t size);
	static void read_bytes(int addr, uint8_t *raw, uint16_t size);

	static void saveULong(int addr, unsigned long& value);
	static unsigned long readULong(int addr);

	static void EEPROMUtils::saveUInt(int addr, uint16_t& value);
	static uint16_t EEPROMUtils::readUInt(int addr);
};

void writeFlash(int deviceaddress, unsigned int eeaddress, uint8_t data);
uint8_t readFlash(int deviceaddress, unsigned int eeaddress );

void writeFlash(int deviceaddress, unsigned int eeaddress, uint8_t* data, uint16_t size);
void readFlash(int deviceaddress, unsigned int eeaddress, uint8_t* data, uint16_t size);

#endif /* EEPROMUTILS_H_ */
