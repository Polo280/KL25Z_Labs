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

#define BAUD4_BDH 0x01
#define BAUD4_BDL 0x38

/////////// Variables //////////

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

/////////// Functions //////////
void SetTX_Buffer(char *);
void UART_Config(void);
void UART_Send(void);

#endif /*UART_H*/
