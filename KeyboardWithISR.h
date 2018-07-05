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
	uint8_t pinSW;
	uint8_t pinCLK;
	uint8_t pinDT;

	uint8_t maxPos;
	int curPos;
	boolean pressed;
	boolean rotated;
	boolean rotating;

	Chrono pressChrono;
	Chrono pressedChrono;
	Chrono rotatedChrono;

	void checkPressed();

public:
	KeyboardWithISR(uint8_t _pinCLK, uint8_t _pinDT, uint8_t _pinSW, uint8_t _maxPos);
	virtual ~KeyboardWithISR();

	void setMaxPos(int _maxPos) { maxPos = _maxPos; curPos = 0; }
	uint8_t getMaxPos() { return maxPos; }

	boolean isPressed() { int p = pressed; pressed = false; return p; }
	boolean isRotated() { int r = rotated; rotated = false; return r; }
	int getPos() { return curPos; }

	void tick();
};

#endif /* KEYBOARDWITHISR_H_ */
