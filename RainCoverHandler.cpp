/*
 * RainCoverHandler.cpp
 *
 *  Created on: 6 авг. 2017 г.
 *      Author: dennis
 */

#include "RainCoverHandler.h"
#include <EEPROM.h>

const uint8_t MASK_OPEN = 0xAA;
const uint8_t MASK_CLOSED = 0x55;

const long OPEN_CLOSE_ROTATIONS = 10;
const boolean DIRECTION_OPEN = false;
const boolean DIRECTION_CLOSE = true;
//const int steps90Degrees = 2100;

RainCoverHandler::RainCoverHandler(const uint8_t* _motorPins, int _memAddress) :
	motorPins(_motorPins)
	, memAddress(_memAddress)
	, currentState(MASK_CLOSED)
	, stepper(_motorPins[0], _motorPins[1], _motorPins[2], _motorPins[3])
	//, stepper(AccelStepper::MotorInterfaceType::HALF4WIRE, motorPins[0], _motorPins[1], _motorPins[2], _motorPins[3])
	, isOpenedManually(false)
{
	currentState = EEPROM.read(memAddress);
	switch (currentState) {
	case MASK_OPEN: case MASK_CLOSED: break;
	default:
		currentState = MASK_CLOSED;
		EEPROM.write(memAddress, currentState);
		break;
	}

	// let's set a custom speed of 20rpm (the default is ~16.25rpm)
	stepper.setRpm(10);
	stepper.setTotalSteps(4096);
	stepper.moveCCW(1);
	stepper.stop();

	/*stepper.setMaxSpeed(500.0);
	stepper.setAcceleration(100.0);
	stepper.setSpeed(100);
	stepper.move(-1);*/
}

RainCoverHandler::~RainCoverHandler() {
	// TODO Auto-generated destructor stub
}

boolean RainCoverHandler::isCoverOpen() const { return currentState == MASK_OPEN; }

void RainCoverHandler::openCover(boolean manual) {
	isOpenedManually = manual;
	for (int i = 0; i < OPEN_CLOSE_ROTATIONS; i++) {
		stepper.moveDegrees (DIRECTION_OPEN, 180);
	}
	stepper.stop();
	/*stepper.move(steps90Degrees * OPEN_CLOSE_ROTATIONS);
	while (stepper.run());*/

	currentState = MASK_OPEN;
	EEPROM.write(memAddress, currentState);
}

void RainCoverHandler::closeCover() {
	for (int i = 0; i < OPEN_CLOSE_ROTATIONS; i++) {
		stepper.moveDegrees (DIRECTION_CLOSE, 180);
	}
	stepper.stop();
	/*stepper.move(steps90Degrees * OPEN_CLOSE_ROTATIONS);
	while (stepper.run());*/

	currentState = MASK_CLOSED;
	EEPROM.write(memAddress, currentState);
}

