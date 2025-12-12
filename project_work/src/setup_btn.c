/* Copied from Course materials. */

#include <xparameters.h>
#include <xgpio.h>
#include "setup_btn.h"

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
	if (XGpio_DiscreteRead(&BTNS_SWTS, BUTTONS_channel)){
		AXI_LED_DATA ^= 0x2;
	}
	XGpio_InterruptClear(&BTNS_SWTS,0xF);
}
