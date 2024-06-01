/*
 * dashboard.c
 *
 *  Created on: May 31, 2024
 *      Author: joachim
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "LPC17xx.h"
#include "dashboard.h"
#include "lib/uart.h"
#include "lib/lcd.h"
#include "lib/can.h"
#include "lib/callback.h"

void can_dashboard_recieve_handler();

typedef struct __attribute__ ((__packed__)) _uart_telemetry
{
	float speed_kmh;
	uint32_t lap_time;
	float wheel_angular_speed_0;
	float gas;
	bool is_abs_enabled;
	bool is_tc_enabled;
	uint32_t message_counter;
}uart_telemetry;

bool screen_whiped = false;

uint32_t internal_message_counter = 0;

void dashboard_main()
{
	uart_init();
	lcd_init();
	can_init();

	callback_add(CAN_IRQn, &can_dashboard_recieve_handler);

	screen_whiped = true;

	char buffer[sizeof(uart_telemetry)];
	internal_message_counter = 0;

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
		printf("TC enabled ? : %i\n", telem->is_tc_enabled);
		printf("Received message counter : %i\n", telem->message_counter);
		printf("Internal message counter : %i\n", internal_message_counter);

		if(internal_message_counter != telem->message_counter)
		{
			printf("Counter missmatch, resseting comm\n");
			dashboard_reset_uart_communication();
			return;
		}

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

		if(telem->is_tc_enabled)
		{
			write_text_small_font("TC enabled", 31, 0, 0, 0, 0, 0, 0, 20, 240);

			screen_whiped = false;
		}
		else if (!screen_whiped)
		{
			whipe_screen();
			screen_whiped = true;
		}

		printf("\n");

		internal_message_counter++;


	}
}

void can_dashboard_recieve_handler()
{

}

void dashboard_reset_uart_communication()
{
	uart_clear();
	uart_send("RESET", 5);
	dashboard_main();
}
