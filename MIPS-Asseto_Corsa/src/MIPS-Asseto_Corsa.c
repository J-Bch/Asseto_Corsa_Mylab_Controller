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
#include "uart.h"
#include "lcd.h"

#include <stdio.h>

typedef struct __attribute__ ((__packed__)) _uart_telemetry
{
	float speed_kmh;
	uint32_t lap_time;
	float wheel_angular_speed_0;
	float gas;
	bool is_abs_enabled;
}uart_telemetry;

bool screen_whiped = false;

int main(void)
{
	uart_init();
	lcd_init();

	screen_whiped = true;

	char buffer[sizeof(uart_telemetry)];

	while(1)
	{
		for(int i = 0; i < sizeof(uart_telemetry); i++)
		{
			buffer[i] = uart_get_char();
		}

		uart_telemetry* telem = (uart_telemetry*)buffer;

		printf("Speed : %f\n", telem->speed_kmh);
		printf("Lap time : %i\n", telem->lap_time);
		printf("Wheel 0 angular :%f\n", telem->wheel_angular_speed_0);
		printf("gas : %f\n", telem->gas);
		printf("Abs enabled ? : %i\n", telem->is_abs_enabled);

		if(telem->is_abs_enabled)
		{
			write_text_small_font("Abs enabled", 31, 0, 0, 0, 0, 0, 0, 0, 240);

			screen_whiped = false;
		}
		else if (!screen_whiped)
		{
			whipe_screen();
			screen_whiped = true;
		}

		printf("\n");


	}
    return 0 ;
}
