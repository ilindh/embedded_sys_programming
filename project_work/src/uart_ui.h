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

// UART semaphore timeout in ms
#define UART_SEM_TIMEOUT_MS 5000

// Function prototypes
void SetupUART(void);
/*
void UART_ProcessInput(void);
void UART_SendWelcomeMessage(void);
void UART_SendPrompt(void);
void UART_SendModeStatus(void);
*/
void UART_SendHelp(void);


// UART send functions
void uart_send_char(char c);
void uart_send_string(const char* str);
char uart_receive(void);


// External semaphore for UART config mode control
extern SemaphoreHandle_t uart_config_SEMAPHORE;
extern SemaphoreHandle_t button_block_SEMAPHORE;

/*
// Additional helper functions
int UART_IsInConfigMode(void);
void UART_BlockButtonsTemporarily(void);
*/

#endif /* SRC_UART_UI_H_ */
