/*
 * WaterMotorizedValve.cpp
 *
 *  Created on: 7 июн. 2017 г.
 *      Author: dennis
 */

#include "WaterMotorizedValve.h"
#include "EEPROMUtils.h"

const int MOTOR_POWER_OFF = 0;
const int MOTOR_POWER_LOW = 100;
const int MOTOR_POWER_FULL = 160;

const int DIRECTION_OPEN = LOW;
const int DIRECTION_CLOSE = HIGH;

WaterMotorizedValve::WaterMotorizedValve(const uint8_t _valveMask
#ifdef BOARD_V2
		, PCF8574& _portExtender
#endif
		, const int _memAddress
		, const uint8_t _motorOpenPin
		, const uint8_t _motorClosePin
#ifndef BOARD_V2
		, const uint8_t _signalPinOpen /*= 0*/
		, const uint8_t _signalPinClosed /*= 0*/
#endif
	) :
#ifdef BOARD_V2
	Valve(_valveMask, _portExtender)
#else
Valve(_valveMask)
#endif
	, motorOpenPin(_motorOpenPin)
	, motorClosePin(_motorClosePin)
#ifndef BOARD_V2
	, signalPinOpen( _signalPinOpen)
	, signalPinClosed( _signalPinClosed)
#endif
	, memAddress(_memAddress)
	, valveTransitChrono(Chrono::MILLIS)
#ifndef BOARD_V2
	, signalPin(0)
	, initialSignalPin(LOW)
	, signalPinChanged(false)
#endif
{
#ifndef BOARD_V2
	pinMode(motorOpenPin, OUTPUT); analogWrite(motorOpenPin, 0);
	pinMode(motorClosePin, OUTPUT); analogWrite(motorClosePin, 0);
	if (signalPinOpen > 0)
		pinMode(signalPinOpen, INPUT_PULLUP);
	if (signalPinClosed > 0)
		pinMode(signalPinClosed, INPUT_PULLUP);
#endif
}

void WaterMotorizedValve::setup() {
	activeValves = EEPROMUtils::read(memAddress);
	//Serial.print("Current state: "); Serial.println(currentState, HEX);

	if (activeValves != valvesMask && activeValves != 0) {
		state = State::Open;
		activeValves = valvesMask;
		EEPROMUtils::save(memAddress, state);
	}

	state = activeValves != 0 ? State::Open : State::Closed;
}


WaterMotorizedValve::~WaterMotorizedValve() {}

void WaterMotorizedValve::loop() {
	switch (state) {
	case State::Opening: case State::Closing:
		boolean stop = false;
		if (valveTransitChrono.elapsed() < VALVE_TRANSIT_TIMEOUT_MILLIS_MAX) {
#ifndef BOARD_V2
			if (signalPin > 0) {
				if (initialSignalPin != digitalRead(signalPin)) signalPinChanged = true;
				if (valveTransitChrono.elapsed() > VALVE_TRANSIT_TIMEOUT_MILLIS_MIN /*&& signalPinChanged*/ && digitalRead(signalPin) == LOW) stop = true;
			}
#endif
		}
		else stop = true;
		if (stop) {
			analogWrite(motorOpenPin, 0);
			analogWrite(motorClosePin, 0);
			switch (state) {
			case State::Opening:
				setValvePosition(State::Open);
				break;
			case State::Closing: {
				activeValves = 0;
				EEPROMUtils::save(memAddress, activeValves);
				setValvePosition(State::Closed);
			} break;
			}
		} break;
	}
}

void WaterMotorizedValve::processValve(State position
#ifndef BOARD_V2
		, int _signalPin
#endif
	) {
	int activePin = 0;

#ifndef BOARD_V2
	signalPin = _signalPin;
#endif
	switch (position) {
	case State::Open:
		activePin = motorOpenPin;
		analogWrite(motorClosePin, 0);
		setValvePosition(State::Opening);
		break;
	case State::Closed:
		activePin = motorClosePin;
		analogWrite(motorOpenPin, 0);
		setValvePosition(State::Closing);
		break;
	}
	analogWrite(activePin, MOTOR_POWER_FULL);

	valveTransitChrono.restart(0);
#ifndef BOARD_V2
	initialSignalPin = (signalPin > 0) ?  digitalRead(signalPin) : HIGH;
	signalPinChanged = false;
#endif

	/*do {
		delay(250);

		if (signalPin > 0) {
			//if (initialSignalPin != digitalRead(signalPin)) signalPinChanged = true;
			if (valveTransitChrono.elapsed() > VALVE_TRANSIT_TIMEOUT_MILLIS_MIN && digitalRead(signalPin) == LOW) break;
		}
	} while (valveTransitChrono.elapsed() < VALVE_TRANSIT_TIMEOUT_MILLIS_MAX);
	analogWrite(motorOpenPin, 0);
	analogWrite(motorClosePin, 0);*/
}

boolean WaterMotorizedValve::openValve(const uint8_t _valvesMask /*= 0xFF*/, boolean manual /*= false*/) {
	Valve::openValve(_valvesMask, manual);

	if (activeValves == _valvesMask) return false;
	activeValves = _valvesMask;
	EEPROMUtils::save(memAddress, activeValves);

	processValve(State::Open
#ifndef BOARD_V2
			, signalPinOpen
#endif
	);

	return true;
}

boolean WaterMotorizedValve::closeValve() {
	if (activeValves == 0) return false;

	processValve(State::Closed
#ifndef BOARD_V2
			, signalPinClosed
#endif
	);
	return true;
}

boolean WaterMotorizedValve::isOpen() {
#ifndef BOARD_V2
	if (signalPinOpen > 0)
		if ((digitalRead(signalPinOpen) ^ digitalRead(signalPinClosed)))
			return digitalRead(signalPinOpen) == LOW;
		else return true;	// This is undefined state and treat it as open
	else
#endif
		return activeValves != 0;
}

boolean WaterMotorizedValve::isClosed() {
#ifndef BOARD_V2
	if (signalPinClosed > 0) {
		if ((digitalRead(signalPinOpen) ^ digitalRead(signalPinClosed)))
			return digitalRead(signalPinClosed) == LOW;
		else return false;	// This is undefined state and treat it as open
	}
	else
#endif
		return activeValves == 0;
}

