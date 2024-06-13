/*
 * dashboard.c
 *
 *  Created on: May 31, 2024
 *      Author: joachim
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "LPC17xx.h"
#include "dashboard.h"
#include "lib/fonts.h"
#include "lib/uart.h"
#include "lib/lcd.h"
#include "lib/can.h"
#include "lib/callback.h"
#include "lib/custom_gui.h"

void can_dashboard_recieve_handler();

typedef struct __attribute__ ((__packed__)) _uart_telemetry
{
	bool stop_display;
	float speed_kmh;
	uint32_t lap_time;
	float wheel_angular_speed_0;
	float gas;
	float brake;
	uint32_t engine_RPM;
	bool is_abs_enabled;
	bool is_tc_enabled;
	uint32_t message_counter;
}uart_telemetry;

bool abs_text_whiped = false;
bool tc_text_whiped = false;

bool is_screen_saver_dashboard_displaying = false;

uint32_t internal_message_counter = 0;


void dashboard_main()
{
	uart_init();
	lcd_init();
	can_init();

	callback_reset();
	callback_add(CAN_IRQn, &can_dashboard_recieve_handler);

	char buffer[sizeof(uart_telemetry)];
	internal_message_counter = 0;

	gui_reset_values();
	gui_draw_screen_saver(50, 170, "Dashboard");
	is_screen_saver_dashboard_displaying = true;

	while(1)
	{
		for(int i = 0; i < sizeof(uart_telemetry); i++)
		{
			buffer[i] = uart_get_char();
		}

		if(is_screen_saver_dashboard_displaying)
		{
			gui_clear_screen_saver(50, 170, "Dashboard");
			is_screen_saver_dashboard_displaying = false;
		}


		uart_telemetry* telem = (uart_telemetry*)buffer;

//		printf("Speed : %f\n", telem->speed_kmh);
//		printf("Lap time : %i\n", telem->lap_time);
//		printf("Wheel 0 angular :%f\n", telem->wheel_angular_speed_0);
//		printf("gas : %f\n", telem->gas);
//		printf("Abs enabled ? : %i\n", telem->is_abs_enabled);
//		printf("TC enabled ? : %i\n", telem->is_tc_enabled);
//		printf("Received message counter : %i\n", telem->message_counter);
//		printf("Internal message counter : %i\n", internal_message_counter);


		if(internal_message_counter != telem->message_counter)
		{
//			printf("Counter missmatch, resseting comm\n");
			dashboard_reset_uart_communication();
			return;
		}

		if(telem->stop_display)
		{
			whipe_screen();
			gui_reset_values();
			gui_draw_screen_saver(50, 170, "Dashboard");
			is_screen_saver_dashboard_displaying = true;

			uint8_t data[8] = { 0 };
			data[0] = CAN_RESET_CMD_NUMBER;
			can_send(0, 0, 1, data);
			continue;
		}

		if(telem->is_abs_enabled)
		{
			write_text_small_font("ABS enabled", 31, 0, 0, 0, 0, 0, 10, 20, 240);

			abs_text_whiped = false;
		}
		else if (!abs_text_whiped)
		{
			draw_square(10, 20, 8*sizeof("ABS enabled"), SMALL_FONT_HEIGHT, 0, 0, 0);
			abs_text_whiped = true;
		}

		if(telem->is_tc_enabled)
		{
			write_text_small_font("TC enabled", 31, 0, 0, 0, 0, 0, 10, 40, 240);

			tc_text_whiped = false;
		}
		else if (!tc_text_whiped)
		{
			draw_square(10, 40, 8*sizeof("TC enabled"), SMALL_FONT_HEIGHT, 0, 0, 0);
			tc_text_whiped = true;
		}


		gui_draw_accel_bar(190, 120, 10, 120, telem->gas);
		gui_draw_brake_bar(210, 120, 10, 120, telem->brake);
		gui_draw_lap_time(10, 0, telem->lap_time);



		//Send speed
		uint8_t data[8] = { 0 };
		uint32_t speed_adjusted = (uint32_t)telem->speed_kmh;
		data[0] = CAN_SPEED_DATA_NUMBER;
		data[1] = (speed_adjusted & 0xFF);
		data[2] = ((speed_adjusted >> 8) & 0xFF);
		data[3] = ((speed_adjusted >> 16) & 0xFF);
		data[4] = ((speed_adjusted >> 24) & 0xFF);
		can_send(0, 0, 5, data);


		//Send engine RPM
		data[0] = CAN_RPM_DATA_NUMBER;
		data[1] = (telem->engine_RPM & 0xFF);
		data[2] = ((telem->engine_RPM  >> 8) & 0xFF);
		data[3] = ((telem->engine_RPM  >> 16) & 0xFF);
		data[4] = ((telem->engine_RPM  >> 24) & 0xFF);
		can_send(0, 0, 5, data);

//		printf("\n");

		internal_message_counter++;

		callback_do();


	}
}

void can_dashboard_recieve_handler()
{
	uint32_t received_id;
	uint8_t* received_data;

	can_get_message(&received_id, &received_data);

	if(received_data[0] == CAN_BTN_A_DATA_NUMBER)
	{
		if(received_data[1] == 1)
			draw_square(0, 280, 8*sizeof("A pressed"), 12, 0, 0, 0);
		else if(received_data[1] == 0)
			write_text_small_font("A pressed", 31, 0, 0, 0, 0, 0, 0, 280, 240);

	}
	else if(received_data[0] == CAN_BTN_B_DATA_NUMBER)
	{
		if(received_data[1] == 1)
			draw_square(0, 300, 8*sizeof("B pressed"), 12, 0, 0, 0);
		else if(received_data[1] == 0)
			write_text_small_font("B pressed", 31, 0, 0, 0, 0, 0, 0, 300, 240);

	}
}

void dashboard_reset_uart_communication()
{
	uart_clear();
	uart_send("RESET", 5);

	//Send reset screen to the wheel
	uint8_t data[8] = { 0 };
	data[0] = CAN_RESET_CMD_NUMBER;
	can_send(0, 0, 1, data);
	dashboard_main();
}
