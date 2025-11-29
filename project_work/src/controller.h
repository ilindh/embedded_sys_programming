#ifndef CONTROLLER_H
#define CONTROLLER_H


/* FreeRTOS includes. */
#include "controller.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Xilinx includes. */
#include "xil_printf.h"
#include "xparameters.h"
#include "xil_types.h"

/* LUT includes. */
#include "zynq_registers.h"


void control_loop(float Kp, float Ki, float Kd, float u_ref);
float PI_controller(float u_meas, float u_ref, float Kd, float Ki, float Kp);

#endif
