/* Copied from Course materials. */

#include <xparameters.h>
#include <xgpio.h>
#include "setup_btn.h"
#include "ui_control.h"
#include "timers.h"

XGpio BTNS_SWTS;
static TimerHandle_t xButtonTimer;
void ButtonTimerCallback(TimerHandle_t xTimer);


void SetupPushButtons() {
	XGpio_Initialize(&BTNS_SWTS, BUTTONS_AXI_ID);
	XGpio_InterruptEnable(&BTNS_SWTS, 0xF);
	XGpio_InterruptGlobalEnable(&BTNS_SWTS);

	Xil_ExceptionInit();

	// Enables interrupts
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
								(Xil_ExceptionHandler) XScuGic_InterruptHandler,
								&xInterruptController);

	// Defines the PushButtons_Intr_Handler as the IRQ (standard Interrupt Request) interrupt handler.
	// Changed to IRQ from FIQ for a more standard interrupt handling approach for push-buttons. M.H.
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
								(Xil_ExceptionHandler) PushButtons_Intr_Handler,
								&xInterruptController);
	Xil_ExceptionEnableMask(XIL_EXCEPTION_FIQ);

	// Add a timer for button debounching (period of 20 ms, pdFalse=runs once)
	xButtonTimer = xTimerCreate( "Debounce", pdMS_TO_TICKS(20), pdFALSE, (void *)0, ButtonTimerCallback);
}

/// @brief Disable button interrups, clear interrupt flag, and start debounce timer
/// @param data 
void PushButtons_Intr_Handler(void *data) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Disable Button Interrupts immediately to ignore the mechanical bounce
    XGpio_InterruptDisable(&BTNS_SWTS, 0xF);

    // This clears the interrupt flag so the GIC (generic interrupt controller) is happy
    XGpio_InterruptClear(&BTNS_SWTS, 0xF);

    // This schedules the "ButtonTimerCallback" to run in 20ms.
    if( xButtonTimer != NULL ) {
        xTimerStartFromISR( xButtonTimer, &xHigherPriorityTaskWoken );
    }

    // Force a context switch if the timer task is high priority
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

/// @brief Callback function for the button debounce timer. Reads button states and changes system mode.
/// @param xTimer 
void ButtonTimerCallback(TimerHandle_t xTimer) {
    // Read the button states after debounce period
    u32 button_states = XGpio_DiscreteRead(&BTNS_SWTS, BUTTONS_channel);
	
	// BUTTON 0 - change system mode
	if (button_states & 0x1) {
		// Change system mode by cycling through the enum values
		current_system_mode = (SystemMode_t)((current_system_mode + 1) % 3);

		xil_printf("System mode changed to: %d\r\n", current_system_mode);
	}

	// BUTTON 1 - change parameter
	if (button_states & 0x2) { 
		xil_printf("Button 1 pressed\r\n");
		// Button 2 functionality can be added here
	}

	// BUTTON 2 - increase parameter / voltage value
	if (button_states & 0x4) { 
		xil_printf("Button 2 pressed\r\n");
		// Button 2 functionality can be added here
	}

	// BUTTON 3 - decrease parameter / voltage value
	if (button_states & 0x8) { 
		xil_printf("Button 3 pressed\r\n");
		// Button 3 functionality can be added here
	}

    // Re-enable interrupts to listen for the NEXT press
    XGpio_InterruptEnable(&BTNS_SWTS, 0xF);
}