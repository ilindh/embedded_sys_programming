/*
 * uart_ui.h
 *
 *  Created on: 1.2.2026
 *      Author: roope
 */

#ifndef SRC_UART_UI_H_
#define SRC_UART_UI_H_

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Xilinx includes. */
#include "xil_printf.h"
#include "xparameters.h"
#include "xil_types.h"
#include <xuartps_hw.h>

/* LUT includes. */
#include "zynq_registers.h"

// Buffer sizes
#define UART_RX_BUFFER_SIZE 64
#define UART_COMMAND_BUFFER_SIZE 32

// Function prototypes
void SetupUART(void);
void UART_SendHelp(void);


// UART send functions
void uart_send_char(char c);
void uart_send_string(const char* str);
char uart_receive(void);
void UART_InputHandler(void);

// External semaphore for UART config mode control
extern SemaphoreHandle_t uart_config_SEMAPHORE;

#endif /* SRC_UART_UI_H_ */
