#include "main.h"
#include "math.h"

#define MAX_RESOLUTION 256

// Functions
static void DAC_Init(void);
static void UART_Config(void);
static void SineGenerate(uint64_t);
static void SquareGenerate(uint64_t);
static void TriangularGenerate(uint64_t);
static uint64_t CalculateTicks(uint64_t);

// Initial values for function generation
uint16_t value_index = 0;
uint16_t resolution = 0;
uint8_t mode = SQUARE;
uint64_t update_ticks = 0;
uint64_t frequency = 50;   // Frequency in hertz
uint8_t updated = 0;  // Used as a boolean

// Time aux
uint64_t blink_aux = 0, wave_aux = 0, print_aux = 0;

// UART buffer
char uart_rx[50];
char uart_tx[100];
uint8_t uart_rx_index = 0;
uint64_t last_rx_aux = 0;

// Systick pointer
const uint64_t *systicks;

// Values for a sine wave
uint16_t lookupTable[256];

// LCD buffer
char lcd_buff[16];

int main(void) {
    // Config functions
    ClkInitIt(CLK_TENS_MICROS);
    // Init RGB led
    RGB_Init();
    // Init DAC
    DAC_Init();
    // Init UART
    UART_Config();
    // LCD i2c init
    LCD_init();
    // Set systick pointer
    systicks = GetSysTicks();

    SquareGenerate(frequency);
    //SquareGenerate(frequency);
    //TriangularGenerate(frequency);

    // Main loop
    while (1) {
    	// If something should be updated
    	if(updated == 1){
    		SquareGenerate(frequency);
    		updated = 0;
    	}

        // Blink led
        if (*systicks - blink_aux >= TIME_TO_BLINK_MS * conv_factor) {
            BlueToggle();
            blink_aux = *systicks;
        }

        // Update waveform
        if(*systicks - wave_aux >= update_ticks){
        	value_index = (value_index >= resolution - 1)? 0 : value_index+1;
        	DAC0->DAT[0].DATL = lookupTable[value_index] & 0xff;   /* write low byte */
        	DAC0->DAT[0].DATH = (lookupTable[value_index] >> 8) & 0x0f; /* write high byte */
        	wave_aux = *systicks;
        }
    }
    return 0;
}

// Function to fill lookup table with sine values
static void SineGenerate(uint64_t freq){
	const float M_PI = 3.1415926535897;
	uint64_t period_ticks = CalculateTicks(freq);
	update_ticks = period_ticks / MAX_RESOLUTION;
	resolution = (update_ticks == 0)? period_ticks : MAX_RESOLUTION;
	float fRadians = ((2 * M_PI) / resolution);
	for (uint16_t i = 0; i < resolution; i++) {
		lookupTable[i] = 2047 * (sinf(fRadians * i) + 1);
	}
}

// Function to fill lookup table with square values
static void SquareGenerate(uint64_t freq){
	uint64_t period_ticks = CalculateTicks(freq);
	update_ticks = period_ticks / MAX_RESOLUTION;
	resolution = (update_ticks == 0)? period_ticks : MAX_RESOLUTION;

	for (uint16_t i = 0; i < resolution; i++) {
		if(i >= resolution / 2){
			lookupTable[i] = 4095;
		}else{
			lookupTable[i] = 0;
		}
	}
}

// Function to fill lookup table with triangular values
static void TriangularGenerate(uint64_t freq){
	uint64_t period_ticks = CalculateTicks(freq);
	update_ticks = period_ticks / MAX_RESOLUTION;
	resolution = (update_ticks == 0)? period_ticks : MAX_RESOLUTION;
	double slope_delta = 4095.0 / ((double) MAX_RESOLUTION / 2.0);

	for (uint16_t i = 0; i < resolution; i++) {
		if(i <= resolution / 2){
			lookupTable[i] = (i == 0)? 0 : lookupTable[i-1] + (uint8_t)slope_delta;
		}else{
			lookupTable[i] = lookupTable[i-1] - (uint8_t)slope_delta;
			if(lookupTable[i] < 0){lookupTable[i] = 0;}
		}
	}
}

// Function to calculate needed ticks in one period to achieve desired frequency
static uint64_t CalculateTicks(uint64_t freq){
	 if (freq == 0) {
		return 0; // Handle the case where frequency is zero
	}
	double period = 1.0 / (double)freq;
	uint64_t ticks = (uint64_t)(period * 100000.0); // Assuming you are using TENS_MICROS as timebase
	return ticks;
}

// DAC configuring
static void DAC_Init(void) {
    SIM->SCGC6 |= SIM_SCGC6_DAC0_MASK;
    DAC0->C1 = 0; /* disable the use of buffer */
    DAC0->C0 = DAC_C0_DACEN_MASK | DAC_C0_DACRFS_MASK; /* enable DAC and use VDDA as reference */
}

static void UART_Config(void) {
    // Clocks
    SIM->SCGC4 |= SIM_SCGC4_UART0_MASK; // Enable clock for UART0
    SIM->SOPT2 |= SIM_SOPT2_UART0SRC(1); // Specify clock

    // Baud Rate
	UART0->BDH = 0x00;
	UART0->BDL = 0x0C;         // Baud Rate = 115200

    // Configurations
    UART0->C1 = 0x00; // 8-bit data, no parity
    UART0->C2 |= UART_C2_TE_MASK | UART_C2_RE_MASK; // Enable transmitter and receiver
    UART0->C4 |= 0x0F;	 // Enable oversampling to 16

    // Ports
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK; // Enable clock for PORTA
    PORTA->PCR[1] = PORT_PCR_MUX(2); // Set PTA1 to UART0_RX
    PORTA->PCR[2] = PORT_PCR_MUX(2); // Set PTA2 to UART0_TX

    // Enable interrupt mode for receiver
    UART0->C2 |= (1 << 5);   // Enable receive interrupt
    NVIC_EnableIRQ(UART0_IRQn);   // Enable interrupt with NVIC
}

// Callback for UART0 interrupt (An event related to UART0 has happened)
void UART0_IRQHandler(void) {
    while (UART0->S1 & UART_S1_RDRF_MASK) {
        char c = UART0->D;
        if (uart_rx_index < sizeof(uart_rx) - 1) {     // Ensure there's space in the buffer
            if (c == ' ') {
                uart_rx[uart_rx_index] = '\0';  // Null-terminate the string
                updated = 1;
                frequency = strtoull(uart_rx, NULL, 10);
                uart_rx_index = 0;  // Reset buffer index
            } else {
                uart_rx[uart_rx_index++] = c;
            }
        } else {
            uart_rx_index = 0;
            uart_rx[0] = '\0';
        }
    }
}
