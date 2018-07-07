/*
 * RobotController.h
 *
 *  Created on: 10 июн. 2017 г.
 *      Author: dennis
 */

#ifndef ROBOTCONTROLLER_H_
#define ROBOTCONTROLLER_H_

#include <Arduino.h>
#include "WaterDropRobot.h"
#include "AbstractController.h"

const long ACTIVE_STATE_TIME_SECONDS = 60;
const int POWER_SAVE_SYCLES = 3;

class RobotController: public AbstractController {
public:
	enum RobotState { Active, PowerSave };

protected:
	Chrono activeStateChrono;
	int powerSaveCyclesCount;
	int temperature;
	RobotState currentState;

	void setCurrentState(RobotState _state);
	virtual boolean checkRainOut();
	virtual boolean processScheduleEvent();
	virtual void prepareWinterOperation();

public:
	RobotController(const uint8_t _mainPowerPin, uint8_t _wifi3VPowerPin);

	virtual void setup();

	virtual void loop();

	boolean checkSchedule();
};

#endif /* ROBOTCONTROLLER_H_ */
