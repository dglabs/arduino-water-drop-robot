/*
 * RainCoverHandler.h
 *
 *  Created on: 6 авг. 2017 г.
 *      Author: dennis
 */

#ifndef RAINCOVERHANDLER_H_
#define RAINCOVERHANDLER_H_


#include <Arduino.h>
#include <Chrono.h>

#include "CommonDefs.h"
#include "Valve.h"

const int STORAGE_SIZE_COVER_HANDLER = sizeof(uint8_t) + sizeof(unsigned long);
const unsigned long TIME_TO_OPEN_COVER_SEC = 75;
const unsigned long TIME_TO_OPEN_COVER_SEC_MIN = 30;
const unsigned long MAX_TIME_TO_OPEN_COVER_SEC = 120;

class RainCoverHandler: public virtual Valve {
private:
	const uint8_t pwm0Pin;
	const uint8_t pwm1Pin;
#ifdef BOARD_V2
	const uint8_t motorENPin;
#endif
	const uint8_t tiltSensorPin;
	const int memAddress;

	unsigned long timeToOpenCover;
	Chrono operationChrono;

public:
	RainCoverHandler(const uint8_t _valveMask
		, const uint8_t _pwm0Pin
		, const uint8_t _pwm1Pin
#ifdef BOARD_V2
		, const uint8_t _motorENPin
#endif
		, const uint8_t _tiltSensorPin
		, int _memAddress);

	virtual void setup();
	virtual void loop();

	virtual boolean openValve(const uint8_t _valvesMask = 0xFF, boolean manual = false);
	virtual boolean closeValve();

	virtual boolean isOpen();
	virtual boolean isClosed();

	virtual boolean isActive() { return state == State::Opening || state == State::Closing; }
};

extern RainCoverHandler rainCoverHandler;

#endif /* RAINCOVERHANDLER_H_ */
