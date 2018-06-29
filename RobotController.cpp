/*
 * RobotController.cpp
 *
 *  Created on: 10 июн. 2017 г.
 *      Author: dennis
 */

#include <LowPower.h>
#include "RobotController.h"

#define _SERIAL_DEBUG

//const DateTime waterOutTime(2017, 06, 10, 21, 0, 0);
//const DateTime waterInTime(2017, 06, 10, 10, 0, 0);

RobotController::RobotController(const uint8_t _mainPowerPin
		, RTC_DS3231& _rtc
		, KeyboardWithISR& _keyboard
		, WaterLevelMeter& _waterLevelMeter
		, WaterMotorizedValve& _waterOutValve
		, WaterInValve& _waterInValve
		, RobotDisplay& _display
		, WaterFlowMeter& _waterFlowMeter
		, RainSensor& _rainSensor
		, RainCoverHandler& _rainCoverHandler
		, WaterSchedule& _schedule
		, BatteryMonitor& _batteryMonitor
		, DS3232RTC& _rtcDS3232) :

		AbstractController( _mainPowerPin
			, _rtc
			, _keyboard
			, _waterLevelMeter
			, _waterOutValve
			, _waterInValve
			, _display
			, _waterFlowMeter
			, _rainSensor
			, _rainCoverHandler
			, _schedule
			, _batteryMonitor
			, _rtcDS3232)

		, currentState(RobotState::Active)
		, activeStateChrono(Chrono::SECONDS)
		, powerSaveCyclesCount(0)
		, temperature(20)
{
}

RobotController::~RobotController() {
}

void RobotController::setCurrentState(RobotState _state) {
	if (currentState == _state) return;
	currentState = _state;
	switch (currentState) {
	case PowerSave: {
		powerSaveCyclesCount = 0;
		if (waterInValve.isOpen()) waterInValve.closeValve();
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

// Process the current schedule action if any and @return true if device should stay active
boolean RobotController::processScheduleEvent() {
	LOOP:
	if (!schedule.isEventAppropriate(schedule.getCurrentEvent(), temperature)) {
		schedule.dismissCurrentEvent(); return false;
	}
	switch (schedule.getCurrentEvent().type) {
	case EventType::WaterIn: {
		if (waterInValve.isClosed()) {
			if (waterLevelMeter.readLevel() <= schedule.getCurrentEvent().minLevel &&
					waterInValve.valveCloseSeconds() > MIN_TIME_VAVLE_CLOSED_SECONDS) {
				waterInValve.openValve();
				display.setState(RobotDisplay::InValve);
				return true;
			}
			else { display.print("HIGH WATER LEVEL", 1); schedule.dismissCurrentEvent(); delay(1000);  }
		}
		else if (waterInValve.isOpen()) {
	    	if (waterLevelMeter.readLevel() >= schedule.getCurrentEvent().maxLevel ||
	    			waterInValve.valveOpenSeconds() > schedule.getCurrentEvent().duration) {
	    		waterInValve.closeValve();
	    		schedule.dismissCurrentEvent();
				display.setState(RobotDisplay::Dashboard);
	    		return false;
	    	}
	    	return true;
		}
		else return false;
	} break;
	case EventType::WaterOut: {
		if (waterOutValve.isClosed()) {
			if (waterLevelMeter.readLevel() >= schedule.getCurrentEvent().minLevel) {
				waterFlowMeter.startWaterOut();
				waterOutValve.openValve();
				display.setState(RobotDisplay::OutValve);
				return true;
			}
			else { display.print("LOW WATER LEVEL", 1); schedule.dismissCurrentEvent(); delay(1000); return false; }
		}
		else if (waterOutValve.isOpen()) {
			boolean startWaterInAfter = false;
			if (waterLevelMeter.readLevel() <= schedule.getCurrentEvent().minLevel ||
					waterOutValve.valveOpenSeconds() > schedule.getCurrentEvent().duration ||
					waterFlowMeter.getVolumeFromStart() > schedule.getCurrentEvent().liters ||
					(startWaterInAfter = (waterOutValve.valveOpenSeconds() > 100 && waterFlowMeter.getVolumeFromStart() == 0))) {
				waterOutValve.closeValve();
	    		waterFlowMeter.stopWaterOut();
	    		schedule.dismissCurrentEvent();
				display.setState(RobotDisplay::Dashboard);

	    		// Start water in if no water poring out
	    		if (startWaterInAfter && !batteryMonitor.isPowerLow()) {
	    			schedule.setCurrentEvent(ScheduleEvent(NO_ID, EventType::WaterIn, now, 250 /*duration*/, 300 /*liters*/, 5 /*minTemperature*/
	    					, 51 /*minLevel*/, 100 /*maxLevel*/, EventFlags::Active));
	    			goto LOOP;
	    		}
	    		else return false;
			}
			else return true;	// Continue water pouring
		}
		else return false;
	} break;
	}

	// Backup branch to close In valve
	if (waterInValve.isOpen() && waterLevelMeter.readLevel() >= 100) {
		waterInValve.closeValve();
		schedule.dismissCurrentEvent();
	}

	return false;
}

boolean RobotController::checkSchedule() {
	boolean result = schedule.scanEvents(temperature);
	if (result) result = processScheduleEvent();
	return result;
}

// Prepare system to winter operation
void RobotController::prepareWinterOperation() {
	if (!waterOutValve.isOpen())
		waterOutValve.openValve();
	waterInValve.closeValve();
	if (rainCoverHandler.isOpen())
		rainCoverHandler.closeValve();
}

void RobotController::setup() {
	AbstractController::setup();

	temperature = rtcDS3232.temperature() / 4;
	if (schedule.isInActiveDateRange(temperature)) {
		waterOutValve.closeValve();
		waterFlowMeter.startWaterOut();
	}
	else {
		prepareWinterOperation();
	}
}

void RobotController::loop() {
LOOP:
	AbstractController::loop();
	switch (currentState) {
	case PowerSave: {
	    if (keyboard.isRotated()) {	// Wake up from power down
	    	setCurrentState(RobotState::Active);
	    }
	    else {
	    	if (powerSaveCyclesCount++ > POWER_SAVE_SYCLES) {
	    		digitalWrite(mainPowerPin, HIGH);	// Turn on all peripherals
    			display.turnOffBacklight();
	    		delay(5);
	    		now = rtc.now();
	    		temperature = rtcDS3232.temperature() / 4;
	    		Serial.println(temperature);
	    		waterLevelMeter.readLevel();

	    		if (schedule.isInActiveDateRange(temperature)) {
					if (!batteryMonitor.isPowerLow()) {
						if (checkSchedule()) { setCurrentState(Active); goto LOOP; }
						if (checkRainOut()) { setCurrentState(Active); display.setState(RobotDisplay::RainControl); goto LOOP; }
					}
	    		}
	    		else prepareWinterOperation();

	    		if (isBackgroundActivity()) {
	    			setCurrentState(Active);
	    			goto LOOP;
	    		}

	    		if (schedule.getCurrentEvent().type != EventType::None) processScheduleEvent();

	    		if (now.hour() == 0 && now.minute() == 0) {
	    			waterFlowMeter.adjustStatictcs();
	    		}

	    		if (batteryMonitor.isPowerLow()) {
	    			display.turnOffBacklight();
	    			display.print("LOW POWER!!!  ");
		    		delay(2000);
	    		}

	    		digitalWrite(mainPowerPin, LOW);	// Turn off all peripherals
	    		powerSaveCyclesCount = 0;
	    	}

	    	if (currentState == PowerSave)
	    		LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
	    }
	} break;
	case Active: {
		now = rtc.now();
		keyboard.tick();

		boolean anyActivity = false;
	    if (keyboard.isRotated()) {	// Wake up from power down
	    	anyActivity = true;
	    	if (!display.isPowerSaveMode()) // Skip display state change if display is on power save mode
	    	{
	    		display.setState((RobotDisplay::State)keyboard.getPos());
	    	}
	    }
	    else if (keyboard.isPressed()) {
	    	anyActivity = false;
	    	switch (display.getState()) {
	    	case RobotDisplay::OutValve: {
	    		if (waterOutValve.isOpen() /*&& waterOutValve.valveOpenSeconds() > 5*/) {
	    			waterOutValve.closeValve();
	    			waterFlowMeter.stopWaterOut();
	    			schedule.dismissCurrentEvent();
	    		}
	    		else {
	    			schedule.setCurrentEvent(ScheduleEvent(NO_ID, EventType::WaterOut, now, MAX_OUT_VALVE_OPEN_TIME_SECONDS /*duration*/, 60 /*liters*/, 5 /*minTemperature*/
	    					, 10 /*minLevel*/, 100 /*maxLevel*/, EventFlags::Active));
   				}
	    	} break;
	    	case RobotDisplay::InValve: {
	    		if (waterInValve.isOpen() && waterInValve.valveOpenSeconds() > 5) {
	    			waterInValve.closeValve();
	    			schedule.dismissCurrentEvent();
	    		}
	    		else {
	    			schedule.setCurrentEvent(ScheduleEvent(NO_ID, EventType::WaterIn, now, MAX_IN_VALVE_OPEN_TIME_SECONDS /*duration*/, 300 /*liters*/, 5 /*minTemperature*/
	    					, 51 /*minLevel*/, 100 /*maxLevel*/, EventFlags::Active));
	    		}
	    	} break;
	    	case RobotDisplay::RainControl: {
	    		if (rainCoverHandler.isOpen()) rainCoverHandler.closeValve(); else rainCoverHandler.openValve(VALVE_RAIN, true);
			    display.update(now);
	    	} break;
	    	default: {
	    		anyActivity = waterOutValve.isOpen() || waterInValve.isOpen();
	    		if (!anyActivity) {
	    			setCurrentState(PowerSave);
	    			goto LOOP;
	    		}
	    	} break;
	    	}
	    }

	    boolean _anyActivity = processScheduleEvent();
	    anyActivity |= _anyActivity;

	    _anyActivity = checkRainOut();
	    anyActivity |= _anyActivity;

	    _anyActivity = isBackgroundActivity();
	    anyActivity |= _anyActivity;

	    if (anyActivity) {
	    	activeStateChrono.restart();
    		display.turnOnBacklight();
	    }

	    delay(10);

	    display.update(now);

	    if (!anyActivity && activeStateChrono.hasPassed(ACTIVE_STATE_TIME_SECONDS))
	    	setCurrentState(PowerSave);
	} break;
	}

}

boolean RobotController::checkRainOut() {
	boolean result = false;

	if (rainSensor.getIntensity() > RainIntensity::mist && rainCoverHandler.isClosed() && waterLevelMeter.readLevel() < 80 &&
			rainSensor.secondsFromRainStarted() > MIN_RAIN_TIME_TO_OPEN_COVER && schedule.isInActiveDateRange(temperature)) {
		result = true;
		display.setState(RobotDisplay::RainControl);
		display.update(now);
		rainCoverHandler.openValve(VALVE_RAIN, false);
	}

	if (((rainCoverHandler.isOpen() && !rainCoverHandler.isManualOpen()) && rainCoverHandler.valveOpenSeconds() > MIN_RAIN_TIME_TO_OPEN_COVER &&
			(waterLevelMeter.readLevel() >= 100 || rainSensor.getIntensity() <= RainIntensity::mist)) ||
			!schedule.isInActiveDateRange(temperature)) {
		result = true;
		display.setState(RobotDisplay::RainControl);
		display.update(now);
		rainCoverHandler.closeValve();
	}

    return result;
}





