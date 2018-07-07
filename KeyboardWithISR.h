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
#include "CommonDefs.h"

class KeyboardWithISR  {
public:
	uint8_t pinCLK;
	uint8_t pinDT;
	uint8_t pinSW;

	uint8_t maxPos;
	int curPos;
	boolean pressed;
	boolean rotated;
	int increment;

	Chrono pressChrono;
	Chrono lastPressedChrono;
	Chrono rotatedChrono;

public:
	KeyboardWithISR(uint8_t _pinCLK, uint8_t _pinDT, uint8_t _pinSW, uint8_t _maxPos);
	virtual ~KeyboardWithISR();

	void setMaxPos(int _maxPos) { maxPos = _maxPos; curPos = 0; }
	uint8_t getMaxPos() { return maxPos; }

	void checkPressed();
	boolean isPressed() { int p = pressed; pressed = false; return p; }
	boolean isRotated() { int r = rotated; rotated = false; return r; }

	int getIncrement() { int i = increment; increment = 0; return i; }
	int getPos() { return curPos; }
	void setPos(int pos) { curPos = pos; }
};

// Keyboard controls
extern KeyboardWithISR keyboard;


#endif /* KEYBOARDWITHISR_H_ */
