#ifndef UI_CONTROL_H
#define UI_CONTROL_H

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
#include "system_params.h"

void setSystemMode(SystemMode_t new_sys_mode);
void ui_control_task(void *pvParameters);


#endif
