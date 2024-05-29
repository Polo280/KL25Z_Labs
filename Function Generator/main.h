#ifndef MAIN_H
#define MAIN_H

/////////// Includes ///////////
#include "MKL25Z4.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "Headers/Rgb.h"
#include "Headers/Clocks.h"
#include "Headers/Stepper.h"
//#include "Headers/UART.h"

//////////// MACROS ////////////
#define TIME_TO_RUN_STEPPER 0
#define TIME_TO_PRINT_MS    1000
#define TIME_TO_READ_ADC_MS 50
#define UART_TIMEOUT_MS  1000
#define UART_RX_TIMEOUT  50
#define ADC_TIMEOUT_MS  500

enum WaveModes{
	SQUARE = 0,
	SINE   = 1,
	TRIANGULAR = 2
};

////////// Functions  //////////

#endif /*MAIN_H*/
