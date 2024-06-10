#ifndef CIRCUITVISION_H
#define CIRCUITVISION_H

/////////// Includes ///////////
#include <stdio.h>
#include "MKL25Z4.h"
// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
// Custom Libraries
#include "Libraries/Headers/ADC.h"
#include "Libraries/Headers/RGB.h"
#include "Libraries/Headers/UART.h"
#include "Libraries/Headers/I2C_LCD.h"
#include "Libraries/Headers/StepperTPM.h"
#include "Libraries/Headers/PIT_Config.h"

//////////// MACROS ////////////
#define SAFE_STACK_SIZE 256
#define UART_BUFFER_SIZE 50

/////////// Variables //////////
// Displays
extern TaskHandle_t MenuDisplay_Handle;
extern TaskHandle_t ManualDisplay_Handle;
// ADC sampling
extern TaskHandle_t ADC_Sampling_Handle;
// LED handles
extern TaskHandle_t BlinkGreen_Handle;
extern TaskHandle_t BlinkBlue_Handle;
// UART
extern TaskHandle_t ConsoleWriteHandle;
// Operation Modes
extern TaskHandle_t ManualMode_Handle;

///////// Queues /////////
extern QueueHandle_t xJoystickQueue;
extern QueueHandle_t yJoystickQueue;

// Global variables
extern char tx_buff[UART_BUFFER_SIZE];
extern uint8_t operation_mode;

enum OperationMode{
	MANUAL = 0,
	AUTOMATIC = 1,
	STOP = 2
};

////////// Functions  //////////
// Functionalities
void CreateTasks(void);
void ConsoleWrite(void *);
void InspectorMenuDisplay(void *);
void ADC_Sampling(void *);
// Modes
void ManualMode(void *);
// LEDs
void BlinkGreen(void *);
void BlinkBlue(void *);
// Bare Metal Configuratuions
void SteppersInit(void);

#endif /*CIRCUITVSION_H*/
