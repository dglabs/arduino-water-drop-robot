/*
 * WaterMotorizedValve.cpp
 *
 *  Created on: 7 июн. 2017 г.
 *      Author: dennis
 */

#include "WaterMotorizedValve.h"
#include "WaterDropRobot.h"
#include "EEPROMUtils.h"

const int MOTOR_POWER_OFF = 0;
const int MOTOR_POWER_FULL = 140;

WaterMotorizedValve::WaterMotorizedValve(const uint8_t _valveMask
#ifdef BOARD_V2
		, const uint8_t* _motorENPins
		, const uint8_t _motorENCount
#endif
		, const int _memAddress
		, const uint8_t _pwm0Pin
		, const uint8_t _pwm1Pin
	) :
	Valve(_valveMask)
#ifdef BOARD_V2
	, motorENPins(motorENPins)
	, motorENCount(_motorENCount)
#endif
	, pwm0Pin(_pwm0Pin)
	, pwm1Pin(_pwm1Pin)
	, memAddress(_memAddress)
	, valveTransitChrono(Chrono::MILLIS)
	, selectedVolume(30)
{
	activeValves = 0;
	state = State::Closed;
}

void WaterMotorizedValve::setup() {
#ifdef BOARD_V2
	for (int i = 0; i < motorENCount; i++)
		portExtender.digitalWrite(motorENPins[i], LOW);
#else
	pinMode(pwm0Pin, OUTPUT); analogWrite(pwm0Pin, MOTOR_POWER_OFF);
	pinMode(pwm1Pin, OUTPUT); analogWrite(pwm1Pin, MOTOR_POWER_OFF);
#endif

	uint8_t signature = EEPROMUtils::read(memAddress + 1);
	activeValves = EEPROMUtils::read(memAddress);

#ifdef _DEBUG
	Serial.print(state); Serial.print(" "); Serial.println(activeValves, BIN);
	Serial.println(signature, HEX);
#endif

	if (signature != MOTORIZED_VALVE_SIGNATURE) {
		Serial.println(F("Wrong signature. Resetting state"));
		state = State::Open;
		activeValves = valvesMask;
		EEPROMUtils::save(memAddress, activeValves);
		signature = MOTORIZED_VALVE_SIGNATURE;
		EEPROMUtils::save(memAddress + 1, signature);
	}
#ifdef _DEBUG
	Serial.print(state); Serial.print(" "); Serial.println(activeValves, BIN);
#endif

	state = activeValves != 0 ? State::Open : State::Closed;
}


void WaterMotorizedValve::loop() {
	switch (state) {
	case State::Opening: case State::Closing: {
#ifdef _DEBUG
			Serial.print(F("Transit elapsed: ")); Serial.println(valveTransitChrono.elapsed());
#endif
			boolean stop = false;
	#ifdef BOARD_V2
			stop = valveTransitChrono.elapsed() > VALVE_TRANSIT_TIMEOUT_MILLIS_MIN;
	#else
			if (valveTransitChrono.elapsed() < VALVE_TRANSIT_TIMEOUT_MILLIS_MIN) {
				delay(100);
			}
			else stop = true;
	#endif
			if (stop) {
	#ifdef BOARD_V2
				for (int i = 0; i < motorENCount; i++)
					portExtender.digitalWrite(motorENPins[i], LOW);
	#endif
				analogWrite(pwm0Pin, MOTOR_POWER_OFF);
				analogWrite(pwm1Pin, MOTOR_POWER_OFF);

#ifdef _DEBUG
				Serial.println(F("Stop valveTransitChrono"));
#endif
				valveTransitChrono.stop();
				switch (state) {
				case State::Opening:
					setValvePosition(State::Open);
					break;
				case State::Closing:
					activeValves = 0;
					setValvePosition(State::Closed);
					break;
				}
#ifdef _DEBUG
				Serial.print(F("Save active valves: ")); Serial.println(activeValves, BIN);
#endif
				EEPROMUtils::save(memAddress, activeValves);
			} else delay(100);
	} break;
	case State::Open: case State::Closed:
		valveTransitChrono.stop();
		break;
	}
}

boolean WaterMotorizedValve::processValve(State position) {
#ifdef _DEBUG
	Serial.print(F("WaterMotorizedValve::processValve: ")); Serial.println(position);
#endif

	while (state == State::Opening || state == State::Closing) loop();

#ifdef BOARD_V2
	for (int i = 0; i < motorENCount; i++) {
		if (((i << i) & activeValves) != 0)
			portExtender.digitalWrite(motorENPins[i], HIGH);
		else
			portExtender.digitalWrite(motorENPins[i], LOW);
	}
#endif

	switch (position) {
	case State::Open:
		analogWrite(pwm0Pin, MOTOR_POWER_OFF);
		analogWrite(pwm1Pin, MOTOR_POWER_FULL);
/*#ifdef _DEBUG
		Serial.print(F("Pin:")); Serial.print(pwm0Pin); Serial.print(F("=")); Serial.println(MOTOR_POWER_FULL);
		Serial.print(F("Pin:")); Serial.print(pwm1Pin); Serial.print(F("=")); Serial.println(MOTOR_POWER_OFF);
#endif*/
		setValvePosition(State::Opening);
		break;
	case State::Closed:
		analogWrite(pwm0Pin, MOTOR_POWER_FULL);
		analogWrite(pwm1Pin, MOTOR_POWER_OFF);
/*#ifdef _DEBUG
		Serial.print(F("Pin:")); Serial.print(pwm0Pin); Serial.print(F("=")); Serial.println(MOTOR_POWER_OFF);
		Serial.print(F("Pin:")); Serial.print(pwm1Pin); Serial.print(F("=")); Serial.println(MOTOR_POWER_FULL);
#endif*/
		setValvePosition(State::Closing);
		break;
	}


#ifdef _DEBUG
	//Serial.println(F("Restart valveTransitChrono"));
#endif
	valveTransitChrono.restart();
	return true;
}

boolean WaterMotorizedValve::openValve(const uint8_t _valvesMask /*= 0xFF*/, boolean manual /*= false*/) {
#ifdef _DEBUG
	Serial.println(F("WaterMotorizedValve::openValve"));
#endif

	Valve::openValve(_valvesMask, manual);

	if (activeValves == _valvesMask) return false;
	activeValves = _valvesMask;
	EEPROMUtils::save(memAddress, activeValves);

	return processValve(State::Open);
}

boolean WaterMotorizedValve::closeValve() {
#ifdef _DEBUG
	Serial.println(F("WaterMotorizedValve::closeValve"));
#endif

	if (activeValves == 0) return false;
	return processValve(State::Closed);
}

boolean WaterMotorizedValve::isOpen() {
#ifdef _DEBUG
		//Serial.print(state); Serial.print(" "); Serial.println(activeValves, BIN);
#endif
		return state != State::Closed || activeValves != 0;
}

boolean WaterMotorizedValve::isClosed() {
#ifdef _DEBUG
		//Serial.print(state); Serial.print(" "); Serial.println(activeValves, BIN);
#endif
		return state == State::Closed && activeValves == 0;
}
