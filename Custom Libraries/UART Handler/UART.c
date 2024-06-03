#include "Headers/UART.h"

// NOTE: UART0 is clocked directly from MCGFLLCLK unlike other peripherals
// which are clocked by the bus/flash clock

static void AccumString(char);

// Buffer settings
uint16_t tx_size = 0, rx_size = 0;
char *tx_buffer;
char *rx_buffer;
// Buffer iterators
volatile uint16_t current_tx = 0, current_rx = 0;

//////// Set Buffers ////////
void SetRX_Buffer(char *buff, uint16_t size){
	rx_size = size;
	rx_buffer = buff;
}

void SetTX_Buffer(char *buff, uint16_t size){
	tx_size = size;
	tx_buffer = buff;
}

void UART0_Config(uint8_t mode, uint32_t baud_rate) {
	// Get variables for mode config
	uint8_t interrupt_rx = (mode == 1)? 1 : 0;
	uint8_t interrupt_tx = (mode == 2)? 1 : 0;
	if(mode == 3){interrupt_rx = 1; interrupt_tx = 1;}
    // Clocks
	SIM->SCGC4 |= 0x400;	   // Enable clock for UART0
	SIM->SOPT2 |= 0x4000000;   // Specify clock
    // Baud Rate
    uint32_t baud_bits[2];
    IdentifyBaudRate(baud_rate, baud_bits);
	UART0->BDH = baud_bits[0];
	UART0->BDL = baud_bits[1]; // Custom baud rate
    // Configurations
    UART0->C1 = 0x00;  // 8-bit data, no parity
    UART0->C2 |= UART_C2_TE_MASK | UART_C2_RE_MASK; // Enable transmitter and receiver
    UART0->C4 |= 0x0F;	 				// Enable oversampling to 16
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK; // Enable clock for PORTA
    PORTA->PCR[1] = PORT_PCR_MUX(2);    // Set PTA1 to UART0_RX
    PORTA->PCR[2] = PORT_PCR_MUX(2);    // Set PTA2 to UART0_TX
    // MODE EVALUATION
    if(interrupt_rx == 1){ UART0->C2 |= (1 << 5);}  // Enable receive interrupt
    if(interrupt_tx == 1){ UART0->C2 |= (1 << 6);}  // Enable transmission complete interrupt
    if(mode != 0){NVIC_EnableIRQ(UART0_IRQn);}      // Enable interrupt with NVIC
}

/**
 * @brief Function to get the BDH and BDL values to configure for a certain baud rate
 * */
void IdentifyBaudRate(uint32_t baud_rate, uint32_t bits[2]){
	uint32_t bdh = 0x00, bdl = 0x00;
	if(baud_rate == baud1){

	} else if(baud_rate == baud2){

	} else if(baud_rate == baud3){

	} else if(baud_rate == baud4){ bdh = BAUD4_BDH; bdl = BAUD4_BDL;
	} else if(baud_rate == baud5){

	} else if(baud_rate == baud6){

	} else if(baud_rate == baud7){

	} else if(baud_rate == baud8){

	} else if(baud_rate == baud9){bdh = BAUD9_BDH; bdl = BAUD9_BDL;
	} else{
	}
	// Modify array values directly
	bits[0] = bdh;
	bits[1] = bdl;
}

void UART_Send(void){
	for(uint8_t i=0; i < strlen(tx_buffer); i++){
		if(*(tx_buffer + i) == '\n' || *(tx_buffer + i) == '\0'){
			break;
		}else{
			while (!(UART0->S1 & UART_S1_TDRE_MASK)); // Wait for the transmit internal buffer to be empty
			UART0->D = *(tx_buffer + i);
		}
	}
}

// Callback for UART0 interrupt (An event related to UART0 has happened)
void UART0_IRQHandler(void) {
	BlueToggle();
	// Manage data reception
    if(UART0->S1 & UART_S1_RDRF_MASK) {
        char c = UART0->D;
        AccumString(c);
    }
}

static void AccumString(char c){
	// Manage overflow
	if(current_rx >= rx_size - 1 || current_rx < 0){
		*rx_buffer = '\0';   // Set to NULL first element of buffer
		current_tx = 0;      // Reset current rx index to 0 (reset the message)
	}
	// Otherwise, a normal operation
	if(c == ' '){
		*(rx_buffer + current_rx) = '\0';  // End string
		current_rx = 0;
		return;
	}else{
		*(rx_buffer + current_rx) = c;  // Update character
		current_rx++;
	}
}
