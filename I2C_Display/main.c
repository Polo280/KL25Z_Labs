#include "main.h"

// Config functions
static void UART_Config(void);
static void I2C_Config(void);
static void GPIO_Config(void);
static void ClockConfig(void);

// Functionalities
static void delayMs(uint16_t);
static void delayMicrosecs(uint64_t);
static void LED_On(GPIO_Type *, uint32_t);
static void LED_Off(GPIO_Type *, uint32_t);
static void LED_Toggle(GPIO_Type *, uint32_t);
static void ResetWatchDog(void);

// Variables
uint64_t millis = 0;
uint64_t blink_aux = 0, print_aux = 0;
char uart_c[50];

// Master control
uint8_t program_state = 0;   // 0 = no error, 1 = error

int main(void){
	// Clock configure
	ClockConfig();
	// Configure GPIOs
	GPIO_Config();
	// Configure UART channel
	UART_Config();
	// I2C Config
	I2C_Config();

	// Turn off LEDS
	GPIOD->PDOR |= 0x02;        // Blue
	GPIOB->PDOR |= 0x40000;     // Red
	GPIOB->PDOR |= 0x80000;     // Green

	// Configure display
	LCD_DefaultInit(I2C0, &millis);

	// Starting sequence
	LED_On(GPIOB, GREEN);
	delayMs(3000);
	LED_Off(GPIOB, GREEN);

	// UART message
	strcpy(uart_c, "hello\n");

	// Main loop
	while(1){
		//ResetWatchDog();

		// Print in UART0
		if(millis - print_aux >= TIME_TO_PRINT_MS){
			for(uint8_t i=0; i < strlen(uart_c); i++){
				UART0->D = uart_c[i];
				while(!(UART0->S1 & 0x40)){  // Wait for the TX buffer to be empty
					if(millis - print_aux >= UART_TIMEOUT_MS){
						print_aux = millis;
						break;
					}
				}
			}
			print_aux = millis;
		}

		// Toggle the LED each second
		if(millis - blink_aux >= TIME_TO_BLINK_MS){
			if(!program_state){
				LED_Off(GPIOB, RED);
				LED_Toggle(GPIOD, BLUE);  // FINE
			}
			else{
				LED_Off(GPIOB, BLUE);
				LED_Toggle(GPIOD, RED);   // ERROR
			}
			blink_aux = millis;
		}
	}
	return 0;
}

// Systick callback
void SysTick_Handler(void){   // I configured it to get called each millisecond
	millis ++;
}

// Function to configure clocks
void ClockConfig(void){
	// Systick clock frequency = 21 Mhz
	SysTick->LOAD = 21799;    	 // Remember default systick freq for KL25Z is 41.94 Mhz
	SysTick->CTRL |= 0x05;     	 // Enable timer and set sysclk as source clock
	SysTick->CTRL |= 0x02;     // Enable Systick interrupt

	// Configure watchdog
	SIM->COPC = 0x0C;
}

// Function to configure GPIOs
void GPIO_Config(void){
	// LED config
	SIM->SCGC5 |= 0x400;   // Enable clock port B
	SIM->SCGC5 |= 0x1000;  // Enable clock port D

	// Set pins as GPIOS
	PORTB->PCR[19] = 0x100;
	PORTB->PCR[18] = 0x100;
	PORTD->PCR[1] = 0x100;

	GPIOB->PDDR |= 0x80000;  // Enable (bit) pin 19 from GPIOB as output (Green)
	GPIOD->PDDR |= 0x02;     // Enable (bit) pin 1 from GPIOD as output (Blue)
	GPIOB->PDDR |= 0x40000;  // Enable (bit) pin 18 from GPIOB as output (Red)
}

// Function to configure UART channels
void UART_Config(void){
	// Clocks
	SIM->SCGC4 |= 0x400;	   // Enable clock for UART0
	SIM->SOPT2 |= 0x4000000;   // Specify clock
	// Baud Rate
	UART0->BDH = 0x00;
	UART0->BDL = 0x0C;         // Baud Rate = 115200
	// Configurations
	UART0->C1 |= 0x80;  	   // UART RX and TX in different pins (normal operation)
	UART0->C2 |= 0x08;		   // Enable UART Transmitter
	UART0->C4 |= 0x0F;		   // Enable oversampling to 16
	// Ports
	SIM->SCGC5 |= 0x200;       // Enable clock for PORTA
	PORTA->PCR[2] = 0x0200;    // Make PTA2 UART0 Pin
}

// Configure I2C
void I2C_Config(void){
	SIM->SCGC4 |= 0x40;   // Enable I2C0 clock
	I2C0->C1 = 0x00;      // Disable I2C0 peripheral to configure

	// Need to reach a frequency close to 100 khz
	I2C0->F |= 0x80;      // Set Mul factor to 4
	I2C0->F |= 0x0E;      // Go to manual page 706 (SCL divider of 56)

	I2C0->C1 |= 0X80;     // Enable I2C module
}

// Turn on LED
void LED_On(GPIO_Type *port, uint32_t pin){
	port->PDOR &= ~pin;
}

// Turn off LED
void LED_Off(GPIO_Type *port, uint32_t pin){
	port->PDOR |= pin;
}

// Toggle the LED
void LED_Toggle(GPIO_Type *port, uint32_t pin){
	port->PTOR |= pin;
}

// A function to reset watchdog
void ResetWatchDog(void){
	// Reset watchdog
	SIM->SRVCOP = 0x55;
	SIM->SRVCOP = 0xAA;
}

// Delay in milliseconds
void delayMs(uint16_t millisecs){   // Use the systick timer module
	uint16_t counter = 0;

	// Check until current count = millis
	while(counter < millisecs){
		if(SysTick->CTRL & 0x10000){
			counter ++;
		}
	}
}

// Delay microseconds
void delayMicrosecs(uint64_t microsecs){   // Use the systick timer module
	uint64_t counter = 0;

	// Check until current count = millis
	while(counter < microsecs){
		if(SysTick->CTRL & 0x10000){
			counter ++;
		}
	}
}
