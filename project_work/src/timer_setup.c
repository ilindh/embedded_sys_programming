
#include "timer_setup.h"
// This is to implement the PWM control tick handler
// We will call the Timer Tickhandler PWM coltrol from controller.c file.
#include "controller.h"
#include "zynq_registers.h"

#include <xttcps.h>
#include <stdint.h>

/* This file is used for controlling ans setting various TIMER parameters */
/* Based on the Course Example file. */

// Pointer to the register value that controls brightness
// volatile u32* ptr_match_register = &TTC1_MATCH_0;

/// @brief this function setups TIMER1 to be used for PWM led.
// Timer0 is used for FreeRTOS tasking, do not use that!
// This is copied and modified from course example!
// EDIT: Timer0 / Counter0 is attached to RGB led in hardware files so I had to try the timer0, which worked after all.
void SetupPWMTimer() {

	// Two TTC module in the Zynq PS (TTC0 & TTC1)
	// Each TTC module contains three independent 16-bit prescalers and 16-bit up/down counters (0,1,2)
	// The register naming follows Xilinx TRM UG585 for consistency - however it's not a very good example of proper naming!

	// First we need to set up the 'Clock Control' -register - TTC0_CLK_CNTRLx :
	//     1. Set prescale to 0 (plus 1) (hint: (value << XTTCPS_CLK_CNTRL_PS_VAL_SHIFT)
	//     2. Enable prescaler (hint: use XTTCPS_CLK_CNTRL_PS_EN_MASK mask)
	TTC0_CLK_CNTRL  = (0 << XTTCPS_CLK_CNTRL_PS_VAL_SHIFT) | XTTCPS_CLK_CNTRL_PS_EN_MASK;
	TTC0_CLK_CNTRL2 = (0 << XTTCPS_CLK_CNTRL_PS_VAL_SHIFT) | XTTCPS_CLK_CNTRL_PS_EN_MASK; // Set identical to TTC0_CLK_CNTRL
	TTC0_CLK_CNTRL3 = (0 << XTTCPS_CLK_CNTRL_PS_VAL_SHIFT) | XTTCPS_CLK_CNTRL_PS_EN_MASK; // Set identical to TTC0_CLK_CNTRL

	// Then let's set correct values to 'Operational mode and reset' -register - TTC0_CNT_CNTRLx :
	//     1. Reset count value (hint: use XTTCPS_CNT_CNTRL_RST_MASK mask)
	//     2. Disable counter (XTTCPS_CNT_CNTRL_DIS_MASK)
	//     3. Set timer to Match mode (XTTCPS_CNT_CNTRL_MATCH_MASK)
	//     4. Set the waveform polarity (XTTCPS_CNT_CNTRL_POL_WAVE_MASK)
	//	   When this bit is high, the waveform output goes from high to low
	//	   when a match interrupt happens. If low, the waveform output goes
	//     from low to high.
	//
	//     (Waveform output is default to EMIO, which is connected in the FPGA to the RGB led (LD6)
	TTC0_CNT_CNTRL  = XTTCPS_CNT_CNTRL_RST_MASK | XTTCPS_CNT_CNTRL_DIS_MASK | XTTCPS_CNT_CNTRL_MATCH_MASK | XTTCPS_CNT_CNTRL_POL_WAVE_MASK;
	TTC0_CNT_CNTRL2 = XTTCPS_CNT_CNTRL_RST_MASK | XTTCPS_CNT_CNTRL_DIS_MASK | XTTCPS_CNT_CNTRL_MATCH_MASK | XTTCPS_CNT_CNTRL_POL_WAVE_MASK; // Set identical to TTC0_CNT_CNTRL
	TTC0_CNT_CNTRL3 = XTTCPS_CNT_CNTRL_RST_MASK | XTTCPS_CNT_CNTRL_DIS_MASK | XTTCPS_CNT_CNTRL_MATCH_MASK | XTTCPS_CNT_CNTRL_POL_WAVE_MASK; // Set identical to TTC0_CNT_CNTRL

	// Notice that we don't touch the XTTCPS_EN_WAVE_MASK bit. This is because, as we see from Appendix B.32 of Zynq TRM,
	// it is of the type "active low". So the waveform output is enabled by default and needs to be turned off by
	// writing one to that bit.

	// Match value register - TTC0_MATCH_x
	//     1. Initialize match value to 0
	TTC0_MATCH_1           = 0;
	TTC0_MATCH_1_COUNTER_2 = 0;
	TTC0_MATCH_1_COUNTER_3 = 0;


	//DEBUG:
	// TTC0_MATCH_0           = 32000;
	// TTC0_MATCH_1_COUNTER_2 = 32000;
	// TTC0_MATCH_1_COUNTER_3 = 32000;

	// Operational mode and reset register - TTC0_CNT_CNTRLx
	//     1. Start timer (hint: clear operation using XTTCPS_CNT_CNTRL_DIS_MASK)
	TTC0_CNT_CNTRL  &= ~XTTCPS_CNT_CNTRL_DIS_MASK;
	TTC0_CNT_CNTRL2 &= ~XTTCPS_CNT_CNTRL_DIS_MASK;
	TTC0_CNT_CNTRL3 &= ~XTTCPS_CNT_CNTRL_DIS_MASK;

}
