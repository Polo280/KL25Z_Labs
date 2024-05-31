#include "Headers/LCD_4BIT.h"

// Static functions
static GPIO_Type* IdentifyGPIO(PORT_Type *);
static uint32_t ToBinaryGPIO(uint32_t);
static void ConfigurePort(PORT_Type *);
static void ConfigureGPIO(PORT_Type *, uint32_t);
static void SetGPIOs(void);
static void SendNibble(unsigned char nibble);

// Variables
GPIO_Type *d4_gpio, *d5_gpio, *d6_gpio, *d7_gpio;
GPIO_Type *enable_gpio, *rw_gpio, *rs_gpio;

// Default config struct
LCD4_Struct LCD4_defaultConfig = {
    // Ports
    .d4_port = PORTD, .d5_port = PORTD, .d6_port = PORTD, .d7_port = PORTD,
    .enable_port = PORTA, .rs_port = PORTA, .rw_port = PORTA,
    // Pins
    .d4_pin = 4, .d5_pin = 5, .d6_pin = 6, .d7_pin = 7,
    .enable_pin = 5, .rs_pin = 2, .rw_pin = 4
};

// LCD4 initialization
void LCD4_Init(void){
    // Configure GPIOs
    ConfigureGPIO(LCD4_defaultConfig.enable_port, LCD4_defaultConfig.enable_pin);
    ConfigureGPIO(LCD4_defaultConfig.rs_port, LCD4_defaultConfig.rs_pin);
    ConfigureGPIO(LCD4_defaultConfig.rw_port, LCD4_defaultConfig.rw_pin);
    // Data pins
    ConfigureGPIO(LCD4_defaultConfig.d4_port, LCD4_defaultConfig.d4_pin);
    ConfigureGPIO(LCD4_defaultConfig.d5_port, LCD4_defaultConfig.d5_pin);
    ConfigureGPIO(LCD4_defaultConfig.d6_port, LCD4_defaultConfig.d6_pin);
    ConfigureGPIO(LCD4_defaultConfig.d7_port, LCD4_defaultConfig.d7_pin);
    // Get the GPIOS with the port
    SetGPIOs();

    delayMs(10);
    LCD4_CommandNoWait(0x02);  // Initialize to 4-bit mode
    LCD4_Command(0x28);  // Function set: 4-bit, 2-line, 5x7 dots
    LCD4_Command(0x06);  // Entry mode set: increment cursor, no shift
    LCD4_Command(0x0F);  // Display on, cursor blinking
    LCD4_Command(0x01);  // Clear display
    delayUs(1090);
}

void LCD4_Command(unsigned char command){
    LCD4_Ready();
    rs_gpio->PCOR |= RS;
    rw_gpio->PCOR |= RW;
    SendNibble(command >> 4);  // Send high nibble
    delayUs(40);
    SendNibble(command & 0x0F);  // Send low nibble
}

void LCD4_CommandNoWait(unsigned char command){
    rs_gpio->PCOR |= RS;
    rw_gpio->PCOR |= RW;
    SendNibble(command >> 4);  // Send high nibble
    SendNibble(command & 0x0F);  // Send low nibble
    delayUs(40);
}

void LCD4_Data(unsigned char data){
    LCD4_Ready();
    rs_gpio->PSOR = RS;
    rw_gpio->PCOR = RW;
    SendNibble(data >> 4);  // Send high nibble
    SendNibble(data & 0x0F);  // Send low nibble
}

void LCD4_Ready(void){
    char status;
    d4_gpio->PDDR = 0;
    rs_gpio->PCOR = RS;
    rw_gpio->PSOR = RW;
    do{
        enable_gpio->PSOR = EN;  // Raise En
        delayMs(0);
        status = (d7_gpio->PDIR & (1 << LCD4_defaultConfig.d7_pin)) >> LCD4_defaultConfig.d7_pin;
        enable_gpio->PCOR = EN;
        delayMs(0);
    }while(status & 0x80);  // Keep blocking while LCD4 is busy

    rw_gpio->PCOR = RW;
    d4_gpio->PDDR = 0xFF;
    d5_gpio->PDDR = 0xFF;
    d6_gpio->PDDR = 0xFF;
    d7_gpio->PDDR = 0xFF;
}

// Higher level commands
void LCD4_Clear(void){
    LCD4_Command(LCD4_CLEAR_COMMAND);
}

void LCD4_NoBlink(void){
    LCD4_Command(LCD4_NO_BLINK);
}

void LCD4_Blink(void){
    LCD4_Command(LCD4_BLINK_HOME);
}

void LCD4_Write(char *data){
    while(*data != '\0'){
        LCD4_Data(*data);
        data++;
        delayUs(500);
    }
}

// Static functions
static void SetGPIOs(void){
    // Data pins
    d4_gpio = IdentifyGPIO(LCD4_defaultConfig.d4_port);
    d5_gpio = IdentifyGPIO(LCD4_defaultConfig.d5_port);
    d6_gpio = IdentifyGPIO(LCD4_defaultConfig.d6_port);
    d7_gpio = IdentifyGPIO(LCD4_defaultConfig.d7_port);
    // Additional pins
    enable_gpio = IdentifyGPIO(LCD4_defaultConfig.enable_port);
    rw_gpio = IdentifyGPIO(LCD4_defaultConfig.rw_port);
    rs_gpio = IdentifyGPIO(LCD4_defaultConfig.rs_port);
}

static void ConfigureGPIO(PORT_Type *port, uint32_t pin){
    ConfigurePort(port);
    port->PCR[pin] = 0x100;  // Set as GPIO

    if(port == PORTA){
        GPIOA->PDDR |= ToBinaryGPIO(pin);  // Set as output
    }else if (port == PORTB){
        GPIOB->PDDR |= ToBinaryGPIO(pin);
    }else if (port == PORTC){
        GPIOC->PDDR |= ToBinaryGPIO(pin);
    }else if (port == PORTD){
        GPIOD->PDDR |= ToBinaryGPIO(pin);
    }else{
        GPIOE->PDDR |= ToBinaryGPIO(pin);
    }
}

static void ConfigurePort(PORT_Type *port){
    if(port == PORTA){
        SIM->SCGC5 |= 0x200;  // Enable clock port A
    }else if (port == PORTB){
        SIM->SCGC5 |= 0x400;  // Enable clock port B
    }else if (port == PORTC){
        SIM->SCGC5 |= 0x800;  // Enable clock port C
    }else if (port == PORTD){
        SIM->SCGC5 |= 0x1000;  // Enable clock port D
    }else{
        SIM->SCGC5 |= 0x2000;  // Enable clock port E
    }
}

static GPIO_Type* IdentifyGPIO(PORT_Type *port){
    if(port == PORTA){
        return GPIOA;
    }else if(port == PORTB){
        return GPIOB;
    }else if(port == PORTC){
        return GPIOC;
    }else if(port == PORTD){
        return GPIOD;
    }else{
        return GPIOE;
    }
}


static uint32_t ToBinaryGPIO(uint32_t num){
    return (1 << num);
}

static void SendNibble(unsigned char nibble){
    // Clear previous data
    d4_gpio->PCOR = ToBinaryGPIO(LCD4_defaultConfig.d4_pin);
    d5_gpio->PCOR = ToBinaryGPIO(LCD4_defaultConfig.d5_pin);
    d6_gpio->PCOR = ToBinaryGPIO(LCD4_defaultConfig.d6_pin);
    d7_gpio->PCOR = ToBinaryGPIO(LCD4_defaultConfig.d7_pin);

    // Set new data
    if(nibble & 0x01) d4_gpio->PSOR = ToBinaryGPIO(LCD4_defaultConfig.d4_pin);
    if(nibble & 0x02) d5_gpio->PSOR = ToBinaryGPIO(LCD4_defaultConfig.d5_pin);
    if(nibble & 0x04) d6_gpio->PSOR = ToBinaryGPIO(LCD4_defaultConfig.d6_pin);
    if(nibble & 0x08) d7_gpio->PSOR = ToBinaryGPIO(LCD4_defaultConfig.d7_pin);

    // Pulse enable
    enable_gpio->PSOR = EN;
    delayMs(0);
    enable_gpio->PCOR = EN;
}
