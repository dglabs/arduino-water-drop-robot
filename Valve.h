#ifndef VALVE_H_
#define VALVE_H_

#include <Arduino.h>
#include <Chrono.h>

const boolean VALVE_OPEN = true;
const boolean VALVE_CLOSED = false;

const long MIN_TIME_VAVLE_CLOSED_SECONDS = 10;

class Valve {
public:
	enum Position {VALVE_OPEN, VALVE_CLOSED};

protected:
	Chrono openValveChrono;
	Chrono closeValveChrono;

	boolean isSamePosition(Position position) {
		switch (position) {
		case VALVE_OPEN: {
			if (isOpen()) return true;
		} break;
		case VALVE_CLOSED: {
			if (!isOpen()) return true;
		} break;
		}
		return false;
	}
public:
	Valve() : openValveChrono(Chrono::SECONDS), closeValveChrono(Chrono::SECONDS) { closeValveChrono.restart();  };
	virtual ~Valve() {};

	virtual boolean setValvePosition(Position position) {
		switch (position) {
		case VALVE_OPEN: {
			openValveChrono.restart();
			closeValveChrono.restart();
			closeValveChrono.stop();
		} break;
		case VALVE_CLOSED: {
			closeValveChrono.restart();
			openValveChrono.restart();
			openValveChrono.stop();
		} break;
		}
		return true;
	}
	boolean openValve() { return setValvePosition(Position::VALVE_OPEN); }
	boolean closeValve() { return setValvePosition(Position::VALVE_CLOSED); }
	virtual boolean isOpen() = 0;

	int valveOpenSeconds() const { return openValveChrono.elapsed(); }
	int valveCloseSeconds() const { return closeValveChrono.elapsed(); }
};

#endif /* VALVE_H_ */
