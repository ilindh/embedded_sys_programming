/*
 * uart_ui.c
 *
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

// Internal state tracking
static volatile int uart_in_config = 0;
static char rx_buffer[UART_RX_BUFFER_SIZE];
static volatile int rx_buffer_index = 0;

static void UART_ExecuteCommand(char *cmd);

void SetupUART(void)
{
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
	UART_MODE &= ~XUARTPS_MR_CLKSEL;		// Clear "Input clock selection" - 0: clock source is uart_ref_clk
	UART_MODE |= XUARTPS_MR_CHARLEN_8_BIT;	// Set "8 bits data"
	UART_MODE |= XUARTPS_MR_PARITY_NONE;	// Set "No parity mode"
	UART_MODE |= XUARTPS_MR_STOPMODE_1_BIT; // Set "1 stop bit"
	UART_MODE |= XUARTPS_MR_CHMODE_NORM;	// Set "Normal mode"

	// Configure baud rate
	// baud_rate = sel_clk / (CD * (BDIV + 1) (ref: UG585 - TRM - UART)
	UART_BAUD_DIV = 6;	 // BDIV
	UART_BAUD_GEN = 124; // CD
	// Baud Rate = 100Mhz / (124 * (6 + 1)) = 115200 bps

	// TX & RX logic reset
	UART_CTRL |= (XUARTPS_CR_TXRST | XUARTPS_CR_RXRST);

	// Enable TX & RX
	r = UART_CTRL;
	r |= XUARTPS_CR_RX_EN | XUARTPS_CR_TX_EN;	   // Set TX & RX enabled
	r &= ~(XUARTPS_CR_RX_DIS | XUARTPS_CR_TX_DIS); // Clear TX & RX disabled
	UART_CTRL = r;
}

/// @brief Receive one character from UART interface receive FIFO
char uart_receive(void)
{
	if ((UART_STATUS & XUARTPS_SR_RXEMPTY) == XUARTPS_SR_RXEMPTY)
	{
		return 0;
	}
	return UART_FIFO;
}

/// @brief Send help message via UART listing available commands
void UART_SendHelp(void)
{

	xil_printf("\r\nAvailable Commands\r\n");
	xil_printf("------------------\r\n");
	xil_printf("help            - Show this help message\r\n");
	xil_printf("config			- Change to CONFIG mode\r\n");
	xil_printf("modulation		- Change to MODULATION mode\r\n");
	xil_printf("exit			- Exit to IDLE mode\r\n");
	xil_printf("------------------\r\n");
	xil_printf("Following commands available only in config mode:\r\n");
	xil_printf("setparam <param> <value> - Set parameter (kp, ki, kd) value (0-100)\r\n");
	xil_printf("------------------\r\n");
	xil_printf("Following commands available only in modulation mode:\r\n");
	xil_printf("setvoltage <value> - Set target voltage (0-400)\r\n");
	xil_printf("\r\n\r\n");
}

/// @brief Execute a command received via UART
/// @param cmd Command string
static void UART_ExecuteCommand(char *cmd)
{
	// Local variables
	char *token;
	char *param;
	int param_val;
	char *value_str;
	float value;

	// Convert to lowercase for comparison
	for (int i = 0; cmd[i]; i++)
	{
		cmd[i] = tolower(cmd[i]);
	}

	xil_printf("\r\n\r\n>%s\r\n", cmd);

	// Tokenize command
	token = strtok(cmd, " \t");

	// Empty command
	if (token == NULL)
	{
		return;
	}

	// Command: help
	if (strcmp(token, "help") == 0)
	{
		UART_SendHelp();
	}


	// IF parameter semaphore is not taken, we can change params.
	if( cooldown_semaphore_take() != pdTRUE){
		// Debug:
		xil_printf("\r\nButtons are in use! Serial terminal blocked!\r\n");
		return;
	} 

	// IF semaphore was successufully taken:
	else {
	// Release the semaphore immediately. We just want to check if we are allowed to use buttons!
	xSemaphoreGive(cooldown_SEMAPHORE);
	
		// Command: config
		if (strcmp(token, "config") == 0)
		{
			// Try to take semaphore so that buttons are blocked
			if (xSemaphoreTake(uart_config_SEMAPHORE, 0) == pdTRUE)
			{
				// Enter configuration mode
				uart_in_config = 1;
				xil_printf("\r\n");
				setSystemMode(MODE_CONFIG);
				xil_printf("\r\nEntered CONFIG mode via UART\r\n");
				xil_printf("Buttons are now blocked.\r\n");
				xil_printf("Type 'exit' to leave configuration mode.\r\n");
			}
			else
			{
				xil_printf("\r\nERROR: Configuration mode already active!\r\n");
			}
		}

		// Command: modulation
		else if (strcmp(token, "modulation") == 0) {
			// if in config mode, exit it first, releasing semaphore
			if (uart_in_config)
			{
				uart_in_config = 0;
				xSemaphoreGive(uart_config_SEMAPHORE);
				xil_printf("\r\nExited CONFIG mode\r\n");
				xil_printf("Buttons are now active again.\r\n");
			}
			xil_printf("\r\n");
			setSystemMode(MODE_MODULATION);
		}

		// Command: exit
		else if (strcmp(token, "exit") == 0)
		{
			// if in config mode, exit it first, releasing semaphore
			if (uart_in_config)
			{
				uart_in_config = 0;
				xSemaphoreGive(uart_config_SEMAPHORE);
				xil_printf("\r\nExited CONFIG mode\r\n");
				xil_printf("Buttons are now active again.\r\n");
			}
			// Exit to idle mode
			xil_printf("\r\n");
			setSystemMode(MODE_IDLE);
		}

		// Command: setparam
		// set parameter value (only in config mode) kp, ki, kd from 0 to 100
		// EXAMPLE setparam kp 50
		else if (strcmp(token, "setparam") == 0)
		{
			// only if in config mode
			if (uart_in_config) {
				// get the param and continue only if param is valid
				param = strtok(NULL, " \t");
				if (param != NULL) {
					
					param_val = -1;
					if (strcmp(param, "kp") == 0)
					{
						param_val = PARAM_KP;
					}
					else if (strcmp(param, "ki") == 0)
					{
						param_val = PARAM_KI;
					}
					else if (strcmp(param, "kd") == 0)
					{
						param_val = PARAM_KD;
					}
					else
					{
						xil_printf("\r\nInvalid usage.\r\n");
						return;
					}

					// get the value and continue only if valid
					value_str = strtok(NULL, " \t");
					if (value_str != NULL)
					{
						// convert to float and check range
						value = atof(value_str);
						if (value >= 0 && value <= 100)
						{
							// set parameter
							setParameter(param_val, value);
							xil_printf("\r\nParameter %s set to %d.%02d\r\n",
									param,
									(int)value, (int)((value - (int)value) * 100 + 0.5));
						}
						else
						{
							xil_printf("\r\nInvalid usage.\r\n");
						}
					}
					else
					{
						xil_printf("\r\nInvalid usage.\r\n");
					}
				}
				else
				{
					xil_printf("\r\nInvalid usage.\r\n");
				}
			}
			else
			{
				xil_printf("\r\nNot in config mode.\r\n");
			}
		}

		// Command: setvoltage
		// set target voltage (only in modulation mode) from 0 to 400
		// EXAMPLE setvoltage 250
		else if (strcmp(token, "setvoltage") == 0)
		{
			// only if in modulation mode
			if (getSystemMode() == MODE_MODULATION)
			{
				// get the value and continue only if valid
				value_str = strtok(NULL, " \t");
				if (value_str != NULL)
				{
					// convert to float and check range
					value = atof(value_str);
					if (value >= 0 && value <= 400)
					{
						// set target voltage
						setTargetVoltage(value);
						xil_printf("\r\nTarget voltage set to %s V\r\n", value_str);
					}
					else
					{
						xil_printf("\r\nInvalid usage.\r\n");
					}
				}
				else
				{
					xil_printf("\r\nInvalid usage.\r\n");
				}
			}
			else
			{
				xil_printf("\r\nNot in modulation mode.\r\n");
			}
		}
		else
		{
			xil_printf("\r\nNot a command.\r\n");
		}
	}
}

void UART_ProcessInput(void)
{
	// Read character from UART
	char c = uart_receive();

	if (c == 0)
	{
		return; // No character received
	}

	// DO NOT Echo character back (with local echo)
	if (c == '\r' || c == '\n')
	{
		// Null-terminate and process command
		rx_buffer[rx_buffer_index] = '\0';

		if (rx_buffer_index > 0)
		{
			UART_ExecuteCommand(rx_buffer);
		}

		// Reset buffer
		rx_buffer_index = 0;
		memset(rx_buffer, 0, UART_RX_BUFFER_SIZE);
	}

	// Add character to buffer if space available
	else if (rx_buffer_index < UART_RX_BUFFER_SIZE - 1)
	{
		// Add to buffer
		rx_buffer[rx_buffer_index++] = c;
	}
}
