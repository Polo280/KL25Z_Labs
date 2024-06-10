#ifndef STEPPER_H
#define STEPPER_H

/////////// Includes ///////////
#include "MKL25Z4.h"
//////////// MACROS ////////////
#define TPM_FREQUENCY 1311120
// Ports
#define STEPPERX_PORT PORTD
#define STEPPERY_PORT PORTA
#define STEPPERZ_PORT PORTB
#define STEPPERX_DIR_PORT PORTD
#define STEPPERY_DIR_PORT PORTA
#define STEPPERZ_DIR_PORT PORTD
// Pins
#define STEPPERX_PIN 0
#define STEPPERY_PIN 12
#define STEPPERZ_PIN 2
#define STEPPERX_DIR_PIN 5
#define STEPPERY_DIR_PIN 5
#define STEPPERZ_DIR_PIN 4
// Direction
#define CLOCKWISE 1
#define COUNTERCLOCKWISE 0
// Resolution for setting the speed
#define STEPPER_MAX_FREQUENCY_HZ 200

/////////// Variables //////////
extern uint16_t StepperXFreq;
extern uint16_t StepperYFreq;
extern uint16_t StepperZFreq;

////////// Functions  //////////
// TPM Configuration
void TPM0_Init(void);
void TPM1_Init(void);
void TPM2_Init(void);
void AllTPMsInit(void);
// Set the direction for the steppers
void StepperX_Direction(uint8_t);
void StepperY_Direction(uint8_t);
void StepperZ_Direction(uint8_t);
// Run Stepper to a certain frequency
void StepperX_RunFreq(uint16_t);
void StepperY_RunFreq(uint16_t);
void StepperZ_RunFreq(uint16_t);
// Run Stepper to a certain velocity percentage
void StepperX_RunPercentage(uint8_t);
void StepperY_RunPercentage(uint8_t);
void StepperZ_RunPercentage(uint8_t);
// Individual offs
void StepperX_Off(void);
void StepperY_Off(void);
void StepperZ_Off(void);
// General Motor Control
void SteppersInit(void);
void SetMotorsOff(void);
void SetMotorsOn(void);
void RunMotorsPercentage(uint8_t);

#endif /*STEPPER_H*/
