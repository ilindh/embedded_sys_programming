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

// System modes for UI (R.M)
typedef enum {
    MODE_CONFIG = 0,
    MODE_IDLE = 1,
    MODE_MODULATION = 2
} SystemMode_t;

extern volatile SystemMode_t current_system_mode;

// Task handles
extern TaskHandle_t control_task_handle;
extern TaskHandle_t plant_model_task_handle;
extern TaskHandle_t ui_control_task_handle;


void ui_control_task(void *pvParameters);

#endif
