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

#include "ui_control.h"
#include "system_params.h"

/* LUT includes. */
#include "zynq_registers.h"


void ui_control_task(void *pvParameters){

	int state = 0;
	for(;;){

		switch(state){

			case 0:

				xil_printf( "State 0 Looped!\r\n" );
				break;

			case 1:

				xil_printf( "State 1 Looped!\r\n" );
				break;

			case 3:

				xil_printf( "State 2 Looped!\r\n" );
				break;

		}
	}


}
