/**
 * @file plant.c
 * @brief Contains all the code related to the converter model
 */

#include "plant.h"
#include "arm_math.h"
#include "system_params.h"
#include "zynq_registers.h"
#include <xttcps.h>
#include <stdint.h>

// Discretized model copied from assignment instruction sheet:
static const float A_matrix[6][6] = {{0.9652, -0.0172, 0.0057, -0.0058, 0.0052, -0.0251},
										{0.7732, 0.1252, 0.2315, 0.07, 0.1282, 0.7754},
										{0.8278, -0.7522, -0.0956, 0.3299, -0.4855, 0.3915},
										{0.9948, 0.2655, -0.3848, 0.4212, 0.3927, 0.2899},
										{0.7648, -0.4165, -0.4855, -0.3366, -0.0986, 0.7281},
										{1.1056, 0.7587, 0.1179, 0.0748, -0.2192, 0.1491}};

static const float B_matrix[6][1] = {{0.0471},
										{0.0377},
										{0.0404},
										{0.0485},
										{0.0375},
										{0.0539}};

// static const float C_matrix[1][6] = 	{{0,0,0,0,0,1}};

// This was changed from [6][1] to [6] because the [1] seemed redundant and produced an error
static float current_state[6] = 		{0,0,0,0,0,0};

// Global variables for input and output.
volatile float u_out_plant;

/// @brief This function allows for retrieving the data with MUTEXes implemented.
static float getCurrentVoltage(void){

	if( xSemaphoreTake(control_out_MUTEX, 5) == pdTRUE ) {
		/* The mutex was successfully obtained so the shared resource can beaccessed safely. */
		float current_u_in = u_out_controller;
		xSemaphoreGive(control_out_MUTEX);
		return current_u_in;
		/* Access to the shared resource is complete, so the mutex is returned. */

	 } else {
		 /* The mutex could not be obtained even after waiting 5 ticks, so the shared resource cannot be accessed. */

		 xil_printf( "Error while retreiving the controller output.");
		 return 0;
	 }
}

/// @brief This function allows for retrieving the data with MUTEXes implemented.
static void setCurrentVoltage(float u_out){

	if( xSemaphoreTake(u_out_plant_MUTEX, 5) == pdTRUE ) {
		/* The mutex was successfully obtained so the shared resource can beaccessed safely. */
		u_out_plant = u_out;
		xSemaphoreGive(u_out_plant_MUTEX);
		/* Access to the shared resource is complete, so the mutex is returned. */

	 } else {
		 /* The mutex could not be obtained even after waiting 5 ticks, so the shared resource cannot be accessed. */
		 xil_printf( "Error while setting the plant output.");
	 }
}

/// @brief Calculates the plant response based on the input signal.
/// @param u_in The input signal to the plant.
/// @return The output response of the plant.
void plant_model_task(void *pvParameters) {
	// Matrix multiplication implementation using CMSIS DSP library
	// Implementing this:
	// current_state = A_matrix*current_state + B_matrix*u_in;

	// Initialize matrices
	arm_matrix_instance_f32 MatA;
	arm_mat_init_f32(&MatA, 6, 6, (float *)A_matrix);

	// Initialize Temporary result matrices
	float Ax_result[6]; // Result of A*current_state
	float Bu_result[6]; // Result of B*u_in

	TickType_t xLastWakeTime;
	const TickType_t xInterval = pdMS_TO_TICKS(plant_interval);

	xLastWakeTime = xTaskGetTickCount();

	// Necessary forever loop. A thread should never be able to exit!
	for( ;; ) { // Same as while(1) or while(true)

		// Get a local copy for calculation.
		// If u_in is changed by controller mid calculation bad stuff will happen.

		float temp_u_in = getCurrentVoltage();

		// DEBUG:
		// float temp_u_in = 100; // Forced input without controller.

		// static float current_state[6][1]; // I dont think this should be here? (M.H.)
											 // Me neither. (I.L.)

		/*** First calculate Ax=A*current_state ***/

		// ARM Matrix and vector multiplication function
		arm_mat_vec_mult_f32(&MatA, current_state, Ax_result);

		/*** Then calculate Bu=B*u_in ***/

		// ARM scale function to multiply matrix B by scalar u_in
		arm_scale_f32((float *)B_matrix, temp_u_in, Bu_result, 6);

		/*** Finally calculate current_state = Ax + Bu ***/

		// ARM vector addition function which gives us the current_state
		arm_add_f32(Ax_result, Bu_result, current_state, 6);

		// the output u_out
		setCurrentVoltage(current_state[5]); // !NOT! defined locally (I.L.)

		// Obtain brightness from the output voltage.
		// Scaled from 0-> TARGET + 100 V and to the 16bit integer value (not anymore)
		uint16_t LED_brightness = (uint16_t)((current_state[5]/(max_out_plant))*65532);

		updatePWMBrightness(LED_brightness);

		// return u_out; // Don't return nothing. We use global variables and semaphores to transfer data in  the system.

		// This function ensures stable loop time.
		vTaskDelayUntil(&xLastWakeTime, xInterval);
	}
}

void updatePWMBrightness(uint16_t Count_Value){
	// Read the interrupt status to clear the interrupt.
	// TTC0_ISR: Triple Timer Counter (TTC) 0 - Interrupt Status Register (ISR)
	// TTC0_ISR; // Cleared on read

	// AXI_LED_DATA ^= 0x8;

	// COURSE EXAMPLE:
	// &TTC0_MATCH_0
	// const u32* ptr_match_register = &TTC0_MATCH_0;
	// *ptr_register = match_value++;

	TTC0_MATCH_1 = Count_Value;	// Order: R, G, B
	TTC0_MATCH_1_COUNTER_2 = (Count_Value >> 1); // Divided by 2
	// TTC0_MATCH_1_COUNTER_3 = Count_Value >> 2; // Divided by 4
}
