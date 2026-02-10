/* Copied from Course materials. */

#include <xparameters.h>
#include <xgpio.h>
#include "setup_btn.h"
#include "ui_control.h"
#include "controller.h"
#include "system_params.h"

XGpio BTNS_SWTS;

void SetupPushButtons()
{
    XGpio_Initialize(&BTNS_SWTS, BUTTONS_AXI_ID);
    XGpio_InterruptEnable(&BTNS_SWTS, 0xF);
    XGpio_InterruptGlobalEnable(&BTNS_SWTS);

    Xil_ExceptionInit();

    // Enable interrupts.
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT, (Xil_ExceptionHandler) XScuGic_InterruptHandler, &xInterruptController);

    /* Defines the PushButtons_Intr_Handler as the FIQ interrupt handler.*/
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_FIQ_INT,
                                 (Xil_ExceptionHandler) PushButtons_Intr_Handler,
                                 &xInterruptController);
    Xil_ExceptionEnableMask(XIL_EXCEPTION_FIQ);
}

void PushButtons_Intr_Handler(void *data)
{
    // Handle button press to chantge system mode

	static TickType_t last_button_time = 0;
	const TickType_t debounce_delay = pdMS_TO_TICKS(200); // 200ms debounce

	// Read button states
    u32 button_states = XGpio_DiscreteRead(&BTNS_SWTS, BUTTONS_channel);

    // start debounce timer and check it
    TickType_t current_time = xTaskGetTickCount();
    if (current_time - last_button_time < debounce_delay) {
    	XGpio_InterruptClear(&BTNS_SWTS, 0xF);
    	return;
    }

    last_button_time = current_time;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // create a notify to ui_control task that will handle the necessary operations
    // Change to this tasknotify method allows for the button ISR to be shorter, suggestion for this implementation came from Claude AI. Implementation is by -R.M.
	xTaskNotifyFromISR(ui_control_task_handle, button_states, eSetBits, &xHigherPriorityTaskWoken);
    XGpio_InterruptClear(&BTNS_SWTS,0xF);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

}
