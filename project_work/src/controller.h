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


extern void PI_controller(float u_ref);

#endif
