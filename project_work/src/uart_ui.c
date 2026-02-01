/*
 * uart_ui.c
 *
 *  Created on: 1.2.2026
 *      Author: roope
 */

#include "uart_ui.h"
#include "controller.h"
#include "ui_control.h"
#include "system_params.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Semaphores for coordination
SemaphoreHandle_t uart_config_SEMAPHORE;
SemaphoreHandle_t button_block_SEMAPHORE;

// Internal state tracking
static volatile int uart_in_config = 0;
static char rx_buffer[UART_RX_BUFFER_SIZE];
static volatile int rx_buffer_index = 0;

static void UART_ExecuteCommand(char* cmd);


void SetupUART(void){
	/*
	* This code snippet is from Ex4_2023.pdf for setting up UART
	*  - R.M.
	*/

	// TEmporary value variable
	uint32_t r = 0;

	// Disable TX & RX
    r = UART_CTRL;
    r &= ~(XUARTPS_CR_TX_EN | XUARTPS_CR_RX_EN);
    r |= XUARTPS_CR_RX_DIS | XUARTPS_CR_TX_DIS;
    UART_CTRL = r;

    // Configure UART mode
    UART_MODE = 0;
    UART_MODE &= ~XUARTPS_MR_CLKSEL; // Clear "Input clock selection" - 0: clock source is uart_ref_clk
	UART_MODE |= XUARTPS_MR_CHARLEN_8_BIT; // Set "8 bits data"
	UART_MODE |= XUARTPS_MR_PARITY_NONE; // Set "No parity mode"
	UART_MODE |= XUARTPS_MR_STOPMODE_1_BIT; // Set "1 stop bit"
	UART_MODE |= XUARTPS_MR_CHMODE_NORM; // Set "Normal mode"

	// Configure baud rate
	// baud_rate = sel_clk / (CD * (BDIV + 1) (ref: UG585 - TRM - UART)
	UART_BAUD_DIV = 6; // BDIV
	UART_BAUD_GEN = 124; // CD
	// Baud Rate = 100Mhz / (124 * (6 + 1)) = 115200 bps

	// TX & RX logic reset
	UART_CTRL |= (XUARTPS_CR_TXRST | XUARTPS_CR_RXRST);

	// Enable TX & RX
	r = UART_CTRL;
	r |= XUARTPS_CR_RX_EN | XUARTPS_CR_TX_EN; // Set TX & RX enabled
	r &= ~(XUARTPS_CR_RX_DIS | XUARTPS_CR_TX_DIS); // Clear TX & RX disabled
	UART_CTRL = r;
}

// Send one character through UART interface
void uart_send(char c) {
	while (UART_STATUS & XUARTPS_SR_TNFUL);
	UART_FIFO = c;
	while (UART_STATUS & XUARTPS_SR_TACTIVE);
}

// Send string (character array) through UART interface
void uart_send_string(const char* str) {
	char *ptr = str;
	while (*ptr != '\0') {
		uart_send(*ptr);
		ptr++;
	}
}
// Check if UART receive FIFO is not empty and return the new data
char uart_receive(void) {
	if ((UART_STATUS & XUARTPS_SR_RXEMPTY) == XUARTPS_SR_RXEMPTY) {
		return 0;
	}
	return UART_FIFO;
}


void UART_SendHelp(void) {

	xil_printf("\r\nAvailable Commands\r\n");
	xil_printf("------------------\r\n");
	xil_printf("help            - Show this help message\r\n");
	xil_printf("status          - Show current system status\r\n");
	xil_printf("config			- Change to CONFIG mode\r\n");
	xil_printf("modulation		- Change to MODULATION mode\r\n");
	xil_printf("exit			- Exit to IDLE mode\r\n");
	xil_printf("\r\n");
}

static void UART_ExecuteCommand(char* cmd) {
    char* token;
    char* value_str;
    float value;

    // Convert to lowercase for comparison
    for (int i = 0; cmd[i]; i++) {
        cmd[i] = tolower(cmd[i]);
    }

    // Tokenize command
    token = strtok(cmd, " \t");

    if (token == NULL) {
        return; // Empty command
    }

    // Command: help
    if (strcmp(token, "help") == 0) {
        UART_SendHelp();
    }

    else if (strcmp(token, "config") == 0){
    	if (xSemaphoreTake(uart_config_SEMAPHORE, 0) == pdTRUE){
    		uart_in_config = 1;
    		xil_printf("\r\n");
    		setSystemMode(MODE_CONFIG);
            xil_printf("\r\nEntered CONFIG mode via UART\r\n");
            xil_printf("Buttons are now blocked.\r\n");
            xil_printf("Type 'exit' to leave configuration mode.\r\n");
        } else {
            xil_printf("\r\nERROR: Configuration mode already active!\r\n");
        }
    }
    else if (strcmp(token, "modulation") == 0){
    	xil_printf("\r\n");
    	setSystemMode(MODE_MODULATION);
    }
    else if (strcmp(token, "exit") == 0){
    	if (uart_in_config) {
    		uart_in_config = 0;
    		xSemaphoreGive(uart_config_SEMAPHORE);
            xil_printf("\r\nExited CONFIG mode\r\n");
            xil_printf("Buttons are now active again.\r\n");
    	}
    	xil_printf("\r\n");
    	setSystemMode(MODE_IDLE);
    }
}

void UART_ProcessInput(void) {
    char c = uart_receive();

    if (c == 0) {
        return; // No character received
    }

        // Echo character back (with local echo)
    if (c == '\r' || c == '\n') {
        // uart_send_string("\r\n");

        // Null-terminate and process command
        rx_buffer[rx_buffer_index] = '\0';

        if (rx_buffer_index > 0) {
            UART_ExecuteCommand(rx_buffer);
        }

        // Reset buffer
        rx_buffer_index = 0;
        memset(rx_buffer, 0, UART_RX_BUFFER_SIZE);
    }
    else if (c == '\b' || c == 127) { // Backspace or DEL
        if (rx_buffer_index > 0) {
            rx_buffer_index--;
            // uart_send_string("\b \b"); // Erase character on terminal
        }
    }
    else if (rx_buffer_index < UART_RX_BUFFER_SIZE - 1) {
        // Add to buffer and echo
        rx_buffer[rx_buffer_index++] = c;
        // uart_send(c);
    }
}
