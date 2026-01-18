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

void SetupPushButtons() {
    int Status;
    
    // 1. Initialize GPIO
    XGpio_Initialize(&BTNS_SWTS, BUTTONS_AXI_ID);
    
    // 2. Setup GIC Connect (Connecting the button handler to the GIC)
    u32 Button_Intr_ID = XPAR_FABRIC_AXI_GPIO_SW_BTN_IP2INTC_IRPT_INTR; 

    Status = XScuGic_Connect(&xInterruptController, Button_Intr_ID,
                            (Xil_ExceptionHandler)PushButtons_Intr_Handler,
                            (void *)&BTNS_SWTS); // Pass the GPIO instance as data

    if (Status != XST_SUCCESS) {
        xil_printf("GIC Connect Failed\r\n");
        return;
    }

    // 3. Enable the interrupt input at the GPIO Core
    // Enable Channel 2 interrupt (Bit 1 = Channel 2, Bit 0 = Channel 1). 
    // 0xF is okay but 0x2 is more precise if buttons are on Ch2.
    XGpio_InterruptEnable(&BTNS_SWTS, 0xF); 
    XGpio_InterruptGlobalEnable(&BTNS_SWTS);

    // 4. Enable the interrupt input at the GIC
    XScuGic_Enable(&xInterruptController, Button_Intr_ID);

    // 5. Register the GIC handler to the CPU (Only do this ONCE)
    Xil_ExceptionInit();
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
                                (Xil_ExceptionHandler) XScuGic_InterruptHandler,
                                &xInterruptController);

    // 6. Enable Interrupts on the CPU
    // Use Xil_ExceptionEnable() for standard IRQ, not EnableMask(FIQ)
    Xil_ExceptionEnable(); 

    // Create debounce timer
    xButtonTimer = xTimerCreate("Debounce", pdMS_TO_TICKS(20), pdFALSE, (void *)0, ButtonTimerCallback);
}

/// @brief Disable button interrups, clear interrupt flag, and start debounce timer
/// @param data 
void PushButtons_Intr_Handler(void *data) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    XGpio *GpioPtr = (XGpio *)data; // Cast the data pointer back to XGpio

    // Disable Button Interrupts
    XGpio_InterruptDisable(GpioPtr, 0xF);

    // Clear the interrupt
    XGpio_InterruptClear(GpioPtr, 0xF);

    // Start Timer
    if( xButtonTimer != NULL ) {
        xTimerStartFromISR( xButtonTimer, &xHigherPriorityTaskWoken );
    }

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
