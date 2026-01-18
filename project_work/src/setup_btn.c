/* Copied from Course materials. */

#include <xparameters.h>
#include <xgpio.h>
#include "setup_btn.h"
#include "ui_control.h"

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

    XGpio_InterruptClear(&BTNS_SWTS,0xF);

}
