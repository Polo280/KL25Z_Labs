/////////// Includes ///////////
#include "i2cdisplay.h"

/////////// Variables //////////
LCD_Config_Struct default_config = {
	.address = addr1,
	.rows = 2,
	.cols = 16,
	.backlight = LCD_BACKLIGHT
};

// I2C handler
I2C_Type *hi2c;
// Timing variable
uint64_t *millisecs;

/////////// Functions //////////
// Default Init
RETURN_TYPE LCD_DefaultInit(I2C_Type *handler, uint64_t *milliseconds){
	hi2c = handler;
	millisecs = milliseconds;
	return LCD_Init(&default_config);
}

// Custom Init
RETURN_TYPE LCD_Init(LCD_Config_Struct *configStruct){
	configStruct->displayFunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;

	if(configStruct->rows > 1)
		configStruct->displayFunction |= LCD_2LINE;

	// For the cases you want a 10 px high font
	if(configStruct->datasize != 0 && configStruct->rows == 1)
		configStruct->displayFunction |= LCD_5x10DOTS;

	// Small delay before sending commands
	delayMillis(50);

	// Pull Rs and R/W to start sending commands
	I2C_WriteByte(configStruct->backlight, configStruct->address);
	delayMillis(1000);

	// We start in 8bit mode, try to set 4bit mode
	Write4Bits((0x03 << 4) | configStruct->backlight, configStruct->address);
	delayMicros(4500);

	// second try
	Write4Bits((0x03 << 4) | configStruct->backlight, configStruct->address);
	delayMicros(4500); // wait min 4.1ms

	// third go!
	Write4Bits((0x03 << 4) | configStruct->backlight, configStruct->address);
	delayMicros(150);

	// finally, set to 4-bit interface
	Write4Bits((0x02 << 4) | configStruct->backlight, configStruct->address);

	// set # lines, font size, etc.
	Command(LCD_FUNCTIONSET | configStruct->displayFunction | configStruct->backlight, configStruct->address);

	// Turn on the display with no cursor and no blinking (default)
	configStruct->displayControl = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	Display();

	// Clear
	Clear();

	// Initialize to default text direction
	configStruct->displayMode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	// Set the entry mode
	Command(LCD_ENTRYMODESET | configStruct->displayMode | configStruct->backlight, configStruct->address);
	// Home
	Home();

	return NO_ERROR;
}

RETURN_TYPE Write4Bits(uint8_t value, uint16_t address){
	I2C_WriteByte(value, address);
	return PulseEnable(value, address);
}

RETURN_TYPE PulseEnable(uint8_t data, uint16_t address){
	I2C_WriteByte(data | En, address);
	delayMicros(1);

	I2C_WriteByte(data & ~En, address);
	delayMicros(50);
	return NO_ERROR;
}

////// Mid level commands for sending data/commands //////
RETURN_TYPE Command(uint8_t value, uint16_t address){
	return Send(value, 0, address);
}

RETURN_TYPE Send(uint8_t value, uint8_t mode, uint16_t address){
	uint8_t highnib = value & 0xf0;
	uint8_t lownib =(value << 4) & 0xf0;
	Write4Bits((highnib)|mode, address);
	return Write4Bits((lownib)|mode, address);
}

////// HIGH LEVEL COMMANDS! - For the User  //////
RETURN_TYPE Clear(void){
	Command(LCD_CLEARDISPLAY | default_config.backlight, default_config.address);  // Clear and set cursor position to 0
	delayMillis(2);  // This command takes a long time!
	return NO_ERROR;
}

RETURN_TYPE Home(void){
	Command(LCD_RETURNHOME | default_config.backlight, default_config.address);
	delayMillis(2);   // This command takes a long time!
	return NO_ERROR;
}

RETURN_TYPE SetCursor(uint8_t col, uint8_t row){
	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	if (row > default_config.rows) {
		row = default_config.rows - 1;    // we count rows starting w/0
	}
	return Command(LCD_SETDDRAMADDR | (col + row_offsets[row]) | default_config.backlight, default_config.address);
}

////// Display configuring functions //////
RETURN_TYPE Display(void){
	default_config.displayControl |= LCD_DISPLAYON;
	return Command(LCD_DISPLAYCONTROL | default_config.displayControl | default_config.backlight, default_config.address);
}

RETURN_TYPE NoDisplay(void){
	default_config.displayControl &= ~LCD_DISPLAYON;
	return Command(LCD_DISPLAYCONTROL | default_config.displayControl | default_config.backlight, default_config.address);
}

RETURN_TYPE Cursor(void){
	default_config.displayControl |= LCD_CURSORON;
	return Command(LCD_DISPLAYCONTROL | default_config.displayControl | default_config.backlight, default_config.address);
}

RETURN_TYPE NoCursor(void){
	default_config.displayControl &= ~LCD_CURSORON;
	return Command(LCD_DISPLAYCONTROL | default_config.displayControl | default_config.backlight, default_config.address);
}

RETURN_TYPE Blink(void){
	default_config.displayControl |= LCD_BLINKON;
	return Command(LCD_DISPLAYCONTROL | default_config.displayControl | default_config.backlight, default_config.address);
}

RETURN_TYPE NoBlink(void){
	default_config.displayControl &= ~LCD_BLINKON;
	return Command(LCD_DISPLAYCONTROL | default_config.displayControl | default_config.backlight, default_config.address);
}


// Delay in milliseconds
void delayMillis(uint16_t millisecs){   // Use the systick timer module
	uint16_t counter = 0;

	// Check until current count = millis
	while(counter < millisecs){
		if(SysTick->CTRL & 0x10000){
			counter ++;
		}
	}
}

// Delay microseconds
void delayMicros(uint64_t microsecs){   // Use the systick timer module
	uint64_t counter = 0;

	// Check until current count = millis
	while(counter < microsecs){
		if(SysTick->CTRL & 0x10000){
			counter ++;
		}
	}
}

// I2C Functionalities - this should go inside an i2c specific file
RETURN_TYPE I2C_WriteByte(uint8_t data, uint16_t address){
	uint64_t time_aux = *millisecs;

	// Wait until i2c bus is free
	while(hi2c->S & 0x20){ 	  // While it is busy check timeout
		if(*millisecs - time_aux >= I2C_TIMEOUT_MS){
			return ERROR;
		}
		time_aux = *millisecs;
	}

	// Send start bit
	hi2c->C1 |= 0x10;   // Transmitter on
	hi2c->C1 |= 0x20;   // Set as master mode

	// Send address
	hi2c->D = address << 1;  // Set LSB to 0 which means write
	while(!(hi2c->S & 0x80)){    // Wait for transfer completed
		if(*millisecs - time_aux >= I2C_TIMEOUT_MS){
			return ERROR;
		}
		time_aux = *millisecs;
	}
	// Read acknowledge bit
	if(hi2c->S & 0x01)
		return ERROR;     // NACK (No acknowledge detected)

	// Send Data
	hi2c->D = data;
	while(!(hi2c->S & 0x80)){    // Wait for transfer completed
		if(*millisecs - time_aux >= I2C_TIMEOUT_MS){
			return ERROR;
		}
		time_aux = *millisecs;
	}
	if(hi2c->S & 0x01)
		return ERROR;     // NACK (No acknowledge detected)

	// End transmission
	hi2c->C1 &= ~0x30;
	delayMicros(500);
	return NO_ERROR;
}
