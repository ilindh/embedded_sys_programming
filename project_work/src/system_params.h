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

// Variable that determines the min and max output for converter, used for brightness
#define max_out_plant 400
#define min_out_plant 0

// System Step response step target voltage
#define step_voltage_tgt 400

extern volatile float Ki;
extern volatile float Kp;
extern volatile float Kd;
extern volatile float u_ref;


extern SemaphoreHandle_t control_out_MUTEX;
extern SemaphoreHandle_t u_out_plant_MUTEX;
extern SemaphoreHandle_t u_ref_MUTEX;


extern volatile float u_out_plant;
extern volatile float u_out_controller;


#endif
