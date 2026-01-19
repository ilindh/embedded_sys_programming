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

// Variables:

// Step size for integration. Mathced with "sampling interval"
float h = (float)controller_interval / 1000.0;

TickType_t xTaskGetTickCount(void);

// Global variables for input and output.
volatile float u_out_controller;

static int print_interval = 1000;
static int i_print = 0;

// Static target voltage variable
volatile float u_ref = 0;

// increase target voltage function - R.M.
/// @brief This function increases the target voltage by a specified step.
/// @param step The amount by which to increase the target voltage.
void increaseTargetVoltage(float step){
	setTargetVoltage(u_ref + step);
	return;
}
// decrease target voltage function - R.M.
/// @brief This function decreases the target voltage by a specified step.
/// @param step The amount by which to decrease the target voltage.
void decreaseTargetVoltage(float step){
	setTargetVoltage(u_ref - step);
	return;
}

/// @brief This function allows for setting the target voltage with MUTEXes implemented.
void setTargetVoltage(float new_target){

	if( xSemaphoreTake(u_ref_MUTEX, 5) == pdTRUE ) {
		/* The mutex was successfully obtained so the shared resource can be accessed safely. */

		// Range checking for the targetvoltage - R.M.
		if (new_target < 0) {
			new_target = 0;
		}
		else if (new_target > 400) {
			new_target = 400;
		}
		u_ref = new_target;
		xSemaphoreGive(u_ref_MUTEX);
		/* Access to the shared resource is complete, so the mutex is returned. */

	 } else {
		 /* The mutex could not be obtained even after waiting 5 ticks, so the shared resource cannot be accessed. */
		 xil_printf( "Error while setting the target voltage for controller.");
	 }
}

/// @brief This function allows for retrieving the data with MUTEXes implemented.
static float getCurrentVoltage(void){

	if(xSemaphoreTake(u_out_plant_MUTEX, 5) == pdTRUE ) {
		/* The mutex was successfully obtained so the shared resource can be accessed safely. */
		float current_u_out = u_out_plant;
		xSemaphoreGive(u_out_plant_MUTEX);
		return current_u_out;
		/* Access to the shared resource is complete, so the mutex is returned. */

	 } else {
		 /* The mutex could not be obtained even after waiting 5 ticks, so the shared resource cannot be accessed. */

		 xil_printf( "Error while retreiving the plant voltage.");
		 return u_ref;
	 }
}

/// @brief This function allows for retrieving the data with MUTEXes implemented.
static void setCurrentVoltage(float u_out){

	if( xSemaphoreTake(control_out_MUTEX, 5) == pdTRUE ) {
		/* The mutex was successfully obtained so the shared resource can be accessed safely. */
		u_out_controller = u_out;
		xSemaphoreGive(control_out_MUTEX);
		/* Access to the shared resource is complete, so the mutex is returned. */

	 } else {
		 /* The mutex could not be obtained even after waiting 5 ticks, so the shared resource cannot be accessed. */
		 xil_printf( "Error while setting the controller output.");
	 }
}



/// @brief This is the Controller Task function
void control_task(void *pvParameters) {

	TickType_t xLastWakeTime;
	const TickType_t xInterval = pdMS_TO_TICKS(controller_interval);

	// TEMPORARY fixed input.
	float Kp = 10;
	float Ki = 5;
	float Kd = 0.00;

	xLastWakeTime = xTaskGetTickCount();

	// Necessary forever loop. A thread should never be able to exit!
	for( ;; ) { // Same as while(1) or while(true)

		float u_meas = getCurrentVoltage();

		setCurrentVoltage(PI_controller(u_meas, u_ref, Kd, Ki, Kp));

		// Print only after print_interval
		if(i_print == print_interval){

			i_print = 0;
			/*
			xil_printf("\n");
			xil_printf( "Control Loop Interval: %d \r\n", (int)xLastWakeTime);
			xil_printf( "Target Voltage: %d \r\n", (int)u_ref);
			xil_printf( "Converter voltage [V]: %d \r\n", (int)u_meas);
			xil_printf( "PI output [V]: %d \r\n", (int)u_out_controller); */

			xil_printf("\n");
			xil_printf( "Int: %d | Tgt: %d | PI: %d | Plant: %d \n\r", (int)xLastWakeTime, (int)u_ref, (int)u_out_controller, (int)u_meas);
		}

		i_print++;

		vTaskDelayUntil(&xLastWakeTime, xInterval);
	}

}

/// @brief This is the PID controller function
/// @param Kp (proportional), Ki (integrative), Kd (derivative), ref (?), y (?) h (?)
/// @return PI controller output
float PI_controller(float u_meas, float u_ref, float Kd, float Ki, float Kp) {

	static float err, err_prev, yi_prev, yd_prev;

	//m��rit� viel� tarkempi arvo!!
	static float windupLimit = 200;
	static float yp, yi, yd, PI_out;

	float u_max = 400;
	float u_min = 0;

    //  err = ref � y;
	err = u_ref-u_meas; // Calculate the error value

	// Calculate yp, yi ja yd
   	yp = Kp*err;
   	yi = Ki*(h/2)*(err+err_prev) + yi_prev;
  	yd = Kd*(err-err_prev) / h;


	// Anti-winding integraattorille (https://codepal.ai/code-generator/query/MjweSyOx/pid-regulator-with-anti-windup)
	if (yi > windupLimit) {
    		yi = windupLimit;
	}
	 else if (yi < -windupLimit) {
   		 yi = -windupLimit;
	}

	// Saturate the output of the controller
	float unsat_out = yp + yi + yd;

	PI_out = unsat_out; //toimiiko if tsydeemi t�ll� en oo yht�� varma??

	if (PI_out > u_max){
		PI_out = u_max;
	}else if(PI_out < u_min){
		PI_out = u_min;
	}

	// Update the old values
   	yi_prev = yi;
	yd_prev = yd;
	err_prev = err;

	return PI_out;

}


void PWM_control(void){


}



