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
#include "uart_ui.h"
#include "system_params.h"

/* LUT includes. */
#include "zynq_registers.h"
#include <stdbool.h>

// LED Definition masks for UI feedback of system mode (M.H)
#define LED_MODE_CONFIG 	0X01	// LED0
#define LED_MODE_IDLE 		0X02   	// LED1
#define LED_MODE_MODULATION 0X04 	// LED2

// global vartiable to ttrack current system mode (R.M)
static volatile SystemMode_t current_system_mode = MODE_CONFIG;
// local variable to hold system mode
static volatile SystemMode_t ui_local_mode = MODE_CONFIG;
static volatile SystemMode_t previous_mode = MODE_CONFIG;

// A flag used to control if modulation print is active or not
volatile int print_modulation = 0;
// A flag used to control if UART UI print is active or not
volatile int print_uart_ui = 0;

// Variable that captures the button ISR value!
static uint32_t ulNotificationValue;

// Degub variable:
float tgt = 0;

/// @brief This function allows for retrieving the data with MUTEXes implemented.
SystemMode_t getSystemMode(void){

	if( xSemaphoreTake(sys_mode_MUTEX, 5) == pdTRUE ) {
		/* The mutex was successfully obtained so the shared resource can beaccessed safely. */
		xSemaphoreGive(sys_mode_MUTEX);
		return current_system_mode;
		/* Access to the shared resource is complete, so the mutex is returned. */

	 } else {
		 /* The mutex could not be obtained even after waiting 5 ticks, so the shared resource cannot be accessed. */

		xil_printf( "Error while retreiving the system mode.");
		// Return IDLE if retreiving system mode fails!
		return 1;
	 }
}

/// @brief This function allows for retrieving the data with MUTEXes implemented.
void setSystemMode(SystemMode_t new_sys_mode){

	if( xSemaphoreTake(sys_mode_MUTEX, 5) == pdTRUE ) {
		/* The mutex was successfully obtained so the shared resource can beaccessed safely. */
		current_system_mode = new_sys_mode;
		// Set also the LOCAL !
		ui_local_mode = new_sys_mode;
		xSemaphoreGive(sys_mode_MUTEX);
		/* Access to the shared resource is complete, so the mutex is returned. */

	 } else {
		 /* The mutex could not be obtained even after waiting 5 ticks, so the shared resource cannot be accessed. */
		 xil_printf("Error while setting the system mode.");
	 }
}

void Button_Handler(void){

	// Check if UART in config mode
	if (xSemaphoreTake(uart_config_SEMAPHORE, 0) == pdFALSE) {
		xTaskNotifyWait(0x00, 0xFFFFFFFF, &ulNotificationValue, 0);
		return;
	}
	// release the semaphore immediately
	xSemaphoreGive(uart_config_SEMAPHORE);

	// IF THERE IS A BUTTON INTERRUPT ACTIVE:
	// NotificationValue contains the button that has caused the interrupt.
	if (xTaskNotifyWait(0x00, 0xFFFFFFFF, &ulNotificationValue, 0) == pdTRUE) {

		// IF BUTTON "0" IS PRESSED
		// WE CHANGE SYSTEM MODE:
		if (ulNotificationValue & 0x01) {
			// Button 0 - mode change
			// Modulo "%" allows for looping and prevents overflow.
			// We set the LOCAL version of the system mode. This is only used in this file.
			// The local Should be in Sync with the "global" system mode, and through this
			// the "global" system-mode is also updated!
			setSystemMode( (SystemMode_t)((ui_local_mode + 1) % 3) );
			xil_printf("\r\n\n");
			xil_printf("System mode changed to: ");

		// ELSE WE OPERATE INSIDE THE MODES:
		} else {

			switch(ui_local_mode){

				case MODE_MODULATION:
					/* BUTTON "1" */
					if (ulNotificationValue & 0x02) {
						setTargetVoltage(step_voltage_tgt);
						xil_printf("\r\n");
						xil_printf("Target voltage set to %d V!\r\n", step_voltage_tgt);

					/* BUTTON "2" */
					} else if (ulNotificationValue & 0x04){
						increaseTargetVoltage(10);
						xil_printf("\r\nTarget voltage increased by: +10V!\r\n");

					/* BUTTON "3" */
					} else if (ulNotificationValue & 0x08){
						decreaseTargetVoltage(10);
						xil_printf("\r\nTarget voltage decreased by: -10V!\r\n");
					} else {
						// Nothing
					}
					break;

				case MODE_CONFIG:
					/* BUTTON "1" */
					if (ulNotificationValue & 0x02) {
						toggleParameter();
						ConfigParam_t param = getSelectedParameter();
						if(param == PARAM_KP) {
							xil_printf("\r\nSelected parameter: [Kp]\r\n");
						}
						else if (param == PARAM_KI){
							xil_printf("\r\nSelected parameter: [Ki]\r\n");
						}
						else {
							xil_printf("\r\nSelected parameter: [Kd]\r\n");
						}
					
					/* BUTTON "2" */
					} else if (ulNotificationValue & 0x04){
						increaseParameter(0.01);
						ConfigParam_t param = getSelectedParameter();
						if(param == PARAM_KP) {
							// xil_printf("\r\nKp increased to: %d.%02d\r\n", (int)Kp, (int)((Kp - (int)Kp) * 100));
						}
						else if (param == PARAM_KI){
							// xil_printf("\r\nKi increased to: %d.%02d\r\n", (int)Ki, (int)((Ki - (int)Ki) * 100));
						}
						else {
							// xil_printf("\r\nKd increased to: %d.%02d\r\n", (int)Kd, (int)((Kd - (int)Kd) * 100));
						}
					/* BUTTON "3" */
					} else if (ulNotificationValue & 0x08){
						decreaseParameter(0.01);
						ConfigParam_t param = getSelectedParameter();

						if(param == PARAM_KP) {
							// xil_printf("Kp decreased to: %d.%02d\r\n", (int)Kp, (int)((Kp - (int)Kp) * 100));
						}
						else if (param == PARAM_KI){
							// xil_printf("Ki decreased to: %d.%02d\r\n", (int)Ki, (int)((Ki - (int)Ki) * 100));
						}
						else {
							// xil_printf("Kd decreased to: %d.%02d\r\n", (int)Kd, (int)((Kd - (int)Kd) * 100));
						}

					} else {
						xil_printf("\r\n\r\n");
					}

					break;

				case MODE_IDLE:
					/* BUTTON "1" */
					if (ulNotificationValue & 0x02) {
						// DO NOTHING
					/* BUTTON "2" */
					} else if (ulNotificationValue & 0x04){
						// DO NOTHING
					/* BUTTON "3" */
					} else if (ulNotificationValue & 0x08){
						// DO NOTHING
					} else {
						// Nothing
					}
					break;
			}
		}
	}

}

void ui_control_task(void *pvParameters){

	const TickType_t xInterval = pdMS_TO_TICKS(ui_interval);
	TickType_t xLastWakeTime;


	xLastWakeTime = xTaskGetTickCount();

	AXI_LED_TRI = 0; // Set all LEDs as output
	AXI_LED_DATA = LED_MODE_CONFIG;

	UART_SendHelp();

	for(;;){

		// Process any UART input
		UART_ProcessInput();

		// check if notify from button ISR
		Button_Handler();

		// Copy global mode to local mode
		// ui_local_mode = current_system_mode;
		
		// switch case to handle different modes
		if (ui_local_mode != previous_mode) {
			switch(ui_local_mode){
	
				case MODE_CONFIG:
					xil_printf("CONFIG \r\n");

					// xil_printf("\n\n");
					xil_printf("\r\nSelected parameter: [Kp]\r\n");
					// Stop controller
					// Debug:
					tgt = 0;
					setTargetVoltage(tgt);
					// vTaskSuspend(control_task_handle);
					// xil_printf("Configuration Mode\r\n");
					AXI_LED_DATA = LED_MODE_CONFIG; // Turn on LED0
					break;
	
				case MODE_IDLE:

					xil_printf("IDLE \r\n");
					// xil_printf("\n\n");
					// Stop controller
					// Debug:
					tgt = 0;
					setTargetVoltage(tgt);
					// vTaskSuspend(control_task_handle);
					// xil_printf("Idle Mode\r\n");
					AXI_LED_DATA = LED_MODE_IDLE; // Turn on LED1
					break;
	
				case MODE_MODULATION:

					xil_printf("MODULATION \r\n");
					// xil_printf("\n\n");
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
