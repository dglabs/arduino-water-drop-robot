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
};

#endif /* EEPROMUTILS_H_ */
