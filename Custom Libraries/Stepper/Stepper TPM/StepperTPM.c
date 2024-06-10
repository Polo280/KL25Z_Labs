#include "Headers/StepperTPM.h"

// Declare functions
static GPIO_Type* IdentifyGPIO(PORT_Type *);
static uint32_t ToBinaryGPIO(uint32_t);
static void PortConfigure(PORT_Type *);
static void SetPinLow(GPIO_Type *, uint32_t);
static void SetPinHigh(GPIO_Type *, uint32_t);
static void GPIO_Configure(PORT_Type *, uint32_t);

// Set initial motor states to Off
uint16_t StepperXFreq = 0;
uint16_t StepperYFreq = 0;
uint16_t StepperZFreq = 0;

// Initialize TPMs and GPIOs used for direction configuration
void SteppersInit(void){
	// TPMs Init
	AllTPMsInit();
	// Direction Pins init
	GPIO_Configure(STEPPERX_DIR_PORT, STEPPERX_DIR_PIN);
	GPIO_Configure(STEPPERY_DIR_PORT, STEPPERY_DIR_PIN);
	GPIO_Configure(STEPPERZ_DIR_PORT, STEPPERZ_DIR_PIN);
	// Set Direction
	StepperX_Direction(CLOCKWISE);
	StepperY_Direction(CLOCKWISE);
	StepperZ_Direction(CLOCKWISE);
	// Motors Off
	//SetMotorsOff();
}

// Initialize all TPMs
void AllTPMsInit(void){
	// Configure timers for the motors
	TPM0_Init();
	TPM1_Init();
	TPM2_Init();
}

// Configure the TPM0 timer (Stepper X) - Options: various pins on PORTD
void TPM0_Init(void) {
    // Enable clock to Port D
    SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
    // Configure PTD0, PTD2, and PTD3 for TPM0 usage
    STEPPERX_PORT->PCR[STEPPERX_PIN] = PORT_PCR_MUX(4); // Channel 0 currently
    // Enable clock to TPM0
    SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;
    // Use MCGFLLCLK as the timer counter clock
    SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1); // MCGFLLCLK
    // Disable TPM0 timer before configuration
    TPM0->SC = 0;
    // Configure TPM0 Channel 0 for edge-aligned PWM with high-true pulses
    TPM0->CONTROLS[0].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;
    // Enable TPM0 with prescaler of 16 and edge-aligned mode
    TPM0->SC = TPM_SC_PS(3) | TPM_SC_CMOD(1); // Prescaler = 16, clock mode = 1
}

// Configure the TPM1 timer (Stepper Y) - Options: PTA12, PTA13
void TPM1_Init(void) {
    // Enable clock to Port A and Port E
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTE_MASK;
    // Configure PTA12 and PTA13 for TPM1 usage
    STEPPERY_PORT->PCR[STEPPERY_PIN] = PORT_PCR_MUX(3); // TPM1_CH0 currently
    // Enable clock to TPM1
    SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK;
    // Use MCGFLLCLK as the timer counter clock
    SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);
    // Disable TPM1 timer before configuration
    TPM1->SC = 0;
    // Configure TPM1 channels for edge-aligned PWM with high-true pulses
    TPM1->CONTROLS[0].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;
    // Enable TPM1 with prescaler of 16 and edge-aligned mode
    TPM1->SC = TPM_SC_PS(3) | TPM_SC_CMOD(1);
}

// Configure the TPM2 timer (Stepper Z) - Options: PTB18, PTB19
void TPM2_Init(void) {
    // Enable clock to Port A and Port B
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK;
    // Configure PTB18 and PTB19 for TPM2 usage
    STEPPERZ_PORT->PCR[STEPPERZ_PIN] = PORT_PCR_MUX(3); // TPM2_CH0 currently
    // Enable clock to TPM2
    SIM->SCGC6 |= SIM_SCGC6_TPM2_MASK;
    // Use MCGFLLCLK as the timer counter clock
    SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);
    // Disable TPM2 timer before configuration
    TPM2->SC = 0;
    // Configure TPM2 channels for edge-aligned PWM with high-true pulses
    TPM2->CONTROLS[0].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;
    // Enable TPM2 with prescaler of 16 and edge-aligned mode
    TPM2->SC = TPM_SC_PS(3) | TPM_SC_CMOD(1);
}

// Set the direction of the steppers
void StepperX_Direction(uint8_t direction){
	if(direction == CLOCKWISE){
		SetPinHigh(IdentifyGPIO(STEPPERX_DIR_PORT), STEPPERX_DIR_PIN);
	}else{
		SetPinLow(IdentifyGPIO(STEPPERX_DIR_PORT), STEPPERX_DIR_PIN);
	}
}

void StepperY_Direction(uint8_t direction){
	if(direction == CLOCKWISE){
		SetPinHigh(IdentifyGPIO(STEPPERY_DIR_PORT), STEPPERY_DIR_PIN);
	}else{
		SetPinLow(IdentifyGPIO(STEPPERY_DIR_PORT), STEPPERX_DIR_PIN);
	}
}

void StepperZ_Direction(uint8_t direction){
	if(direction == CLOCKWISE){
		SetPinHigh(IdentifyGPIO(STEPPERZ_DIR_PORT), STEPPERZ_DIR_PIN);
	}else{
		SetPinLow(IdentifyGPIO(STEPPERZ_DIR_PORT), STEPPERX_DIR_PIN);
	}
}


// Set Stepper speed based on an input percentage
void StepperX_RunPercentage(uint8_t percentage){
	// Calculate operating frequency based on maximum allowed frequency
	uint16_t frequency = (STEPPER_MAX_FREQUENCY_HZ * percentage) / 100;
	StepperX_RunFreq(frequency);
}

void StepperY_RunPercentage(uint8_t percentage){
	// Calculate operating frequency based on maximum allowed frequency
	uint16_t frequency = (STEPPER_MAX_FREQUENCY_HZ * percentage) / 100;
	StepperY_RunFreq(frequency);
}

void StepperZ_RunPercentage(uint8_t percentage){
	// Calculate operating frequency based on maximum allowed frequency
	uint16_t frequency = (STEPPER_MAX_FREQUENCY_HZ * percentage) / 100;
	StepperZ_RunFreq(frequency);
}

// Set all motors speed to the same specific speed
void RunMotorsPercentage(uint8_t percentage){
	StepperX_RunPercentage(percentage);
	StepperY_RunPercentage(percentage);
	StepperZ_RunPercentage(percentage);
}

// Run steppers to an specific frequency
void StepperX_RunFreq(uint16_t freq){
	// Calculate the mod for adjusting tpm frequency
	uint16_t tpm_mod = (TPM_FREQUENCY / freq) - 1;
	TPM0->MOD = tpm_mod;
	// Run to duty cycle 50%
	TPM0->CONTROLS[0].CnV = tpm_mod / 2;
	// Update global variable
	StepperXFreq = freq;
}

void StepperY_RunFreq(uint16_t freq){
	// Calculate the mod for adjusting tpm frequency
	uint16_t tpm_mod = (TPM_FREQUENCY / freq) - 1;
	TPM1->MOD = tpm_mod;
	// Run to duty cycle 50%
	TPM1->CONTROLS[0].CnV = tpm_mod / 2;
	// Update global variable
	StepperYFreq = freq;
}

void StepperZ_RunFreq(uint16_t freq){
	// Calculate the mod for adjusting tpm frequency
	uint16_t tpm_mod = (TPM_FREQUENCY / freq) - 1;
	TPM2->MOD = tpm_mod;
	// Run to duty cycle 50%
	TPM2->CONTROLS[0].CnV = tpm_mod / 2;
	// Update global variable
	StepperZFreq = freq;
}

// Set TPMs duty cycle to 0
void SetMotorsOff(void){
	// Set channel value for 0% duty cycle on Stepper X
	TPM0->CONTROLS[0].CnV = 0;
	// Set channel value for 0% duty cycle on Stepper Y
	TPM1->CONTROLS[0].CnV = 0;
	// Set channel value for 0% duty cycle on Stepper Z
	TPM2->CONTROLS[0].CnV = 0;
}

// Set TPM duty cycle to 50%
void SetMotorsOn(void){
	// Set channel value for 50% duty cycle on Stepper X
	TPM0->CONTROLS[0].CnV = StepperXFreq / 2;
	// Set channel value for 50% duty cycle on Stepper Y
	TPM1->CONTROLS[0].CnV = StepperYFreq / 2;
	// Set channel value for 50% duty cycle on Stepper Z
	TPM2->CONTROLS[0].CnV = StepperZFreq / 2;
}

// GPIO Management for direction set
static void SetPinHigh(GPIO_Type *port, uint32_t pin){
	port->PDOR |= pin;
}

static void SetPinLow(GPIO_Type *port, uint32_t pin){
	port->PDOR &= ~pin;
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

static void GPIO_Configure(PORT_Type *port, uint32_t pin){
	PortConfigure(port);
	port->PCR[pin] = 0x100;

	if(port == PORTA){
		GPIOA->PDDR |= ToBinaryGPIO(pin);    // Set as output
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

static void PortConfigure(PORT_Type *port){
	if(port == PORTA){
		SIM->SCGC5 |= 0x200;   // Enable clock port A
	}else if (port == PORTB){
		SIM->SCGC5 |= 0x400;   // Enable clock port B
	}else if (port == PORTC){
		SIM->SCGC5 |= 0x800;   // Enable clock port C
	}else if (port == PORTD){
		SIM->SCGC5 |= 0x1000;  // Enable clock port D
	}else{
		SIM->SCGC5 |= 0x2000;  // Enable clock port E
	}
}

static uint32_t ToBinaryGPIO(uint32_t num){
	return (1 << num);
}
