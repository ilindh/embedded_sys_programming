/*
    FreeRTOS V8.2.1 - Copyright (C) 2015 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    1 tab == 4 spaces!

    ***************************************************************************
     *                                                                       *
     *    Having a problem?  Start by reading the FAQ "My application does   *
     *    not run, what could be wrong?".  Have you defined configASSERT()?  *
     *                                                                       *
     *    http://www.FreeRTOS.org/FAQHelp.html                               *
     *                                                                       *
    ***************************************************************************

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    ***************************************************************************
     *                                                                       *
     *   Investing in training allows your team to be as productive as       *
     *   possible as early as possible, lowering your overall development    *
     *   cost, and enabling you to bring a more robust product to market     *
     *   earlier than would otherwise be possible.  Richard Barry is both    *
     *   the architect and key author of FreeRTOS, and so also the world's   *
     *   leading authority on what is the world's most popular real time     *
     *   kernel for deeply embedded MCU designs.  Obtaining your training    *
     *   from Richard ensures your team will gain directly from his in-depth *
     *   product knowledge and years of usage experience.  Contact Real Time *
     *   Engineers Ltd to enquire about the FreeRTOS Masterclass, presented  *
     *   by Richard Barry:  http://www.FreeRTOS.org/contact
     *                                                                       *
    ***************************************************************************

    ***************************************************************************
     *                                                                       *
     *    You are receiving this top quality software for free.  Please play *
     *    fair and reciprocate by reporting any suspected issues and         *
     *    participating in the community forum:                              *
     *    http://www.FreeRTOS.org/support                                    *
     *                                                                       *
     *    Thank you!                                                         *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org - Documentation, books, training, latest versions,
    license and Real Time Engineers Ltd. contact details.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!

    Modified by lindh LUT
*/

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Xilinx includes. */
#include "xil_printf.h"
#include "xparameters.h"
#include "xil_types.h"

// Custom header files:
#include "controller.h"
#include "plant.h"
#include "ui_control.h"
#include "system_params.h"
#include "zynq_registers.h"


SemaphoreHandle_t control_out_MUTEX;
SemaphoreHandle_t u_out_plant_MUTEX;


int main( void ) {

	  // AXI_BTN_TRI |= 0xF; 		// Set direction for buttons 0..3 ,  0 means output, 1 means input
      AXI_LED_TRI = ~0xF;		// Set direction for bits 0-3 to output for the LEDs

      // Create MUTEX instances.
      control_out_MUTEX = xSemaphoreCreateMutex();
      u_out_plant_MUTEX = xSemaphoreCreateMutex();

	xil_printf( "Control System starting...\r\n" );

	/**
	 * Create four tasks t
	 * Each function behaves as if it had full control of the controller.
	 * https://www.freertos.org/a00125.html
	 */
	xTaskCreate(control_task, 					// The function that implements the task.
					"Controller loop", 			// Text name for the task, provided to assist debugging only.
					4096, 						// The stack allocated to the task.
					NULL, 						// The task parameter is not used, so set to NULL.
					tskIDLE_PRIORITY+3,			// The task runs at the idle priority. Higher number means higher priority.
					NULL );

	xTaskCreate(plant_model_task, 					// The function that implements the task.
					"Plant model loop", 		// Text name for the task, provided to assist debugging only.
					4096, 						// The stack allocated to the task.
					NULL, 						// The task parameter is not used, so set to NULL.
					tskIDLE_PRIORITY+2,			// The task runs at the idle priority. Higher number means higher priority.
					NULL );

	xTaskCreate(ui_control_task, 					// The function that implements the task.
					"UI control loop", 			// Text name for the task, provided to assist debugging only.
					4096, 						// The stack allocated to the task.
					NULL, 						// The task parameter is not used, so set to NULL.
					tskIDLE_PRIORITY+1,			// The task runs at the idle priority. Higher number means higher priority.
					NULL );

	// Start the tasks and timer running.
	// https://www.freertos.org/a00132.html
	vTaskStartScheduler();

	for( ;; );
}
