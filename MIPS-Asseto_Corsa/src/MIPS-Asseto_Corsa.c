/*
 * Copyright 2022 NXP
 * NXP confidential.
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "dashboard.h"
#include "driving_wheel.h"
#include "lib/uart.h"
#include "lib/lcd.h"
#include "lib/can.h"
#include "lib/usb.h"
#include "lib/touch.h"
#include "lib/callback.h"
#include "lib/buttons.h"


int mode = 0;

int main(void)
{
	LPC_GPIO2->FIODIR &= ~(1); //dip switch 0 to input

	// clear all interrupts on gpio2
	LPC_GPIOINT->IO2IntClr = ~0;

	if((LPC_GPIO2->FIOPIN & 0b1) == 0)
	{
		mode = 0;
		dashboard_main();
	}
	else
	{
		mode = 1;
		driving_wheel_main();
	}

    return 0;
}

// since we can't have multiple definitions of functions
// EINT3_IRQhandler is here to be used by both mode
void EINT3_IRQHandler(){

	if(mode == 1){

		buttons_irqhandler();

	} else {

		// set touch flag
		if(LPC_GPIOINT->IO2IntStatR & TOUCH_INT_GPIO){
			LPC_GPIOINT->IO2IntClr = TOUCH_INT_GPIO;

			callback_setflag(TOUCH_CALLBACK);
		}

	}
}
