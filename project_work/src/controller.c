/**
 * @file controller.c
 * @brief Contains all the code related to the PI controller.
 */


#include "controller.h"
#include "plant.h"
/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Xilinx includes. */
#include "xil_printf.h"
#include "xparameters.h"
#include "xil_types.h"
#include "system_params.h"
/* LUT includes. */
#include "zynq_registers.h"
#include "system_params.h"

TickType_t xTaskGetTickCount( void );

float Ki;
float Kp;

float u_old = 0;

void PI_controller(float u_ref) {

	// Source: FreeRTOS_Reference_Manual_V10.0.0.pdf pages 52 & 93

	TickType_t xLastWakeTime, xTime1, xTime2, xExecutionTime;

	/* Get the time the function started. */
	const TickType_t xPeriod = pdMS_TO_TICKS(excecution_interval);

	xLastWakeTime = xTaskGetTickCount();

	// Necessary forever loop. A thread should never be able to exit!
	for( ;; ) { // Same as while(1) or while(true)
		xTime1 = xTaskGetTickCount();
		xil_printf( "Control Loop Interval: %d \r\n", xTime1);

		/* Insert PI Controller Here! */

		// u_meas = plant_response(u1);
		// error = u_meas - u_ref

		// u1 = PI(error);

		vTaskDelayUntil(&xLastWakeTime, xPeriod);

	}
}

