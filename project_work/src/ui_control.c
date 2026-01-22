/**
 * @file ui_control.c
 * @brief Contains all the code related to the UI of the system.
 */


/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Xilinx includes. */
#include "xil_printf.h"
#include "xparameters.h"
#include "xil_types.h"

#include "controller.h"
#include "ui_control.h"
#include "system_params.h"

/* LUT includes. */
#include "zynq_registers.h"

// LED Definition masks for UI feedback of system mode (M.H)
#define LED_MODE_CONFIG 	0X01	// LED0
#define LED_MODE_IDLE 		0X02   	// LED1
#define LED_MODE_MODULATION 0X04 	// LED2 ? tarkista onko ok led 2 tuolla 0x04

// global vartiable to ttrack current system mode (R.M)
volatile SystemMode_t current_system_mode = MODE_CONFIG;

// Degub variable:
float tgt = 0;

void ui_control_task(void *pvParameters){

	const TickType_t xInterval = pdMS_TO_TICKS(ui_interval);
	TickType_t xLastWakeTime;
	uint32_t ulNotificationValue;

	xLastWakeTime = xTaskGetTickCount();

	// local variable to hold system mode
	SystemMode_t ui_local_mode = MODE_CONFIG;
	SystemMode_t previous_mode = MODE_CONFIG;

	AXI_LED_TRI = 0; // Set all LEDs as output

	for(;;){

		// check if notify from button ISR
        if (xTaskNotifyWait(0x00, 0xFFFFFFFF, &ulNotificationValue, 0) == pdTRUE) {

        	if (ulNotificationValue & 0x01) {
                // Button 0 - mode change
                current_system_mode = (SystemMode_t)((current_system_mode + 1) % 3);
                xil_printf("System mode changed to: %d\r\n", current_system_mode);
            }

        	if (ulNotificationValue & 0x02) {
        		// Button 1 - change parameter - NOT YET IMPLEMENTED
        		xil_printf("Button 1 pressed");
        	}

            if (ulNotificationValue & 0x04) {
                // Button 2 - increase voltage
                if (current_system_mode == MODE_MODULATION) {
                    increaseTargetVoltage(10);
                    xil_printf("Target voltage increased by 10\r\n");
                }
            }

            if (ulNotificationValue & 0x08) {
                // Button 3 - decrease voltage
                if (current_system_mode == MODE_MODULATION) {
                    decreaseTargetVoltage(10);
                    xil_printf("Target voltage decreased by 10\r\n");
                }
            }
        }

		// Copy global mode to local mode
		ui_local_mode = current_system_mode;

		// switch case to handle different modes
		if (ui_local_mode != previous_mode) {
			switch(ui_local_mode){
	
				case MODE_CONFIG:
	
					// Stop controller
					// Debug:
					tgt = 0;
					setTargetVoltage(tgt);
					// vTaskSuspend(control_task_handle);
					// xil_printf("Configuration Mode\r\n");
					AXI_LED_DATA = LED_MODE_CONFIG; // Turn on LED0
					break;
	
				case MODE_IDLE:
	
					// Stop controller
					// Debug:
					tgt = 0;
					setTargetVoltage(tgt);
					// vTaskSuspend(control_task_handle);
					// xil_printf("Idle Mode\r\n");
					AXI_LED_DATA = LED_MODE_IDLE; // Turn on LED1
					break;
	
				case MODE_MODULATION:
	
					// Debug:
					// SOMEBODY PLEASE IMPLEMENT THIS!!!
					// tgt = 100;
					// setTargetVoltage(tgt);
					// Resume controller and model task
					// vTaskResume(plant_model_task_handle);
					// vTaskResume(control_task_handle);
					// xil_printf("Modulation Mode\r\n");
					AXI_LED_DATA = LED_MODE_MODULATION; // Turn on LED2
					break;
				}
			previous_mode = ui_local_mode;		
		}

	vTaskDelayUntil(&xLastWakeTime, xInterval);

	}

}
