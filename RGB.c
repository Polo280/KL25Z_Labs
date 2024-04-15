#include "MKL25Z4.h"

// Definitions
enum RGB_LED {
	RED = 0x40000,
	GREEN = 0x20,
	BLUE = 0x80000
};

void ClockConfig(void);
void GPIO_Config(void);

// Functionalities
void LED_On(GPIO_Type * , uint32_t);
void LED_Off(GPIO_Type * , uint32_t);

int main(void){

	// Enable clocks
	ClockConfig();
	GPIO_Config();

	// Turn off LEDS
	GPIOD->PDOR |= 0x20;        // Green
	GPIOB->PDOR |= 0x40000;     // Red
	GPIOB->PDOR |= 0x80000;     // Blue

	// Turn off alternative
	//LED_Off(GPIOD, GREEN);
	//LED_Off(GPIOB, RED);
	//LED_Off(GPIOB, BLUE);

	// Turn on leds sequentially

	GPIOD->PDOR &= ~0x20;    // blue
	//GPIOB->PDOR &= ~0x40000;   // RED
	//GPIOB->PDOR &= ~0x80000; // Green

	// Easier alternative
	//LED_On(GPIOD, GREEN);
	//LED_On(GPIOB, RED);
	//LED_On(GPIOB, BLUE);

	// Main loop
	while(1){

	}

	return 0;
}

// Clock configurations
void ClockConfig(void){
	SIM->SCGC5 |= 0x400;   // Enable clock port B
	SIM->SCGC5 |= 0x1000;  // Enable clock port D
}

// GPIO Configurations
void GPIO_Config(void){
	// Set pins as GPIOS
	PORTB->PCR[19] = 0x100;
	PORTB->PCR[18] = 0x100;
	PORTD->PCR[5] = 0x100;

	GPIOB->PDDR |= 0x80000;  // Enable (bit) pin 19 from GPIOB as output (Green)
	GPIOD->PDDR |= 0x20;     // Enable (bit) pin 5 from GPIOD as output (Blue)
	GPIOB->PDDR |= 0x40000;  // Enable (bit) pin 18 from GPIOB as output (Red)
}

// Turn on LED
void LED_On(GPIO_Type *port, uint32_t pin){
	port->PDOR &= ~pin;
}

// Turn off LED
void LED_Off(GPIO_Type *port, uint32_t pin){
	port->PDOR |= pin;
}


