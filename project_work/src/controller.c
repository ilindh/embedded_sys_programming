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
#include <stdbool.h>

// Variables:

// Step size for integration. Mathced with "sampling interval"
float h = (float)controller_interval / 1000.0;

TickType_t xTaskGetTickCount(void);

// Global variables for input and output.
static volatile float u_out_controller;

static const int print_interval = 500;
static volatile int i_print = 0;

// volatile target voltage variable
volatile float u_ref = 0;

// variables for the Kp and Ki parameters
static float Kp = 4.5;
static float Ki = 6.0;
static float Kd = 0.01;

// Controller state structure.
// !STATIC!
static PIDControllerState_t controller_state = {0};

volatile ConfigParam_t selected_param = PARAM_KP;

/// set parameter function for UART usage - R.M.
/// @brief This function allows for setting the controller parameters from UART with MUTEXes implemented.
/// @param param The parameter to set (Kp/Ki/Kd).
/// @param target_value The value to set the parameter to.
void setParameter(int param, float target_value)
{
	if (xSemaphoreTake(controller_MUTEX, 5) == pdTRUE)
	{
		/* The mutex was successfully obtained so the shared resource can beaccessed safely. */

		// change the param based on selected param
		if (param == PARAM_KP){
			Kp = target_value;
		}
		else if (param == PARAM_KI){
			Ki = target_value;
		}
		else{
			Kd = target_value;
		}
		xSemaphoreGive(controller_MUTEX);
		/* Access to the shared resource is complete, so the mutex is returned. */
	}
	else{
		// error getting the mutex
		xil_printf("\r\nError while setting the controller parameter.\r\n");
	}
}

// toggle parameter function for button usage - R.M.
/// @brief This function toggles the selected parameter (Kp/Ki) for button usage
void toggleParameter(void)
{
	selected_param = (ConfigParam_t)((selected_param + 1) % 3);
}

// get selected parameter function for button usage - R.M.
/// @brief This function retrieves the currently selected parameter (Kp/Ki/Kd) for button usage
ConfigParam_t getSelectedParameter(void)
{
	return selected_param;
}

// increase parameter function - R.M.
/// @brief This function increases the selected parameter (Kp/Ki) by a specified step.
/// @param step The amount by which to increase the selected parameter voltage.
void increaseParameter(float step)
{
	if (xSemaphoreTake(controller_MUTEX, 5) == pdTRUE){
		/* The mutex was successfully obtained so the shared resource can beaccessed safely. */
		if (selected_param == PARAM_KP){
			float new_kp = Kp + step;
			if (new_kp > 100.0)
			{
				new_kp = 100.0;
			}
			Kp = new_kp;
		}
		else if (selected_param == PARAM_KI){
			float new_ki = Ki + step;
			if (new_ki > 100.0)
			{
				new_ki = 100.0;
			}
			Ki = new_ki;
		}
		else if (selected_param == PARAM_KD){
			float new_kd = Kd + step;
			if (new_kd > 100.0)
			{
				new_kd = 100.0;
			}
			Kd = new_kd;
		}
		xSemaphoreGive(controller_MUTEX);
		/* Access to the shared resource is complete, so the mutex is returned. */
	}
	else{
		// error getting the mutex
		xil_printf("\r\nError while increasing the controller parameter.\r\n");
	}
}

// decrease parameter function - R.M.
/// @brief This function decreases the selected parameter (Kp/Ki) by a specified step.
/// @param step The amount by which to decreases the selected parameter voltage.
void decreaseParameter(float step){
	if (xSemaphoreTake(controller_MUTEX, 5) == pdTRUE){
		/* The mutex was successfully obtained so the shared resource can beaccessed safely. */
		if (selected_param == PARAM_KP){
			float new_kp = Kp - step;
			if (new_kp < 0)
			{
				new_kp = 0;
			}
			Kp = new_kp;
		}
		else if (selected_param == PARAM_KI) {
			float new_ki = Ki - step;
			if (new_ki < 0)
			{
				new_ki = 0;
			}
			Ki = new_ki;
		}
		else if (selected_param == PARAM_KD) {
			float new_kd = Kd - step;
			if (new_kd < 0)
			{
				new_kd = 0;
			}
			Kd = new_kd;

		}
		/* Access to the shared resource is complete, so the mutex is returned. */
		xSemaphoreGive(controller_MUTEX);
	}
	else{
		// error getting the mutex
		xil_printf("\r\nError while decreasing the controller parameter.\r\n");
	}

}

// increase target voltage function - R.M.
/// @brief This function increases the target voltage by a specified step.
/// @param step The amount by which to increase the target voltage.
void increaseTargetVoltage(float step)
{
	if (xSemaphoreTake(controller_MUTEX, 5) == pdTRUE)
	{
		/* The mutex was successfully obtained so the shared resource can beaccessed safely. */
		float new_target = u_ref + step;
		// Range checking
		if (new_target > 400)
		{
			new_target = 400;
		}
		u_ref = new_target;
		xSemaphoreGive(controller_MUTEX);
		/* Access to the shared resource is complete, so the mutex is returned. */
	}
	else{
		// error getting the mutex
		xil_printf("\r\nError while setting the target voltage for controller.\r\n");
	}
}
// decrease target voltage function - R.M.
/// @brief This function decreases the target voltage by a specified step.
/// @param step The amount by which to decrease the target voltage.
void decreaseTargetVoltage(float step)
{
	if (xSemaphoreTake(controller_MUTEX, 5) == pdTRUE)
	{
		/* The mutex was successfully obtained so the shared resource can beaccessed safely. */
		float new_target = u_ref - step;
		// Range checking
		if (new_target < 0)
		{
			new_target = 0;
		}
		u_ref = new_target;
		xSemaphoreGive(controller_MUTEX);
		/* Access to the shared resource is complete, so the mutex is returned. */
	}
	else{
		// error getting the mutex
		xil_printf("\r\nError while decreasing the target voltage for controller.\r\n");
	}
}

/// @brief This function allows for setting the target voltage with MUTEXes implemented.
void setTargetVoltage(float new_target)
{
	if (xSemaphoreTake(controller_MUTEX, 5) == pdTRUE)
	{
		/* The mutex was successfully obtained so the shared resource can beaccessed safely. */
		// Range checking for the targetvoltage - R.M.
		if (new_target < 0)
		{
			new_target = 0;
		}
		else if (new_target > 400)
		{
			new_target = 400;
		}
		u_ref = new_target;
		xSemaphoreGive(controller_MUTEX);
		/* Access to the shared resource is complete, so the mutex is returned. */
	}
	else
	{
		// error getting the mutex
		xil_printf("\r\nError while setting the target voltage for controller.\r\n");
	}
}

/// @brief This function allows for retrieving the data with MUTEXes implemented.
/// This function allows other tasks to access the controller voltage variable
float getCurrentControllerVoltage(void){

	if( xSemaphoreTake(control_out_MUTEX, 5) == pdTRUE ) {
		/* The mutex was successfully obtained so the shared resource can beaccessed safely. */
		float controller_voltage = u_out_controller;
		xSemaphoreGive(control_out_MUTEX);
		return controller_voltage;
		/* Access to the shared resource is complete, so the mutex is returned. */

	 } else {
		 /* The mutex could not be obtained even after waiting 5 ticks, so the shared resource cannot be accessed. */
		 xil_printf( "Error while retreiving the controller output.");
		 return 0;
	 }
}

/// @brief This function allows for retrieving the data with MUTEXes implemented.
static void setControllerOutputVoltage(float u_out)
{

	if (xSemaphoreTake(control_out_MUTEX, 5) == pdTRUE)
	{
		/* The mutex was successfully obtained so the shared resource can beaccessed safely. */
		u_out_controller = u_out;
		xSemaphoreGive(control_out_MUTEX);
		/* Access to the shared resource is complete, so the mutex is returned. */
	}
	else
	{
		// error getting the mutex
		xil_printf("\nError while setting the controller output.\n");
	}
}

/// @brief This is the Controller Task function
void control_task(void *pvParameters)
{

	TickType_t xLastWakeTime;
	const TickType_t xInterval = pdMS_TO_TICKS(controller_interval);

	xLastWakeTime = xTaskGetTickCount();

	// Necessary forever loop. A thread should never be able to exit!
	for (;;)
	{ // Same as while(1) or while(true)

		float u_meas = getPlantOutputVoltage();
		// Reset = 0 (final parameter)

		// Get motherfucker
		SystemMode_t current_mode = getSystemMode();

		if(current_mode == MODE_MODULATION){
			setControllerOutputVoltage(PI_controller(u_meas, u_ref, Kd, Ki, Kp, 0, &controller_state));
		} else {
			// ZERO THE SYSTEM!!
			PI_controller(0,0,0,0,0,1, &controller_state);
			setControllerOutputVoltage(0);
		}

		// Print only after print_interval and if modulation print is set as active
		if ((i_print == print_interval))
		{

			i_print = 0;

			switch (current_mode)
			{
			case MODE_CONFIG:
				xil_printf("\rCurrent Params: Kp: %d.%02d | Ki: %d.%02d | Kd: %d.%02d  | Plant: %d (mV)      ",
						   (int)Kp, (int)((Kp - (int)Kp) * 100 + 0.5),
						   (int)Ki, (int)((Ki - (int)Ki) * 100 + 0.5),
						   (int)Kd, (int)((Kd - (int)Kd) * 100 + 0.5),
						   (int)(u_meas * 1000));
				break;
			case MODE_MODULATION:
				// Write new controller output value to plant:
				xil_printf("\rRnd: %d (s) | Tgt: %d (mV) | PI: %d (mV) | Plant: %d (mV)      ",
						   (int)(xLastWakeTime / 10000),
						   (int)(u_ref * 1000),
						   (int)(u_out_controller * 1000),
						   (int)(u_meas * 1000));
				break;
			case MODE_IDLE:
				xil_printf("\rRnd: %d (s) | Plant: %d (mV)      ",
						   (int)(xLastWakeTime / 10000),
						   (int)(u_meas * 1000));
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
float PI_controller(float u_meas, float u_ref, float Kd, float Ki, float Kp, uint32_t reset, PIDControllerState_t *state){

	// If reset command sent, reset all!
	if(reset){
		state->err = 0;
		state->err_prev_1 = 0;
		state->err_prev_2 = 0;
		state->yi_prev = 0;
		state->yp = 0;
		state->yi = 0;
		state->yd = 0;
		state->PI_out = 0;
	}

	// Juho fucking fix this shit:
	float u_max = 400.0;
	float u_min = 0.0;

	state->err = u_ref - u_meas; // Calculate the error value

	// Calculate
	// YP //
	state->yp = Kp * state->err;
	// YI //
	state->yi = Ki * (h / 2) * (state->err + state->err_prev_1) + state->yi_prev;

	// YD //
	// Calculate mean for the d to reduce noise.
	float err_d = ((state->err - state->err_prev_1) + (state->err_prev_1 - state->err_prev_2)) / 2;
	state->yd = Kd * (err_d) / h;

	// Anti-winding for integrator (https://codepal.ai/code-generator/query/MjweSyOx/pid-regulator-with-anti-windup)
	if (state->yi > WINDUP_LIMIT)
	{
		state->yi = WINDUP_LIMIT;
	}
	else if (state->yi < -WINDUP_LIMIT)
	{
		state->yi = -WINDUP_LIMIT;
	}

	// Saturate the output of the controller
	float unsat_out = state->yp + state->yi + state->yd;

	state->PI_out = unsat_out; // IS THIS OK?

	if (state->PI_out > u_max)
	{
		state->PI_out = u_max;
	}
	else if (state->PI_out < u_min)
	{
		state->PI_out = u_min;
	}

	// Update the old values
	state->yi_prev = state->yi;
	// yd_prev = yd;
	state->err_prev_2 = state->err_prev_1;
	state->err_prev_1 = state->err;

	return state->PI_out;
}
