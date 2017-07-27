/*
 * KeyboardWithISR.h
 *
 *  Created on: 8 июн. 2017 г.
 *      Author: dennis
 */

#ifndef KEYBOARDWITHISR_H_
#define KEYBOARDWITHISR_H_

#include <Arduino.h>
#include <Chrono.h>

const int KEYS_COUNT = 2;

class KeyboardWithISR {
public:
	const uint8_t* keyPins;

	volatile boolean pressed[KEYS_COUNT];
	volatile boolean longPressed[KEYS_COUNT];

	Chrono keyChronos[KEYS_COUNT];

	Chrono key0_Chrono;
	Chrono key1_Chrono;

public:
	enum KEYS { KEY0, KEY1 };

	KeyboardWithISR(const uint8_t* keyPins);
	virtual ~KeyboardWithISR();

	boolean isPressed(uint8_t index);
	boolean isLongPressed(uint8_t index);
	void refresh();	// Scan keyboard

	void clear();
};

#endif /* KEYBOARDWITHISR_H_ */
