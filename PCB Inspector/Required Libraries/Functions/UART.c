#include "Headers/UART.h"

// NOTE: UART0 is clocked directly from MCGFLLCLK unlike other peripherals
// which are clocked by the bus/flash clock

char *tx_buffer;

void SetTX_Buffer(char *buff){
	tx_buffer = buff;
}

void UART_Config(void){
	// Clocks
	SIM->SOPT2 |= (1 << 26);   // Specify clock  --> Uses the same as clock core (48 Mhz currently)
	SIM->SCGC4 |= 0x400;	   // Enable clock for UART0
	// Baud Rate
	UART0->BDH = 0x01;
	UART0->BDL = 0x38;         // Baud Rate = 9600
	//UART0->BDH = 0x01;       // 9600 for 72Mhz clock
	//UART0->BDL = 0xD6;
	// Configurations
	UART0->C1 |= 0x80;  	   // UART RX and TX in different pins (normal operation)
	UART0->C2 |= 0x08;		   // Enable UART Transmitter
	UART0->C4 |= 0x0F;		   // Enable oversampling to 16
	// Ports
	SIM->SCGC5 |= 0x200;       // Enable clock for PORTA
	PORTA->PCR[2] = 0x0200;    // Make PTA2 UART0 Pin
}

void UART_Send(void){
	for(uint8_t i=0; i < strlen(tx_buffer); i++){
		UART0->D = tx_buffer[i];
	}
}
