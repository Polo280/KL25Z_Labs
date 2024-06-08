#ifndef PIT_CONFIG_H_
#define PIT_CONFIG_H_

/////////// Includes ///////////
#include "MKL25Z4.h"

//////////// MACROS ////////////

/////////// Variables //////////
extern uint64_t pit_delay;
extern uint8_t pit_timebase;

enum TimeBase{
	NONE = 0,
	PIT_MICROSECONDS = 1,
	PIT_MILLIS = 2
};

////////// Functions  //////////
void PIT_InitUs(void);
void PIT_InitMs(void);
void PIT_DelayMicroseconds(uint64_t);
void PIT_DelayMilliseconds(uint64_t);
#endif /* PIT_CONFIG_H_ */
