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

#include "timers.h"

// LED Definition masks for UI feedback of system mode (M.H)
#define LED_MODE_CONFIG 0X01	 // LED0
#define LED_MODE_IDLE 0X02		 // LED1
#define LED_MODE_MODULATION 0X04 // LED2

// global vartiable to ttrack current system mode (R.M)
static volatile SystemMode_t current_system_mode = MODE_CONFIG;
// local variable to hold system mode
static volatile SystemMode_t ui_local_mode = MODE_CONFIG;
static volatile SystemMode_t previous_mode = MODE_CONFIG;

// Variable that captures the button ISR value!
static uint32_t ulNotificationValue;

// Target voltage for step changes
static float tgt = 0;


// Claude AI was used to help in figuring out the implementation of system mode changes and these getSystemMode and setSystemMode
// Also, the base for buttons got help from Claude AI. Implementation is by -R.M.
/// @brief This function allows for retrieving the data with MUTEXes implemented.
SystemMode_t getSystemMode(void)
{
	if (xSemaphoreTake(sys_mode_MUTEX, 5) == pdTRUE)
	{
		/* The mutex was successfully obtained so the shared resource can be accessed safely. */
		xSemaphoreGive(sys_mode_MUTEX);
		return current_system_mode;
		/* Access to the shared resource is complete, so the mutex is returned. */
	}
	else
	{
		/* The mutex could not be obtained even after waiting 5 ticks, so the shared resource cannot be accessed. */

		xil_printf("Error while retreiving the system mode.");
		// Return IDLE if retreiving system mode fails!
		return 1;
	}
}

/// @brief This function allows for retrieving the data with MUTEXes implemented.
void setSystemMode(SystemMode_t new_sys_mode)
{
	if (xSemaphoreTake(sys_mode_MUTEX, 5) == pdTRUE)
	{
		/* The mutex was successfully obtained so the shared resource can beaccessed safely. */
		current_system_mode = new_sys_mode;
		// Set also the LOCAL !
		ui_local_mode = new_sys_mode;
		xSemaphoreGive(sys_mode_MUTEX);
		/* Access to the shared resource is complete, so the mutex is returned. */
	}
	else
	{
		/* The mutex could not be obtained even after waiting 5 ticks, so the shared resource cannot be accessed. */
		xil_printf("Error while setting the system mode.");
	}
}


/// @brief When TIMER semaphore is taken, it starts a 5s timer with FreeRTOS function.
// Timer calls automatically the timer callback that releases the sempahore.
// Claude AI was used to help in figuring out the caveats for this timed MUTEX. Implementation is by -I.L.
// BaseType_t is the FreeRTOS "base" type for messaging if a function was success or not (pdTRUE / pdFALSE).
// If the timed mutex is taken and another task tries to use it, zero wait time but FALSE is returned.
BaseType_t cooldown_semaphore_take(void){

	if (xSemaphoreTake(cooldown_SEMAPHORE, 0) == pdTRUE){
		/* The mutex was successfully obtained so the shared resource can beaccessed safely. */
		return pdTRUE;
		// Semaphore taken!
		/* Access to the shared resource is complete, so the mutex is returned. */
	} else {
		// xil_printf("Error fetching the timer semaphore.");
		return pdFALSE;
	}
}

// Callback function for the FreeRTOS timer instance.
// This gives the cooldown semaphore after 5s time interval (if not resetted.)
void cooldown_timer_callback(TimerHandle_t cooldown_timer){
	xSemaphoreGive(cooldown_SEMAPHORE);
	return;
}


void Button_Handler(void)
{
	// Check if UART in config mode, if UART is in config mode, then buttons are disabled -> not handled -> return
	if (xSemaphoreTake(uart_config_SEMAPHORE, 0) == pdFALSE)
	{
		xTaskNotifyWait(0x00, 0xFFFFFFFF, &ulNotificationValue, 0);
		return;
	}
	// release the semaphore immediately
	xSemaphoreGive(uart_config_SEMAPHORE);

	// IF THERE IS A BUTTON INTERRUPT ACTIVE:
	// NotificationValue contains the button that has caused the interrupt.
	// Change to this tasknotify method allows for the button ISR to be shorter, suggestion for this implementation came from Claude AI. Implementation is by -R.M.
	if (xTaskNotifyWait(0x00, 0xFFFFFFFF, &ulNotificationValue, 0) == pdTRUE)
	{
		// IF parameter semaphore is not taken, we can change params.
		
		if(cooldown_semaphore_take() == pdFALSE){
			xTimerReset(cooldown_timer, 0);
			//xil_printf("\r\nDBG: Button cooldown resetted... \r\n");
		} else {
			// From FreeRTOS_Reference_Manual_V10.0.0.pdf -I.L.
			if(xTimerStart(cooldown_timer, 0 ) == pdPASS){
				/* The timer could not be set into the Active state. */
				//xil_printf("\r\nDBG: Button 5s cooldown started... \r\n");
			} else {
				xil_printf("Error starting the UART block timer.");
			}
		}
		
		// IF BUTTON "0" IS PRESSED
		// WE CHANGE SYSTEM MODE:
		if (ulNotificationValue & 0x01){
			// Button 0 - mode change
			// Modulo "%" allows for looping and prevents overflow.
			// We set the LOCAL version of the system mode. This is only used in this file.
			// The local Should be in Sync with the "global" system mode, and through this
			// the "global" system-mode is also updated!
			setSystemMode((SystemMode_t)((ui_local_mode + 1) % 3));
			xil_printf("\r\n\n");
			xil_printf("System mode changed to: ");

		// ELSE WE OPERATE INSIDE THE MODES:
		}	else	{

			switch (ui_local_mode)
			{

			case MODE_MODULATION:
				/* BUTTON "1" */
				// Set step target voltage to 400V - set_voltage_tgt from system_params.h
				if (ulNotificationValue & 0x02)
				{
					setTargetVoltage(step_voltage_tgt);
					xil_printf("\r\n");
					xil_printf("Target voltage set to %d V!\r\n", step_voltage_tgt);
				}
				/* BUTTON "2" */
				// Increase target voltage by 10V
				else if (ulNotificationValue & 0x04)
				{
					increaseTargetVoltage(10);
					xil_printf("\r\nTarget voltage increased by: +10V!\r\n");
				}
				/* BUTTON "3" */
				// Decrease target voltage by 10V
				else if (ulNotificationValue & 0x08)
				{
					decreaseTargetVoltage(10);
					xil_printf("\r\nTarget voltage decreased by: -10V!\r\n");
				}
				break;

			case MODE_CONFIG:
				/* BUTTON "1" */
				// toggle selected parameter (Kp, Ki, Kd)
				if (ulNotificationValue & 0x02)
				{
					toggleParameter();
					ConfigParam_t param = getSelectedParameter();
					if (param == PARAM_KP)
					{
						xil_printf("\r\nSelected parameter: [Kp]\r\n");
					}
					else if (param == PARAM_KI)
					{
						xil_printf("\r\nSelected parameter: [Ki]\r\n");
					}
					else
					{
						xil_printf("\r\nSelected parameter: [Kd]\r\n");
					}
				}
				/* BUTTON "2" */
				// Increase selected parameter by 0.01
				else if (ulNotificationValue & 0x04)
				{
					increaseParameter(0.01);
				}
				/* BUTTON "3" */
				// Decrease selected parameter by 0.01
				else if (ulNotificationValue & 0x08)
				{
					decreaseParameter(0.01);
				}
				break;

			case MODE_IDLE:
				// In IDLE mode, buttons 1-3 do nothing
				break;
			}
		}
	}
}

void ui_control_task(void *pvParameters)
{

	const TickType_t xInterval = pdMS_TO_TICKS(ui_interval);
	TickType_t xLastWakeTime;

	xLastWakeTime = xTaskGetTickCount();

	AXI_LED_TRI = 0; // Set all LEDs as output
	AXI_LED_DATA = LED_MODE_CONFIG; // Start with CONFIG mode LED on

	UART_SendHelp(); // Send help message on startup

	for (;;)
	{

		// Process any UART input
		UART_ProcessInput();

		// check if notify from button ISR
		Button_Handler();

		// switch case to handle different modes
		if (ui_local_mode != previous_mode)
		{
			switch (ui_local_mode)
			{

			case MODE_CONFIG:
				// configuration mode actions, e.g., enable parameter tuning
				// target voltage set to 0V
				xil_printf("CONFIG \r\n");
				xil_printf("\r\nSelected parameter: [Kp]\r\n");
				tgt = 0;
				setTargetVoltage(tgt);
				AXI_LED_DATA = LED_MODE_CONFIG; // Turn on LED0
				break;

			case MODE_IDLE:
				// idle mode actions, e.g., disable modulation
				// target voltage set to 0V
				xil_printf("IDLE \r\n");
				tgt = 0;
				setTargetVoltage(tgt);
				AXI_LED_DATA = LED_MODE_IDLE; // Turn on LED1
				break;

			case MODE_MODULATION:
				// modulation mode actions, e.g., enable modulation
				xil_printf("MODULATION \r\n");
				AXI_LED_DATA = LED_MODE_MODULATION; // Turn on LED2
				break;
			}
			previous_mode = ui_local_mode;
		}

		vTaskDelayUntil(&xLastWakeTime, xInterval);
	}
}
