#include "CircuitVision.h"

// Task handlers
TaskHandle_t BlinkBlue_Handle = NULL;
TaskHandle_t ConsoleWriteHandle = NULL;
TaskHandle_t MenuDisplay_Handle = NULL;
TaskHandle_t ADC_Sampling_Handle = NULL;
TaskHandle_t ManualDisplay_Handle = NULL;
TaskHandle_t ManualMode_Handle = NULL;
// Queues
QueueHandle_t xJoystickQueue;
QueueHandle_t yJoystickQueue;

// Global variables
char tx_buff[UART_BUFFER_SIZE];
uint8_t operation_mode = MANUAL;

//////////////////// TASK FUNCTIONS ////////////////////
// Manual Mode
void ManualMode(void *pvParameters){
	uint16_t xJoystick, yJoystick;
	uint8_t xMotorDirection, yMotorDirection;
	int xMotorPercentage, yMotorPercentage;

	for(;;){
		// Only operate this task if mode is manual
		if(operation_mode == MANUAL){
			// Receive X axis Joystick
			if(xQueueReceive(xJoystickQueue, &xJoystick, 0) == pdPASS){
				// Convert to a range 0 - 100 in each direction
				if(xJoystick <= 48700){
					xMotorPercentage = (xJoystick - 48800) * (-1) / 482;
					StepperX_Direction(COUNTERCLOCKWISE);
				}else{
					xMotorPercentage = (xJoystick - 48800) / 169;
					StepperX_Direction(CLOCKWISE);
				}
			}
			// Receive Y axis Joystick
			if(xQueueReceive(yJoystickQueue, &yJoystick, 0) == pdPASS){
				yMotorPercentage = (yJoystick - 40400) * (-1) / 402;
			}
			// RUN MOTORS with calculated values
			snprintf(tx_buff, sizeof(tx_buff), "Motors x: %d, y: %d\n", xMotorPercentage, yJoystick);
			StepperX_RunPercentage(xMotorPercentage);

		}else{
			// If operation mode is not manual suspend this task
			vTaskSuspend(NULL);
		}

		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

// ADC Sampling
void ADC_Sampling(void *pvParameters) {
	// ADC results
	uint16_t adc_result, adc_result2;
    // Start ADC sampling in channel 0
    ADC0->SC1[0] = 0;
    for (;;) {
    	// Handle sampling in ADC 0 channel 0
    	 if (ADC0->SC1[0] & ADC_SC1_COCO_MASK) {
    		 adc_result = ADC0->R[0];
    		 // Start ADC conversion in channel 4
    		 ADC0->SC1[0] = 0x04;
    	 }
    	 vTaskDelay(pdMS_TO_TICKS(1));
    	 // Handle sampling in ADC 0 channel 0
    	 if (ADC0->SC1[0] & ADC_SC1_COCO_MASK) {
			 adc_result2 = ADC0->R[0];
			 // Start ADC conversion in channel 0
			 ADC0->SC1[0] = 0;
		 }
    	 //snprintf(tx_buff, sizeof(tx_buff), "ADC result: %d, %d\n", adc_result, adc_result2);
    	 // Send data to queues
    	 xQueueSendToBack(xJoystickQueue, &adc_result, 1);
    	 xQueueSendToBack(yJoystickQueue, &adc_result2, 1);
         // Delay
         vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Display Manual Mode
void ManualDisplay(void *pvParameters){
	// Print content and suspend itself
	for (;;){
		if(operation_mode == MANUAL){
			LCD_print_1st_line("  Manual Mode");
			LCD_print_2nd_line("             ");
			vTaskDelay(pdMS_TO_TICKS(2000));
			LCD_print_1st_line("  Use Joystick");
			vTaskResume(ManualMode_Handle);
			vTaskSuspend(NULL);;
		}
	}
}

// Display in LCD Menu
void InspectorMenuDisplay(void *pvParameters){
	for(;;){
		LCD_print_1st_line(" Circuit Vision");
		LCD_print_2nd_line("      MENU");
		vTaskDelay(pdMS_TO_TICKS(1500));
		//LCD_clear();
		LCD_print_1st_line("    Press key");
		LCD_print_2nd_line("              ");
		vTaskDelay(pdMS_TO_TICKS(1500));
		LCD_print_1st_line("A-Automatic Mode");
		LCD_print_2nd_line("B-Manual Mode");
		vTaskDelay(pdMS_TO_TICKS(3000));
		vTaskResume(ManualDisplay_Handle);
		vTaskSuspend(NULL);
	}
}

// Write to console through UART
void ConsoleWrite(void *pvParameters){
	for(;;){
		UART_Send();
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}

// Blink Blue LED
void BlinkBlue(void *pvParameters){
	for(;;){
		BlueToggle();
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

////////////////////////////////////////////////////////

// Tasks Creation
void CreateTasks(void){
	// Queues
	xJoystickQueue = xQueueCreate(1, sizeof(uint16_t));
	yJoystickQueue = xQueueCreate(1, sizeof(uint16_t));

	// MODES
	xTaskCreate(ManualMode, "ManualModeTask", 512, NULL,
								tskIDLE_PRIORITY + 1, &ManualMode_Handle);
	vTaskSuspend(ManualMode_Handle);

	// Manual Mode Display
	xTaskCreate(ManualDisplay, "ManualDisplayTask", 100, NULL,
								tskIDLE_PRIORITY + 1, &ManualDisplay_Handle);
	vTaskSuspend(ManualDisplay_Handle);

	// Main Menu (not doing anything)
	xTaskCreate(InspectorMenuDisplay, "MenuDisplayTask", 64, NULL,
							    tskIDLE_PRIORITY + 1, &MenuDisplay_Handle);
	// UART Task
	xTaskCreate(ConsoleWrite, "ConsoleWriteTask", 16, NULL,
								tskIDLE_PRIORITY + 2, &ConsoleWriteHandle);

	// ADC Processing task
	xTaskCreate(ADC_Sampling, "ADC_SamplingTask", 150, NULL,
							    tskIDLE_PRIORITY + 2, &ADC_Sampling_Handle);

	// Blink blue LED task
	xTaskCreate(BlinkBlue, "BlinkBlueTask", 8, NULL,
								tskIDLE_PRIORITY + 1, &BlinkBlue_Handle);
	//vTaskSuspend(BlinkBlue_Handle);
}

////////////////// BARE METAL CONFIG //////////////////


//////////////////// MAIN FUNCTION ////////////////////
int main(void) {
	////////// Configurations //////////
	RGB_Init();
	UART0_Config(interrupt_rx, baud4);
	ADC_Config();
	SteppersInit();
	// LCD I2C Init
	LCD_Init_Config(I2C_HANDLE);
	// UART set buffers
	SetTX_Buffer(tx_buff, UART_BUFFER_SIZE);
	//////////// FreeRTOS ////////////
	// Tasks creation
	CreateTasks();
	// Give program control to scheduler
	vTaskStartScheduler();

	// Should never reach here
	for(;;);
    return 0 ;
}
