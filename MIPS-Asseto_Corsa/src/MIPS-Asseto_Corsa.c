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



int main(void)
{
	LPC_GPIO2->FIODIR &= ~(1); //dip switch 0 to input

//	usb_init();

	if((LPC_GPIO2->FIOPIN & 0b1) == 0)
	{
		dashboard_main();
	}
	else
	{
		driving_wheel_main();
	}

    return 0;
}
