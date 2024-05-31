#ifndef LCD4_H
#define LCD4_H

/////////// Includes ///////////
#include "MKL25Z4.h"
#include "main.h"

//////////// MACROS ////////////
// This should vary depending on the LCD4 connections
#define RS 0x04 /* PTA2 mask */
#define RW 0x10 /* PTA4 mask */
#define EN 0x20 /* PTA5 mask */

/* LCD COMMANDS
	- 0x38 -> set 8-bit data, 2-line, 5x7 font
	- 0x06 -> move cursor right
	- 0x01 -> clear screen, move cursor to 0,0
	- 0x0F -> turn on display, cursor blinking
*/

#define LCD4_CONFIG_COMMAND   	    0x38
#define LCD4_RIGHT_CURSOR_COMMAND   0x06
#define LCD4_CLEAR_COMMAND 		    0x01
#define LCD4_BLINK_HOME				0x0F
#define LCD4_NO_BLINK				(0x08) & ~ 1

/////////// Variables //////////
typedef struct{
	// LCD4 data pins	// LCD4 PORTS
	uint8_t d0_pin;		PORT_Type *d0_port;
	uint8_t d1_pin;		PORT_Type *d1_port;
	uint8_t d2_pin;     PORT_Type *d2_port;
	uint8_t d3_pin;		PORT_Type *d3_port;
	uint8_t d4_pin;		PORT_Type *d4_port;
	uint8_t d5_pin;		PORT_Type *d5_port;
	uint8_t d6_pin;		PORT_Type *d6_port;
	uint8_t d7_pin;		PORT_Type *d7_port;
	// Mode pins
	uint8_t enable_pin;	PORT_Type *enable_port;
	uint8_t rs_pin;		PORT_Type *rs_port;
	uint8_t rw_pin;		PORT_Type *rw_port;
} LCD4_Struct;

// GPIOs for data pins
extern GPIO_Type *d0_gpio, *d1_gpio, *d2_gpio, *d3_gpio,
				  *d4_gpio, *d5_gpio, *d6_gpio, *d7_gpio;

// GPIOs for additional pins
extern GPIO_Type *enable_gpio, *rw_gpio, *rs_gpio;

////////// Functions  //////////
void LCD4_Init(void);
void LCD4_Command(unsigned char);
void LCD4_CommandNoWait(unsigned char);
void LCD4_Data(unsigned char);
void LCD4_Ready(void);

// Higher level commands
void LCD4_Write(char *);
void LCD4_Clear(void);
void LCD4_Blink(void);
void LCD4_NoBlink(void);

#endif /*LCD4_H*/
