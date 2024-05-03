#ifndef MAIN_H
#define MAIN_H

/////////// Includes ///////////
#include "MKL25Z4.h"
#include "stdio.h"
#include "string.h"
#include "i2cdisplay.h"

//////////// MACROS ////////////
#define TIME_TO_BLINK_MS 1000
#define TIME_TO_PRINT_MS 2000
#define UART_TIMEOUT_MS  300

// Enum for RGB bits
enum RGB_LED {
	RED = 0x40000,
	GREEN = 0x80000,
	BLUE = 0x02
};

#endif /*MAIN_H*/
