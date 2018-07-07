/*
 * KeyboardWithISR.cpp
 *
 *  Created on: 8 июн. 2017 г.
 *      Author: dennis
 */

#include "KeyboardWithISR.h"
#include <PinChangeInterrupt.h>
#include "CommonDefs.h"
#include "RobotDisplay.h"

KeyboardWithISR* instance = NULL;

void clk_ISR();
void dt_ISR();
void press_ISR();

KeyboardWithISR::KeyboardWithISR(uint8_t _pinCLK, uint8_t _pinDT, uint8_t _pinSW, uint8_t _maxPos) :
		pinCLK(_pinCLK)
		, pinDT(_pinDT)
		, pinSW(_pinSW)
		, maxPos(_maxPos)
		, curPos(0)
		, pressed(false)
		, rotated(false)
		, increment(0)
		, pressChrono(Chrono::MILLIS)
		, lastPressedChrono(Chrono::MILLIS)
		, rotatedChrono(Chrono::MILLIS)
{
	instance = this;

	pinMode(pinCLK, INPUT);
	digitalWrite(pinCLK, HIGH);      // turn on pull-up resistor
	pinMode(pinDT, INPUT);
	digitalWrite(pinDT, HIGH);       // turn on pull-up resistor
	pinMode(pinSW, INPUT);
	digitalWrite(pinSW, HIGH);       // turn on pull-up resistor
	pressChrono.stop();
	rotatedChrono.restart();
	lastPressedChrono.restart();

	/*if (pinCLK == 2 || pinCLK == 3)
		attachInterrupt(digitalPinToInterrupt(pinCLK), clk_ISR, CHANGE);
	else
		attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(pinCLK), clk_ISR, CHANGE);*/

	attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(pinDT), dt_ISR, CHANGE);
	attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(pinSW), press_ISR, LOW);
}

void KeyboardWithISR::checkPressed() {
	boolean p = digitalRead(instance->pinSW) == LOW;
	if (p) {
		if (pressChrono.isRunning())
		{
			if (pressChrono.elapsed() > 50) {
				if (lastPressedChrono.elapsed() > 500) {
					pressed = true;
					lastPressedChrono.restart();
				}
				pressChrono.stop();
			}
		}
		else if (!pressed) instance->pressChrono.restart();
	}
	else instance->pressChrono.stop();
}

void press_ISR() {
	instance->checkPressed();
}

/*void clk_ISR() {
	if (instance->rotatedChrono.elapsed() > 50) {
		int increment = 0;
		if (digitalRead(instance->pinCLK) == HIGH) {   // found a low-to-high on channel A
			if (digitalRead(instance->pinDT) == LOW) {  // check channel B to see which way
			  // encoder is turning
				increment = -1;         // CCW
			}
			else {
				increment = 1;         // CW
			}
		}
		else                                        // found a high-to-low on channel A
		{
			if (digitalRead(instance->pinDT) == LOW) {   // check channel B to see which way
			  // encoder is turning
				increment = -1;          // CW
			}
			else {
				increment = 1;          // CCW
			}
		}
		instance->curPos += instance->increment;
		instance->curPos = instance->curPos < 0 ? instance->maxPos - 1 : instance->curPos % instance->maxPos;
		instance->increment = increment;

		instance->rotated = increment != 0;
		instance->rotatedChrono.restart();
	}
}*/

void dt_ISR() {
	if (/*!instance->rotated && */instance->rotatedChrono.elapsed() > 50) {
		int increment = 0;
		if (digitalRead(instance->pinDT) == HIGH) {   // found a low-to-high on channel A
			if (digitalRead(instance->pinCLK) == LOW) {  // check channel B to see which way
			  // encoder is turning
				increment = -1;         // CCW
			}
			else {
				increment = 1;         // CW
			}
		}
		else                                        // found a high-to-low on channel A
		{
			if (digitalRead(instance->pinCLK) == LOW) {   // check channel B to see which way
			  // encoder is turning
				increment = -1;          // CW
			}
			else {
				increment = 1;          // CCW
			}
		}
		instance->curPos += increment;
		instance->curPos = instance->curPos < 0 ? instance->maxPos - 1 : instance->curPos % instance->maxPos;
		instance->increment = increment;

		instance->rotated = true;
		instance->rotatedChrono.restart();
	}
}

KeyboardWithISR::~KeyboardWithISR() {
	detachPinChangeInterrupt(digitalPinToPinChangeInterrupt(pinDT));
	detachPinChangeInterrupt(digitalPinToPinChangeInterrupt(pinSW));
	instance = NULL;
}
