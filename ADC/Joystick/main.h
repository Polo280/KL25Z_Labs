#ifndef MAIN_H
#define MAIN_H

/////////// Includes ///////////
#include "MKL25Z4.h"
#include "stdio.h"
#include "string.h"
//#include "i2cdisplay.h"
#include "Headers/Rgb.h"

//////////// MACROS ////////////
#define TIME_TO_PRINT_MS 1000
#define TIME_TO_READ_ADC_MS 1000
#define UART_TIMEOUT_MS  1000
#define ADC_TIMEOUT_MS  500

////////// Functions  //////////
void delayMs(uint64_t);
void delayUs(uint64_t);

#endif /*MAIN_H*/
