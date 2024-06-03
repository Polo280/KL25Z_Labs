#ifndef UART_H
#define UART_H

/////////// Includes ///////////
#include "MKL25Z4.h"
#include "main.h"
//////////// MACROS ////////////
#define UART0_PORT PORTA
#define UART0_PIN_RX 1
#define UART0_PIN_TX 2

#define UART1_PORT PORTE
#define UART1_PIN_RX 1
#define UART1_PIN_TX 0

#define UART2_PORT PORTE
#define UART2_PIN_RX 23
#define UART2_PIN_TX 22

#define BAUD4_BDH 0x00
#define BAUD4_BDL 0x82

#define BAUD9_BDH 0x00
#define BAUD9_BDL 0x0C

/////////// Variables //////////

volatile extern uint16_t current_tx, current_rx;
extern uint16_t tx_size, rx_size;
extern char *tx_buffer;
extern char *rx_buffer;

// Store common baud rates
enum BaudRates{
	baud1 = 1200,
	baud2 = 2400,
	baud3 = 4800,
	baud4 = 9600,
	baud5 = 19200,
	baud6 = 38400,
	baud7 = 57600,
	baud8 = 74880,
	baud9 = 115200
};

// Mode selection for UART channels
enum UART_Modes{
	polling = 0,
	interrupt_rx = 1,
	interrupt_tx = 2,
	interrupt_all = 3
};

// UART module selection
enum UART_Channels{
	uart0 = 0,
	uart1 = 1,
	uart2 = 2
};

/////////// Functions //////////
void SetTX_Buffer(char *, uint16_t);
void SetRX_Buffer(char *, uint16_t);
void UART_Init(uint8_t, uint8_t);
void UART0_Config(uint8_t, uint32_t);
void IdentifyBaudRate(uint32_t, uint32_t [2]);
void UART_Send(void);

#endif /*UART_H*/
