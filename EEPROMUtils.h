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

	static void save_bytes(int addr, uint8_t *raw, uint16_t size);
	static void read_bytes(int addr, uint8_t *raw, uint16_t size);
};

#endif /* EEPROMUTILS_H_ */
