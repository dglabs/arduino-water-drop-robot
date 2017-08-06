/*
 * RainCoverHandler.h
 *
 *  Created on: 6 авг. 2017 г.
 *      Author: dennis
 */

#ifndef RAINCOVERHANDLER_H_
#define RAINCOVERHANDLER_H_


#include <Arduino.h>
#include <CheapStepper.h>
//#include <AccelStepper.h>

const int STORAGE_SIZE_COVER_HANDLER = sizeof(uint8_t);

class RainCoverHandler {
private:
	const uint8_t* motorPins;
	const int memAddress;

	boolean isOpenedManually;
	uint8_t currentState;
	CheapStepper stepper;
	//AccelStepper stepper;

public:
	RainCoverHandler(const uint8_t* _motorPins, int _memAddress);
	virtual ~RainCoverHandler();

	boolean isCoverOpen() const;
	void openCover(boolean manual = false);
	void closeCover();

	boolean isManualOpen() const { return isOpenedManually; }
};

#endif /* RAINCOVERHANDLER_H_ */
