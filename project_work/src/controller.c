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

float u_old = 0;
float h = 0.1;

/// @brief This is the Controller Task function
void control_loop(float Kp, float Ki, float Kd, float u_ref) {

	static u_meas, PI_out;

	// Necessary forever loop. A thread should never be able to exit!
	for( ;; ) { // Same as while(1) or while(true)

		xTime1 = xTaskGetTickCount();
		xil_printf( "Control Loop Interval: %d \r\n", xTime1);

		u_meas = plant_response(PI_out);

		PI_out = PI_controller(u_meas, u_ref);

		xil_printf( "Converter voltage [V]: %d \r\n", u_meas);
		xil_printf( "PI output [V]: %d \r\n", PI_out);

		vTaskDelayUntil(&xLastWakeTime, xPeriod);
	}

}

/// @brief This is the PID controller function
/// @param Kp (proportional), Ki (integrative), Kd (derivative), ref (?), y (?) h (?)
/// @return PI controller output
float PI_controller(float u_meas, float u_ref) {

	static float32_t err, err_prev, yi_prev, yd_prev;

	//määritä vielä tarkempi arvo!!
	static float32_t windupLimit =10;
	static float32_t yp, yi, yd, PI_out;

	float u_max = 400;
	float u_min = 0;

	err = u_meas - u_ref; // Calculate the error value

    //  err = ref – y;

	// Calculate yp, yi ja yd
   	yp = Kp*err;
   	yi = Ki*(h/2)*(err+err_prev) + yi_prev;
  	yd = Kd*(2/h)*(err-err_prev) - yd_prev;


	// Anti-winding integraattorille (https://codepal.ai/code-generator/query/MjweSyOx/pid-regulator-with-anti-windup)
	if (yi > windupLimit) {
    		yi = windupLimit;
	}
	 else if (yi < -windupLimit) {
   		 yi = -windupLimit;
	}

	// Saturate the output of the controller
	unsat_out = yp + yi + yd;

	PI_out = unsat_out; //toimiiko if tsydeemi tällä en oo yhtää varma??

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



