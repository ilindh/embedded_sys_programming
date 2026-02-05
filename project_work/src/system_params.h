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

#include "timers.h"

// Task handles
// Used for what?
extern TaskHandle_t control_task_handle;
extern TaskHandle_t plant_model_task_handle;
extern TaskHandle_t ui_control_task_handle;


// Task loop intervals in ticks! Check tickrate for conversion to ms. Currently tickrate is probably 10 kHz
#define controller_interval 1
#define ui_interval 100
#define plant_interval 1

// A flag used to control if modulation print is active or not
// REPLACED WITH GLOBAL SYSTEM MODES!
// extern volatile int print_modulation;
// extern volatile int print_uart_ui;


// Variable that determines the min and max output for converter, used for brightness
#define max_out_plant 400
#define min_out_plant 0

// System Step response step target voltage
#define step_voltage_tgt 400
extern volatile float Ki;
extern volatile float Kp;
extern volatile float Kd;
extern volatile float u_ref;


// CANNOT BE extern. The true current sysmode is protected in the ui_control.c file. 
// Each instace that wants to use the system mode has to create an instance of it with
// the getSystemMode-function! (I.L)
// extern volatile SystemMode_t current_system_mode;


// System modes for UI (R.M)
typedef enum {
    MODE_CONFIG = 0,
    MODE_IDLE = 1,
    MODE_MODULATION = 2
} SystemMode_t;

// FUnction prototype. This getter allows other files to retreive the system mode securely!
extern SystemMode_t getSystemMode(void);
extern BaseType_t cooldown_semaphore_take(void);
extern void cooldown_timer_callback(TimerHandle_t cooldown_timer);

extern SemaphoreHandle_t control_out_MUTEX;
extern SemaphoreHandle_t u_out_plant_MUTEX;
extern SemaphoreHandle_t u_ref_MUTEX;
extern SemaphoreHandle_t sys_mode_MUTEX;

// Cooldown mutex stuff:
extern SemaphoreHandle_t cooldown_SEMAPHORE;
extern TimerHandle_t cooldown_timer;

extern volatile float u_out_plant;
extern volatile float u_out_controller;


#endif
