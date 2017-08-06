/*
 * RobotController.cpp
 *
 *  Created on: 10 июн. 2017 г.
 *      Author: dennis
 */

#include <LowPower.h>
#include "RobotController.h"

#define _SERIAL_DEBUG

const DateTime waterOutTime(2017, 06, 10, 21, 0, 0);
const DateTime waterInTime(2017, 06, 10, 10, 0, 0);

RobotController::RobotController(const uint8_t _mainPowerPin
		, RTC_DS1307& _rtc
		, KeyboardWithISR& _keyboard
		, WaterLevelMeter& _waterLevelMeter
		, WaterMotorizedValve& _waterOutValve
		, WaterInValve& _waterInValve
		, RobotDisplay& _display
		, WaterFlowMeter& _waterFlowMeter
		, RainSensor& _rainSensor
		, RainCoverHandler& _rainCoverHandler) :

		mainPowerPin(_mainPowerPin)
		, rtc(_rtc)
		, keyboard(_keyboard)
		, waterLevelMeter(_waterLevelMeter)
		, waterOutValve(_waterOutValve)
		, waterInValve(_waterInValve)
		, display(_display)
		, waterFlowMeter(_waterFlowMeter)
		, rainSensor(_rainSensor)
		, rainCoverHandler(_rainCoverHandler)

		, currentState(RobotState::Active)
		, activeStateChrono(Chrono::SECONDS)
{
	pinMode(mainPowerPin, OUTPUT); digitalWrite(mainPowerPin, HIGH);	// Turn on all peripherals for the first time
	powerSaveCyclesCount = 0;
}

RobotController::~RobotController() {
	digitalWrite(mainPowerPin, LOW);	// Turn off all peripherals
}

void RobotController::setCurrentState(RobotState _state) {
	if (currentState == _state) return;
	currentState = _state;
	switch (currentState) {
	case PowerSave: {
		powerSaveCyclesCount = 0;
		activeStateChrono.restart();
		activeStateChrono.stop();
		display.powerDown();
		digitalWrite(mainPowerPin, LOW);	// Turn off all peripherals
	} break;
	case Active: {
		powerSaveCyclesCount = 0;
		activeStateChrono.restart();
		digitalWrite(mainPowerPin, HIGH);	// Turn off all peripherals
		display.initialize();
	} break;
	}
}

boolean RobotController::checkSchedule() {
	DateTime waterIn(now.year(), now.month(), now.day(), waterInTime.hour(), waterInTime.minute(), waterInTime.second());
	DateTime waterOut(now.year(), now.month(), now.day(), waterOutTime.hour(), waterOutTime.minute(), waterOutTime.second());

	TimeSpan inSpan = now - waterIn;
	TimeSpan outSpan = now - waterOut;

	if (inSpan.totalseconds() > 0 && inSpan.totalseconds() < MAX_IN_VALVE_OPEN_TIME_SECONDS / 2) {
		startWaterIn();
		return true;
	}

	if (outSpan.totalseconds() > 0 && outSpan.totalseconds() < MAX_OUT_VALVE_OPEN_TIME_SECONDS / 2) {
		startWaterOut();
		return true;
	}


	return false;
}

void RobotController::loop() {
	switch (currentState) {
	case PowerSave: {
	    if (keyboard.isPressed(KeyboardWithISR::KEY0)) {	// Wake up from power down
	    	setCurrentState(RobotState::Active);
	    }
	    else {
	    	if (powerSaveCyclesCount++ > POWER_SAVE_SYCLES) {
	    		digitalWrite(mainPowerPin, HIGH);	// Turn off all peripherals
	    		delay(10);
	    		now = rtc.now();
	    		waterLevelMeter.readLevel();
	    		if (waterOutValve.isOpen()) waterOutValve.closeValve();
	    		if (checkSchedule()) { setCurrentState(Active); break; }
	    		if (checkRainOut()) { setCurrentState(Active); display.setState(RobotDisplay::RainControl); break; }
	    		digitalWrite(mainPowerPin, LOW);	// Turn off all peripherals
	    		powerSaveCyclesCount = 0;
	    	}
	    	if (currentState == PowerSave)
	    		LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);
	    }
	} break;
	case Active: {
		now = rtc.now();

		boolean anyActivity = false;
	    if (keyboard.isPressed(KeyboardWithISR::KEY0)) {	// Wake up from power down
	    	anyActivity = true;
	    	if (!display.isPowerSaveMode()) // Skip display state change if display is on power save mode
	    		display.switchNextState();
	    	keyboard.clear();
	    }
	    if (keyboard.isPressed(KeyboardWithISR::KEY1)) {
	    	anyActivity = true;
	    	switch (display.getState()) {
	    	case RobotDisplay::OutValve: {
	    		if (waterOutValve.isOpen() && waterOutValve.valveOpenSeconds() > 5) {
	    			waterOutValve.closeValve();
	    		}
	    		else {
	    			if (waterOutValve.valveCloseSeconds() > MIN_TIME_VAVLE_CLOSED_SECONDS)
	    				waterOutValve.openValve();
   				}
	    	} break;
	    	case RobotDisplay::InValve: {
	    		if (waterInValve.isOpen() && waterInValve.valveOpenSeconds() > 5) {
	    			waterInValve.closeValve();
	    		}
	    		else {
	    			if (waterInValve.valveCloseSeconds() > MIN_TIME_VAVLE_CLOSED_SECONDS)
	    				waterInValve.openValve();
	    		}
	    	} break;
	    	case RobotDisplay::WaterLevel: {
	    		waterLevelMeter.forceCurrentValuesAsAverages();
	    	} break;
	    	case RobotDisplay::RainControl: {
	    		if (rainCoverHandler.isCoverOpen()) rainCoverHandler.closeCover(); else rainCoverHandler.openCover(true);
			    display.update(now);
	    	} break;
	    	}
	    	keyboard.clear();
	    }

	    if (waterInValve.isOpen()) {
	    	anyActivity = true;
	    	if (!checkWaterLevel() || waterInValve.valveOpenSeconds() > MAX_IN_VALVE_OPEN_TIME_SECONDS) {
	    		waterInValve.closeValve();
	    		waterFlowMeter.stopWaterOut();
	    	}
	    }

	    if (waterOutValve.isOpen()) {
	    	anyActivity = true;
	    	if (!checkWaterLevel() || waterOutValve.valveOpenSeconds() > MAX_OUT_VALVE_OPEN_TIME_SECONDS) {
	    		waterOutValve.closeValve();
	    		waterOutValve.closeValve();
	    	}
	    }

	    if (anyActivity) {
	    	activeStateChrono.restart();
    		display.turnOnBacklight();
	    }

	    boolean _anyActivity = checkRainOut();
	    anyActivity |= _anyActivity;

	    display.update(now);

	    if (activeStateChrono.hasPassed(ACTIVE_STATE_TIME_SECONDS))
	    	setCurrentState(PowerSave);
	    else {
	    	for (int c = 0; c < 10; c++) {
	    		keyboard.refresh();
	    		//LowPower.powerDown(SLEEP_60MS, ADC_OFF, BOD_OFF);
	    		delay(50);
	    	}
	    }
	} break;
	}

}

boolean RobotController::checkRainOut() {
	boolean result = false;

    if (rainSensor.getIntensity() > RainIntensity::mist && !rainCoverHandler.isCoverOpen() && waterLevelMeter.readLevel() < 80) {
    	result = true;
    	display.setState(RobotDisplay::RainControl);
    	display.update(now);
    	rainCoverHandler.openCover();
    }

    if ((rainCoverHandler.isCoverOpen() && !rainCoverHandler.isManualOpen()) &&
    		(waterLevelMeter.readLevel() >= 80 || rainSensor.getIntensity() <= RainIntensity::mist)) {
    	result = true;
    	display.setState(RobotDisplay::RainControl);
    	display.update(now);
    	rainCoverHandler.closeCover();
    }

    return result;
}


void RobotController::startWaterOut() {
	if (waterLevelMeter.readLevel() > 0 && waterOutValve.valveCloseSeconds() > MIN_TIME_VAVLE_CLOSED_SECONDS) {
		waterInValve.closeValve();
		waterOutValve.openValve();
		display.setState(RobotDisplay::OutValve);
		waterFlowMeter.startWaterOut();
	}
}

void RobotController::startWaterIn() {
	if (waterLevelMeter.readLevel() <= 30 && waterInValve.valveCloseSeconds() > MIN_TIME_VAVLE_CLOSED_SECONDS) {
		waterOutValve.closeValve();
		waterInValve.openValve();
		display.setState(RobotDisplay::InValve);
	}
}

boolean RobotController::checkWaterLevel() {
	if (waterInValve.isOpen()) {
		if (waterLevelMeter.readLevel() > 70) return false;
	}
	if (waterOutValve.isOpen()) {
		if (waterLevelMeter.readLevel() <= 0) return false;
	}
	return true;
}



