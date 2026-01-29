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

void increaseTargetVoltage(float step);
void decreaseTargetVoltage(float step);
void setTargetVoltage(float target_voltage);
void increaseParameter(float step);
void decreaseParameter(float step);
void toggleParameter(void);

typedef enum {
    PARAM_KP = 0,
    PARAM_KI = 1,
	PARAM_KD = 2
} ConfigParam_t;

ConfigParam_t getSelectedParameter(void);

void control_task(void *pvParameters);
float PI_controller(float u_meas, float u_ref, float Kd, float Ki, float Kp);
void PWM_control(void);
#endif
