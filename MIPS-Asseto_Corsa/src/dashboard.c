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
#include "lib/touch.h"
#include "lib/i2c.h"

#define LEDS_R_ROTATION (1 << 7)
#define LEDS_R_A		(1 << 6)
#define LEDS_R_B		(1 << 5)
#define LEDS_R_UART		(1 << 4)
#define LEDS_T_UART		(1 << 3)


void can_dashboard_recieve_handler();
void abs_touch_button_plus(int state);
void abs_touch_button_minus(int state);
void tc_touch_button_plus(int state);
void tc_touch_button_minus(int state);

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


typedef struct __attribute__ ((__packed__)) _inputs_t
{
	uint8_t rotation;
	uint8_t accelerator;
	uint8_t brake;
	uint8_t abs;
	uint8_t tc;
} inputs_t;

bool abs_text_whiped = false;
bool tc_text_whiped = false;

bool is_screen_saver_dashboard_displaying = false;

uint32_t internal_message_counter = 0;


static char sync_marker = 255;

#define SYNC uart_send(&sync_marker, 1);

inputs_t inputs = {0};

void dashboard_display_screen_saver(){
	whipe_screen();
	gui_reset_values();
	gui_draw_screen_saver(50, 170, "Dashboard");
	is_screen_saver_dashboard_displaying = true;
}

void dashboard_main()
{
	lcd_init();
	i2c_init();
	// set clock for i2c
	LPC_I2C0->I2SCLH = 125;
	LPC_I2C0->I2SCLL = 125;

	uart_init();

	can_init();

	LPC_GPIO2->FIODIR |= LEDS_R_A | LEDS_R_B | LEDS_R_ROTATION | LEDS_R_UART | LEDS_T_UART;
	LPC_GPIO2->FIOCLR = LEDS_R_A | LEDS_R_B | LEDS_R_ROTATION | LEDS_R_UART | LEDS_T_UART;

	LPC_GPIOINT->IO2IntEnR = TOUCH_INT_GPIO;
	NVIC_EnableIRQ(EINT3_IRQn);

	callback_reset();
	callback_add(CAN_IRQn, &can_dashboard_recieve_handler);
	callback_add(TOUCH_CALLBACK, &touch_process_int);

	char buffer[sizeof(uart_telemetry)];
	internal_message_counter = 0;

	dashboard_display_screen_saver();


	touch_button_t touch_buttons[] = {
		touch_create_button("ABS +", 0, 160, 120, 50, 255, 0, 0, &abs_touch_button_plus),
		touch_create_button("ABS -", 0, 220, 120, 50, 255, 255, 0, &abs_touch_button_minus),
		touch_create_button("TC -", 120, 160, 120, 50, 0, 255, 0, &tc_touch_button_plus),
		touch_create_button("TC +", 120, 220, 120, 50, 0, 255, 255, &tc_touch_button_minus)
	};

	int i = 0;
	int local_counter = 0;
	int uart_watchdog  = get_ms_counter(); // used to timeout on host

	while(1)
	{

		if(uart_get_char(&buffer[i])){
			uart_watchdog = get_ms_counter();
			i++;
		}

		if(i >= sizeof(uart_telemetry)){

			uart_clear();


			LPC_GPIO2->FIOSET = LEDS_R_UART;

			i = 0;

			uart_telemetry* telem = (uart_telemetry*)buffer;

			if(internal_message_counter != telem->message_counter)
			{
				// this means a paquet was lost
				dashboard_reset_uart_communication();
				continue;
			}

			if(telem->stop_display && !is_screen_saver_dashboard_displaying)
			{
				dashboard_display_screen_saver();
				touch_init(0, 0);
				continue;
			}

			if(is_screen_saver_dashboard_displaying)
			{
				gui_clear_screen_saver(50, 170, "Dashboard");
				is_screen_saver_dashboard_displaying = false;
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


			internal_message_counter++;

			LPC_GPIO2->FIOCLR = LEDS_R_UART;

			if(internal_message_counter==1){
				touch_init(touch_buttons, GET_BUTTONS_COUNT(touch_buttons));
			}

		}

		callback_do();


		// send via uart the inputs if more than 20 ms have passed since last tranmission
		if((local_counter + 20) < get_ms_counter()){
			LPC_GPIO2->FIOSET = LEDS_T_UART;

			uart_send((char*)&inputs, sizeof(inputs));
			SYNC

			LPC_GPIO2->FIOCLR = LEDS_T_UART;

			local_counter += 20;
		}

		if(((uart_watchdog + 1000) < get_ms_counter()) && !is_screen_saver_dashboard_displaying){

			uart_clear();

			dashboard_display_screen_saver();

		}
	}
}

void can_dashboard_recieve_handler()
{
	uint32_t received_id;
	uint8_t* received_data;

	can_get_message(&received_id, &received_data);

	if(received_data[0] == CAN_BTN_A_DATA_NUMBER)
	{
		LPC_GPIO2->FIOSET = LEDS_R_A;
		if(received_data[1] == 1) {
			draw_square(0, 280, 8*sizeof("A pressed"), 12, 0, 0, 0);
			inputs.accelerator = 0;
		}
		else if(received_data[1] == 0) {
			write_text_small_font("A pressed", 31, 0, 0, 0, 0, 0, 0, 280, 240);
			inputs.accelerator = 1;
		}

		LPC_GPIO2->FIOCLR = LEDS_R_A;
	}
	else if(received_data[0] == CAN_BTN_B_DATA_NUMBER)
	{
		LPC_GPIO2->FIOSET = LEDS_R_B;
		if(received_data[1] == 1){
			draw_square(0, 300, 8*sizeof("B pressed"), 12, 0, 0, 0);
			inputs.brake = 0;
		}
		else if(received_data[1] == 0){
			write_text_small_font("B pressed", 31, 0, 0, 0, 0, 0, 0, 300, 240);
			inputs.brake = 1;
		}

		LPC_GPIO2->FIOCLR = LEDS_R_B;
	}
	else if(received_data[0] == CAN_WHEEL_ROTATION)
	{
		LPC_GPIO2->FIOSET = LEDS_R_ROTATION;
		// 255 is protected value
		inputs.rotation = received_data[1] == 255 ? 254 : received_data[1];
		LPC_GPIO2->FIOCLR = LEDS_R_ROTATION;
	}
}

void abs_touch_button_plus(int state){
	if(state)
		inputs.abs |= 1;
	else
		inputs.abs &= ~(1);
}

void abs_touch_button_minus(int state){
	if(state)
		inputs.abs |= 2;
	else
		inputs.abs &= ~(2);
}

void tc_touch_button_plus(int state){
	if(state)
		inputs.tc |= 1;
	else
		inputs.tc &= ~(1);
}

void tc_touch_button_minus(int state){
	if(state)
		inputs.tc |= 2;
	else
		inputs.tc &= ~(2);
}

void dashboard_reset_uart_communication()
{
	uart_clear();

	uart_send("RESET", sizeof(inputs));
	SYNC

	//Send reset screen to the wheel
	uint8_t data[8] = { 0 };
	data[0] = CAN_RESET_CMD_NUMBER;
	can_send(0, 0, 1, data);
	dashboard_main();
}
