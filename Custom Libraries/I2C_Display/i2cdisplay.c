/////////// Includes ///////////
#include "i2cdisplay.h"

/////////// Variables //////////
LCD_Config_Struct default_config = {
	.address = addr2,
	.rows = 2,
	.cols = 16,
	.backlight = LCD_BACKLIGHT
};

// I2C handler
I2C_Type *hi2c;
// Timing variable
volatile const uint64_t *millisecs;

/////////// Functions //////////
// Default Init
RETURN_TYPE LCD_DefaultInit(I2C_Type *handler, volatile const uint64_t *milliseconds){
	hi2c = handler;
	millisecs = milliseconds;
	return LCD_Init(&default_config);
}

// Custom Init
RETURN_TYPE LCD_Init(LCD_Config_Struct *configStruct){
	uint8_t status = 0;
	configStruct->displayFunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;

	if(configStruct->rows > 1)
		configStruct->displayFunction |= LCD_2LINE;

	// For the cases you want a 10 px high font
	if(configStruct->datasize != 0 && configStruct->rows == 1)
		configStruct->displayFunction |= LCD_5x10DOTS;

	// Small delay before sending commands
	delayMs(50);

	// Pull Rs and R/W to start sending commands
	status = I2C_WriteByte(configStruct->backlight, configStruct->address);
	if(status ==  1){return 1;}
	delayMs(800);

	// We start in 8bit mode, try to set 4bit mode
	status = Write4Bits((0x03 << 4), configStruct->address);
	if(status ==  1){return 1;}
	delayMs(10);

	// second try
	status = Write4Bits((0x03 << 4), configStruct->address);
	if(status ==  1){return 1;}
	delayMs(10); // wait min 4.1ms

	// third go!
	status = Write4Bits((0x03 << 4), configStruct->address);
	if(status ==  1){return 1;}
	delayMs(10);

	// finally, set to 4-bit interface
	status = Write4Bits((0x02 << 4) | configStruct->backlight, configStruct->address);
	if(status ==  1){return 1;}
	delayMs(10);

	// set # lines, font size, etc.
	status = Command(LCD_FUNCTIONSET | configStruct->displayFunction, configStruct->address);
	if(status ==  1){return 1;}
	delayMs(5);

	// Turn on the display with no cursor and no blinking (default)
	configStruct->displayControl = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	//Display();
	///delayMs(4);

	// Clear
	//Clear();
	delayMs(5);
	Backlight();

	// Initialize to default text direction
	//configStruct->displayMode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	// Set the entry mode
	//Command(LCD_ENTRYMODESET | configStruct->displayMode | configStruct->backlight, configStruct->address);
	// Home
	//Home();

	return NO_ERROR;
}

RETURN_TYPE Write4Bits(uint8_t value, uint16_t address){
	I2C_WriteByte(value | default_config.backlight, address);
	delayMs(10);
	return PulseEnable(value, address);
}

RETURN_TYPE PulseEnable(uint8_t data, uint16_t address){
	I2C_WriteByte((data | En) | default_config.backlight, address);
	delayMs(10);

	I2C_WriteByte((data & ~En) | default_config.backlight, address);
	delayMs(10);
	return NO_ERROR;
}

////// Mid level commands for sending data/commands //////
RETURN_TYPE Command(uint8_t value, uint16_t address){
	return Send(value, 0, address);
}

RETURN_TYPE Send(uint8_t value, uint8_t mode, uint16_t address){
	uint8_t highnib = (value & 0xf0) | mode;
	uint8_t lownib = ((value << 4) & 0xf0) | mode;
	Write4Bits(highnib, address);
	delayMs(10);
	return Write4Bits(lownib, address);
}

////// HIGH LEVEL COMMANDS! - For the User  //////
RETURN_TYPE Clear(void){
	Command(LCD_CLEARDISPLAY, default_config.address);  // Clear and set cursor position to 0
	delayMs(10);  // This command takes a long time!
	return NO_ERROR;
}

RETURN_TYPE Home(void){
	Command(LCD_RETURNHOME | default_config.backlight, default_config.address);
	delayMs(4);   // This command takes a long time!
	return NO_ERROR;
}

// Print the specified text into the display
RETURN_TYPE PrintStr(const char *text){
	uint8_t max_length = 16, index = 0;
	while(*text != '\0'){   // Wait for null terminated character
		I2C_WriteByte(*text, default_config.address);
		text++;
	}
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

RETURN_TYPE Backlight(void){
	default_config.backlight = LCD_BACKLIGHT;
	return I2C_WriteByte(0 | default_config.backlight, default_config.address);
}

RETURN_TYPE NoBacklight(void){
	default_config.backlight = LCD_NOBACKLIGHT;
	return I2C_WriteByte(0 | default_config.backlight, default_config.address);
}

// I2C Functionalities - this should go inside an i2c specific file
RETURN_TYPE I2C_WriteByte(uint8_t data, uint16_t address){
	uint64_t time_aux = *millisecs;

	data = data;

	// Wait until i2c bus is free
	time_aux = *millisecs;
	while(hi2c->S & 0x20){ 	  // While it is busy check timeout
		if(*millisecs - time_aux >= I2C_TIMEOUT_MS * conv_factor){
			return ERROR;
		}
	}

	// Send start bit
	hi2c->C1 |= 0x10;   // Transmitter on
	hi2c->C1 |= 0x20;   // Set as master mode
	delayUs(100);

	// Send address
	hi2c->D = address << 1;  // Set LSB to 0 which means write

	time_aux = *millisecs;
	while(!(hi2c->S & 0x20)){    // Wait for transfer completed
		if(*millisecs - time_aux >= I2C_TIMEOUT_MS * conv_factor){
			return ERROR;
		}
	}
	// Clear Interrupt Flag
	hi2c->S |= 0x02;
	// Read acknowledge bit
	if(hi2c->S & 0x01)
		return ERROR;     // NACK (No acknowledge detected)
	delayUs(100);

	// Send Data
	hi2c->D = data;
	while(!(hi2c->S & 0x20)){    // Wait for transfer completed
		if(*millisecs - time_aux >= I2C_TIMEOUT_MS * conv_factor){
			return ERROR;
		}
		time_aux = *millisecs;
	}

	// Clear Interrupt Flag
	hi2c->S |= 0x02;

	if(hi2c->S & 0x01)
		return ERROR;     // NACK (No acknowledge detected)
	delayUs(100);

	// End transmission
	hi2c->C1 &= ~0x30;
	delayUs(1000);
	return NO_ERROR;
}
