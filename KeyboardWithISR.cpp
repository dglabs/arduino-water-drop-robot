/*
 * KeyboardWithISR.cpp
 *
 *  Created on: 8 июн. 2017 г.
 *      Author: dennis
 */

#include "KeyboardWithISR.h"

const long KEY_PRESS_MILLIS = 50l;
const long KEY_LONG_PRESS_MILLIS = 800l;

KeyboardWithISR *instance;

void key1_ISR() {
	if (digitalRead(instance->keyPins[0]) == LOW) {
		 if (!instance->key0_Chrono.isRunning()) {
			 instance->pressed[0] = true;	// temporary consider key pressed
			 instance->key0_Chrono.restart();
		 }
	}
	else {
		if (instance->key0_Chrono.isRunning()) {
			// If long pressed interval satisfied, then set long press for key
			if (instance->key0_Chrono.hasPassed(KEY_LONG_PRESS_MILLIS)) {
				instance->pressed[0] = false;
				instance->longPressed[0] = true;
			}
			// Else consider short press if time passed
			else if (instance->key0_Chrono.hasPassed(KEY_PRESS_MILLIS)) {
				instance->pressed[0] = true;
				instance->longPressed[0] = false;
			}
			else {	// veryshort press. Ignoring
				instance->pressed[0] = false;
				instance->longPressed[0] = false;
			}
		}
		else {	// Id chrono is not running, then consider artifact and ignore
			instance->pressed[0] = instance->longPressed[0] = false;
		}
		// Stop key chrono
		instance->key0_Chrono.restart();
		instance->key0_Chrono.stop();
	}
}

void key2_ISR() {
	if (digitalRead(instance->keyPins[1]) == LOW) {
		 if (!instance->key1_Chrono.isRunning()) {
			 instance->pressed[1] = true;	// temporary consider key pressed
			 instance->key1_Chrono.restart();
		 }
	}
	else {
		if (instance->key1_Chrono.isRunning()) {
			// If long pressed interval satisfied, then set long press for key
			if (instance->key1_Chrono.hasPassed(KEY_LONG_PRESS_MILLIS)) {
				instance->pressed[1] = false;
				instance->longPressed[1] = true;
			}
			// Else consider short press if time passed
			else if (instance->key1_Chrono.hasPassed(KEY_PRESS_MILLIS)) {
				instance->pressed[1] = true;
				instance->longPressed[1] = false;
			}
			else {	// veryshort press. Ignoring
				instance->pressed[1] = false;
				instance->longPressed[1] = false;
			}
		}
		else {	// Id chrono is not running, then consider artifact and ignore
			instance->pressed[1] = instance->longPressed[1] = false;
		}
		// Stop key chrono
		instance->key1_Chrono.restart();
		instance->key1_Chrono.stop();
	}
}

KeyboardWithISR::KeyboardWithISR(const uint8_t* _keyPins) :
		key0_Chrono(Chrono::MILLIS),
		key1_Chrono(Chrono::MILLIS),
		keyPins(_keyPins)
{
	for (int i = 0; i < KEYS_COUNT; i++)
		pinMode(keyPins[i], INPUT_PULLUP);

	instance = this;
	attachInterrupt(digitalPinToInterrupt(keyPins[0]), key1_ISR, CHANGE);
	/*if (KEYS_COUNT > 1)
		attachInterrupt(digitalPinToInterrupt(keyPins[1]), key2_ISR, CHANGE);*/
}

void KeyboardWithISR::clear() {
	key0_Chrono.restart(); key0_Chrono.stop();
	key1_Chrono.restart(); key1_Chrono.stop();
	for (int i = 0; i < KEYS_COUNT; i++) {
		pressed[i] = false;
		longPressed[i] = false;
	}
}

void KeyboardWithISR::refresh() {
	key2_ISR();
}


KeyboardWithISR::~KeyboardWithISR() {
	detachInterrupt(digitalPinToInterrupt(keyPins[0]));
	if (KEYS_COUNT > 1)
		detachInterrupt(digitalPinToInterrupt(keyPins[1]));
}

boolean KeyboardWithISR::isPressed(uint8_t index) {
	boolean result = pressed[index];
	pressed[index] = false;
	return result;
}


boolean KeyboardWithISR::isLongPressed(uint8_t index)
{
	boolean result = longPressed[index];
	longPressed[index] = false;
	return result;

}



