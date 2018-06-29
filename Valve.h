#ifndef VALVE_H_
#define VALVE_H_

#include <Arduino.h>
#include <Chrono.h>

const boolean VALVE_OPEN = true;
const boolean VALVE_CLOSED = false;

const long MIN_TIME_VAVLE_CLOSED_SECONDS = 10;

class Valve {
public:
	enum State { Open, Opening, Closing, Closed };

protected:
	Chrono openValveChrono;
	Chrono closeValveChrono;

	const uint8_t valvesMask;
	uint8_t activeValves;
	State state;
	boolean isOpenManually;

	boolean isSamePosition(State position) {
		switch (position) {
		case Open: case Opening: {
			if (isOpen()) return true;
		} break;
		case Closed: case Closing: {
			if (isClosed()) return true;
		} break;
		}
		return false;
	}
public:
	Valve(const uint8_t _valvesMask) :
		openValveChrono(Chrono::SECONDS)
		, closeValveChrono(Chrono::SECONDS)
		, valvesMask(_valvesMask)
		, activeValves(_valvesMask)
		, state(State::Open)
		, isOpenManually(false)
	{ closeValveChrono.restart();  };
	virtual ~Valve() {};

	virtual void setup() {};
	virtual void loop() {};

	boolean setValvePosition(State position) {
		state = position;
		switch (position) {
		case Open: {
			openValveChrono.restart();
			closeValveChrono.restart();
			closeValveChrono.stop();
		} break;
		case Closed: {
			closeValveChrono.restart();
			openValveChrono.restart();
			openValveChrono.stop();
		} break;
		}
		return true;
	}

	virtual boolean openValve(const uint8_t /*_valvesMask = 0xFF*/, boolean manual = false) {
		isOpenManually = manual;
	}

	virtual boolean closeValve() = 0;
	virtual boolean isOpen() = 0;
	virtual boolean isClosed() = 0;

	virtual State getState() { return state; }
	virtual boolean isActive() { return isOpen() || (state == State::Opening || state == State::Closing); }

	const char* getStateString() {
		switch (getState()) {
		case State::Open: return "OPEN";
		case State::Opening: return "OPENING";
		case State::Closing: return "CLOSING";
		case State::Closed: return "CLOSED";
		default: return "UNDEFINED";
		}
	}

	int valveOpenSeconds() const { return openValveChrono.elapsed(); }
	int valveCloseSeconds() const { return closeValveChrono.elapsed(); }

	boolean isManualOpen() const { return isOpenManually; }

};

#endif /* VALVE_H_ */
