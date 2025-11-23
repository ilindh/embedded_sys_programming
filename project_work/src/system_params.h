#ifndef SYSTEM_PARAMS_H
#define SYSTEM_PARAMS_H


/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Xilinx includes. */
#include "xil_printf.h"
#include "xparameters.h"
#include "xil_types.h"

/* LUT includes. */
#include "zynq_registers.h"

// PI-Controller loop interval in ms
int excecution_interval = 10;
extern float Ki;
extern float Kp;

#endif
