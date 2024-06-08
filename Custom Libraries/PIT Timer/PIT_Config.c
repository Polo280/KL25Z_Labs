/*
 * This module is used to set a common time base for different events that require precise timing
 * using PIT module, which generates an interrupt every time the timer reaches a certain value
 * */

#include "Headers/PIT_Config.h"

uint64_t pit_delay;
uint8_t pit_timebase;

// Configure a PIT interrupt to run at a certain interval
void PIT_InitUs(void){
	// Enable clock for PIT
	SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;
	// Enable PIT module and freeze timers in debug mode
	PIT->MCR = 0x00;
	// Set PIT0 reload value to generate an interrupt every 1 us
	PIT->CHANNEL[0].LDVAL = (SystemCoreClock / 2000000) - 1;  // The bus clock runs by default half the core clock freq (48Mhz/2= 24 Mhz)
	PIT->CHANNEL[0].TCTRL = PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK;
	// Update PIT timebase value
	pit_timebase = PIT_MICROSECONDS;
	// Enable PIT interrupt in NVIC
	NVIC_EnableIRQ(PIT_IRQn);
}

void PIT_InitMs(void){
	// Enable clock for PIT
	SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;
	// Enable PIT module and freeze timers in debug mode
	PIT->MCR = 0x00;
	// Set PIT0 reload value to generate an interrupt every 1 us
	PIT->CHANNEL[0].LDVAL = (SystemCoreClock / 2000) - 1;
	PIT->CHANNEL[0].TCTRL = PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK;
	// Update PIT timebase value
	pit_timebase = PIT_MILLIS;
	// Enable PIT interrupt in NVIC
	NVIC_EnableIRQ(PIT_IRQn);
}

void PIT_IRQHandler(void){
	// Clear interrupt flag
	PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;
	// Manage delay library
	pit_delay = (pit_delay > 0)? pit_delay-1 : 0;
}

void PIT_DelayMicroseconds(uint64_t delay){
	pit_delay  = (pit_timebase == PIT_MICROSECONDS)? delay : 1; // If timebase is millis spend only one millisecond in cycle
	while(pit_delay != 0){}
}

void PIT_DelayMilliseconds(uint64_t delay){
	pit_delay  = (pit_timebase == PIT_MICROSECONDS)? delay*1000 : delay; // If timebase is millis spend only one millisecond in cycle
	while(pit_delay != 0){}
}
