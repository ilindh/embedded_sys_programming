/**
 * @file controller.c
 * @brief Contains all the code related to the PI controller.
 */



#include "controller.h"
#include "plant.h"
#include "timer_setup.h"
#include "ui_control.h"

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
#include <xttcps.h>
#include <sleep.h>
#include <stdint.h>

// Variables:

// Step size for integration. Mathced with "sampling interval"
float h = (float)controller_interval / 1000.0;

TickType_t xTaskGetTickCount(void);

// Global variables for input and output.
volatile float u_out_controller;

static const int print_interval = 500;
static volatile int i_print = 0;

// volatile target voltage variable
volatile float u_ref = 0;

// volatile variables for the Kp and Ki parameters
volatile float Kp = 4.5;
volatile float Ki = 6.0;
volatile float Kd = 0.01;

volatile ConfigParam_t selected_param = PARAM_KP;

void toggleParameter(void){
	selected_param = (ConfigParam_t)((selected_param +1) % 3);
}

ConfigParam_t getSelectedParameter(void){
	return selected_param;
}

// increase parameter function - R.M.
/// @brief This function increases the selected parameter (Kp/Ki) by a specified step.
/// @param step The amount by which to increase the selected parameter voltage.
void increaseParameter(float step) {
    if (selected_param == PARAM_KP) {
        if (xSemaphoreTake(u_ref_MUTEX, 5) == pdTRUE) {
            float new_kp = Kp + step;
            if (new_kp > 100.0) {
            	new_kp = 100.0;
            }
            Kp = new_kp;
            xSemaphoreGive(u_ref_MUTEX);
        }
    } else if (selected_param == PARAM_KI) {
        if (xSemaphoreTake(u_ref_MUTEX, 5) == pdTRUE) {
            float new_ki = Ki + step;
            if (new_ki > 100.0) {
            	new_ki = 100.0;
            }
            Ki = new_ki;
            xSemaphoreGive(u_ref_MUTEX);
        }
    } else if (selected_param == PARAM_KD) {
        if (xSemaphoreTake(u_ref_MUTEX, 5) == pdTRUE) {
            float new_kd = Kd + step;
            if (new_kd > 100.0) {
            	new_kd = 100.0;
            }
            Kd = new_kd;
            xSemaphoreGive(u_ref_MUTEX);
        }
    }
}
// decrease parameter function - R.M.
/// @brief This function decreases the selected parameter (Kp/Ki) by a specified step.
/// @param step The amount by which to decreases the selected parameter voltage.
void decreaseParameter(float step) {
    if (selected_param == PARAM_KP) {
        if (xSemaphoreTake(u_ref_MUTEX, 5) == pdTRUE) {
            float new_kp = Kp - step;
            if (new_kp < 0) {
            	new_kp = 0;
            }
            Kp = new_kp;
            xSemaphoreGive(u_ref_MUTEX);
        }
    } else if (selected_param == PARAM_KI) {
        if (xSemaphoreTake(u_ref_MUTEX, 5) == pdTRUE) {
            float new_ki = Ki - step;
            if (new_ki < 0) {
            	new_ki = 0;
            }
            Ki = new_ki;
            xSemaphoreGive(u_ref_MUTEX);
        }
    } else if (selected_param == PARAM_KD) {
        if (xSemaphoreTake(u_ref_MUTEX, 5) == pdTRUE) {
            float new_kd = Kd - step;
            if (new_kd < 0) {
            	new_kd = 0;
            }
            Kd = new_kd;
            xSemaphoreGive(u_ref_MUTEX);
        }
    }
}

// increase target voltage function - R.M.
/// @brief This function increases the target voltage by a specified step.
/// @param step The amount by which to increase the target voltage.
void increaseTargetVoltage(float step){
    if (xSemaphoreTake(u_ref_MUTEX, 5) == pdTRUE) {
        float new_target = u_ref + step;
        // Range checking
        if (new_target > 400) {
        	new_target = 400;
        }
        u_ref = new_target;
        xSemaphoreGive(u_ref_MUTEX);
    }
}
// decrease target voltage function - R.M.
/// @brief This function decreases the target voltage by a specified step.
/// @param step The amount by which to decrease the target voltage.
void decreaseTargetVoltage(float step){
    if (xSemaphoreTake(u_ref_MUTEX, 5) == pdTRUE) {
        float new_target = u_ref - step;
        // Range checking
        if (new_target < 0) {
        	new_target = 0;
        }
        u_ref = new_target;
        xSemaphoreGive(u_ref_MUTEX);
    }
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
		 xil_printf( "\nError while setting the target voltage for controller.\n");
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

		 xil_printf( "\nError while retreiving the plant voltage.\n");
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
		 xil_printf( "\nError while setting the controller output.\n");
	 }
}


// Helper variable for controlling the PWM kled brightness.
// Static and Volatile to encapsulate a volatile variable to this file only!
// Im too lazy to implement semaphore now so static volatile will do now.
// static volatile uint32_t PWM_brightness = 0;

/// @brief This is the Controller Task function
void control_task(void *pvParameters) {

	TickType_t xLastWakeTime;
	const TickType_t xInterval = pdMS_TO_TICKS(controller_interval);

	/*
	// TEMPORARY fixed input.
	float Kp = 4.5;
	float Ki = 6;
	float Kd = 0.01;
	*/

	xLastWakeTime = xTaskGetTickCount();

	// Necessary forever loop. A thread should never be able to exit!
	for( ;; ) { // Same as while(1) or while(true)

		float u_meas = getCurrentVoltage();

		setCurrentVoltage(PI_controller(u_meas, u_ref, Kd, Ki, Kp));

		// Print only after print_interval
		if(i_print == print_interval){

			i_print = 0;

			SystemMode_t current_mode = current_system_mode;

			switch(current_mode){
				case MODE_CONFIG:
					xil_printf("Rnd: %d (s) | Kp: %d.%02d | Ki: %d.%02d | Kd: %d.%02d\r\n",
						(int)(xLastWakeTime/10000),
						(int)Kp, (int)((Kp - (int)Kp) * 100),
						(int)Ki, (int)((Ki - (int)Ki) * 100),
						(int)Kd, (int)((Kd - (int)Kd) * 100));
					break;
				case MODE_MODULATION:
					xil_printf("Rnd: %d (s) | Tgt: %d (mV) | PI: %d (mV) | Plant: %d (mV)\r\n",
						(int)(xLastWakeTime/10000),
						(int)(u_ref*100),
						(int)(u_out_controller*100),
						(int)(u_meas*100)); // Integer and decimal parts of Kd
					break;
				case MODE_IDLE:
					break;
			}
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

	// Define a proper value for windup!!!
	static float windupLimit = 400;
	static float yp, yi, yd, PI_out;

	float u_max = 400;
	float u_min = 0;

	err = u_ref-u_meas; // Calculate the error value

	// Calculate yp, yi ja yd
   	yp = Kp*err;
   	yi = Ki*(h/2)*(err+err_prev) + yi_prev;
  	yd = Kd*(err-err_prev) / h;

	// Anti-winding for integrator (https://codepal.ai/code-generator/query/MjweSyOx/pid-regulator-with-anti-windup)
	if (yi > windupLimit) {
    		yi = windupLimit;
	}
	 else if (yi < -windupLimit) {
   		 yi = -windupLimit;
	}

	// Saturate the output of the controller
	float unsat_out = yp + yi + yd;

	PI_out = unsat_out; // IS THIS OK?

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



