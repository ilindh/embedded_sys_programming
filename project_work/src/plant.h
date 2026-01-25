#ifndef PLANT_H
#define PLANT_H


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

/* CMSIS DSP Include 
 * Required for float32_t definition used in the prototype below 
 */
#include "arm_math.h"

/* Function Prototypes */
// This allows other files (like main.c) to call your plant function
void plant_model_task(void *pvParameters);
void updatePWMBrightness(uint16_t Count_Value);

#endif
