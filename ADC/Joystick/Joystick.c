#include "main.h"

// Config functions
static void ClockConfig(void);
static void UART_Config(void);
static void ADC_Config(void);

volatile uint64_t millis = 0;
uint64_t blink_aux = 0, print_aux = 0, adc_aux = 0;

// UART buffer
char uart_c[50];
// ADC readings
uint16_t adc_res = 0, adc_res2 = 0;

int main(void){
	ClockConfig();
	UART_Config();
	ADC_Config();

	// Init RGB led
	RGB_Init();

	// Start ADC conversion in channel 0
	ADC0->SC1[0] = 0;

	// Main loop
	while(1){
		// Blink led
		if(millis - blink_aux >= TIME_TO_BLINK_MS){
			PurpleToggle();
			blink_aux = millis;
		}

		// Read ADC and clear flag
		if(millis - adc_aux >= TIME_TO_READ_ADC_MS){
			adc_aux = millis;

			while(!(ADC0->SC1[0] & 0x80)) {   // Wait for end of conversion flag
				if(millis - adc_aux >= ADC_TIMEOUT_MS){
					break;
				}
			}
			// Assign ADC result
			adc_res = ADC0->R[0];
			adc_aux = millis;

			// Start ADC conversion in channel 4
			ADC0->SC1[0] = 0x04;

			while(!(ADC0->SC1[0] & 0x80)) {   // Wait for end of conversion flag
				if(millis - adc_aux >= ADC_TIMEOUT_MS){
					break;
				}
			}
			// Store result
			adc_res2 = ADC0->R[0];
			adc_aux = millis;
			// Start ADC conversion in channel 0
			ADC0->SC1[0] = 0;
		}

		// Print in UART0
		if(millis - print_aux >= TIME_TO_PRINT_MS){
			// Assign values to the buffer
			snprintf(uart_c, sizeof(uart_c), "X: %d, Y: %d\n", adc_res, adc_res2);
			print_aux = millis;

			for(uint8_t i=0; i < strlen(uart_c); i++){
				UART0->D = uart_c[i];
				while(!(UART0->S1 & 0x40)){  // Wait for the TX buffer to be empty
					if(millis - print_aux >= UART_TIMEOUT_MS){
						break;
					}
				}
			}
			print_aux = millis;
		}
	}
	return 0;
}

// Systick callback
void SysTick_Handler(void){
	millis++;
}

// Configure ADC
void ADC_Config(void){
	SIM->SCGC5 |= 0x2000;       // Enable PORTE clock
	SIM->SCGC6 |= 0x8000000;    // ADC0 clock
	PORTE->PCR[20] = 0x000;     // Configure pin PORTE 20 as analog
	PORTE->PCR[21] = 0x000;     // Configure pin PORTE 21 as analog

	ADC0->SC2 &= ~0x40;      // Software trigger
	ADC0->CFG1 = 0x40 | 0x04 | 0x00 | 0x10;
}

void ClockConfig(void){
	// Systick clock frequency = 21 Mhz
	SysTick->LOAD = (SystemCoreClock / 1000) - 1;  // Remember default systick freq for KL25Z is 41.94 Mhz
	SysTick->CTRL |= 0x05;     	 // Enable timer and set sysclk as source clock
	SysTick->CTRL |= 0x02;       // Enable Systick interrupt
}

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
	PORTA->PCR[1] = 0x0200;    // Make PTA1 UART0 Pin
	PORTA->PCR[2] = 0x0200;    // Make PTA2 UART0 Pin
}
