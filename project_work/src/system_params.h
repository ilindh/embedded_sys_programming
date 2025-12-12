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

// Task loop intervals in ms
#define controller_interval 1
#define ui_interval 10
#define plant_interval 1

extern volatile float Ki;
extern volatile float Kp;

extern SemaphoreHandle_t control_out_MUTEX;
extern SemaphoreHandle_t u_out_plant_MUTEX;

extern volatile float u_out_plant;
extern volatile float u_out_controller;


#endif
