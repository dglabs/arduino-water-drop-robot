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

const int STORAGE_SIZE_COVER_HANDLER = sizeof(uint8_t) + sizeof(unsigned long);
const unsigned long TIME_TO_OPEN_COVER_SEC = 75;
const unsigned long MAX_TIME_TO_OPEN_COVER_SEC = 120;

class RainCoverHandler {
private:
	const uint8_t motorPowerPin;
	const uint8_t motorDirectionPin;
	const uint8_t tiltSensorPin;
	const int memAddress;

	boolean isOpenedManually;
	uint8_t currentState;
	unsigned long timeToOpenCover;
	Chrono operationChrono;

public:
	RainCoverHandler(const uint8_t _motorPowerPin, const uint8_t _motorDirectionPin, const uint8_t _tiltSensorPin, int _memAddress);
	virtual ~RainCoverHandler();

	boolean isCoverOpen();
	void openCover(boolean manual = false);
	void closeCover();

	boolean isManualOpen() const { return isOpenedManually; }
};

#endif /* RAINCOVERHANDLER_H_ */
