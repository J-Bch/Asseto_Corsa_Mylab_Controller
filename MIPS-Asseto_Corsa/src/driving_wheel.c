/*
 * driving_wheel.c
 *
 *  Created on: May 31, 2024
 *      Author: joachim
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "LPC17xx.h"
#include "driving_wheel.h"
#include "lib/accelerometer.h"
#include "lib/can.h"
#include "lib/callback.h"
#include "lib/custom_gui.h"
#include "lib/lcd.h"
#include "lib/i2c.h"
#include "lib/buttons.h"


#define CAR_MAX_RPM 18000



void can_wheel_recieve_handler();
void btn_a_rising_handler();
void btn_b_rising_handler();
void btn_a_falling_handler();
void btn_b_falling_handler();
void send_wheel_rotation();

bool is_screen_saver_wheel_displaying = false;

void wheel_display_screen_saver(){
	whipe_screen();
	gui_reset_values();
	gui_draw_screen_saver(50, 170, "Driving Wheel");
	is_screen_saver_wheel_displaying = true;
}



int can_watchdog = 0;

void driving_wheel_main()
{
	btns_init();
	lcd_init();
	can_init();
	i2c_init();
	delay(100); //wait for the accelerometer to boot
	accelerometer_init();
	accelerometer_config();

	callback_reset();
	callback_add(CAN_IRQn, &can_wheel_recieve_handler);
	callback_add(BTN_A_RISING_CALLBACK, &btn_a_rising_handler);
	callback_add(BTN_B_RISING_CALLBACK, &btn_b_rising_handler);
	callback_add(BTN_A_FALLING_CALLBACK, &btn_a_falling_handler);
	callback_add(BTN_B_FALLING_CALLBACK, &btn_b_falling_handler);


	wheel_display_screen_saver();

	int local_counter = 0;


	while(1)
	{
		callback_do();



		// send via uart the inputs if more than 20 ms have passed since last tranmission
		if((local_counter + 20) < get_ms_counter()){

			send_wheel_rotation();

			local_counter += 20;
		}

		if(((can_watchdog + 1000) < get_ms_counter()) && !is_screen_saver_wheel_displaying){

			wheel_display_screen_saver();

		}
	}
}


void can_wheel_recieve_handler()
{
	if(is_screen_saver_wheel_displaying)
	{
		gui_clear_screen_saver(50, 170, "Driving Wheel");
		is_screen_saver_wheel_displaying = false;
	}


	uint32_t received_id;
	uint8_t* received_data;

	can_get_message(&received_id, &received_data);

	if(received_data[0] == CAN_SPEED_DATA_NUMBER)
	{
		uint32_t speed = (received_data[1] + (received_data[2] << 8) + (received_data[3] << 16) + (received_data[4] << 24));
		gui_draw_speed(80, 30, speed);

		gui_draw_speedometer(120, 160, 80, speed);
	}
	else if(received_data[0] == CAN_RPM_DATA_NUMBER)
	{
		uint32_t engine_RPM = (received_data[1] + (received_data[2] << 8) + (received_data[3] << 16) + (received_data[4] << 24));

		gui_display_shift_indicator_leds(engine_RPM, CAR_MAX_RPM);
	}

	can_watchdog = get_ms_counter();
}

void btn_a_rising_handler()
{
	uint8_t data[8] = { 0 };
	data[0] = CAN_BTN_A_DATA_NUMBER;
	data[1] = 1;
	can_send(0, 0, 2, data);
}

void btn_b_rising_handler()
{
	uint8_t data[8] = { 0 };
	data[0] = CAN_BTN_B_DATA_NUMBER;
	data[1] = 1;
	can_send(0, 0, 2, data);
}

void btn_a_falling_handler()
{
	uint8_t data[8] = { 0 };
	data[0] = CAN_BTN_A_DATA_NUMBER;
	data[1] = 0;
	can_send(0, 0, 2, data);
}

void btn_b_falling_handler()
{
	uint8_t data[8] = { 0 };
	data[0] = CAN_BTN_B_DATA_NUMBER;
	data[1] = 0;
	can_send(0, 0, 2, data);
}

#define ABS(x)(x > 0 ? x : -x)
#define ANGLE_FACTOR 40.584510488 // 255 / (2 pi), 255 as it is max value
#define PI 3.141592654


void send_wheel_rotation(){

	static float accelerometer_values_real_world[3];

	accelerometer_get_real_world_value(accelerometer_values_real_world);

	float x = accelerometer_values_real_world[0];
	float y = accelerometer_values_real_world[1];
	float z = accelerometer_values_real_world[2];

	float rotation;

	// dead zone and avoid using data when wheel is tilted
	if((ABS(z) > 0.4 | ABS(x) > 0.97)){

		// if titled or perfectly vertical

		rotation = 128;

	} else {

		rotation = ((acos(x / sqrt(x * x + y * y)) * (y > 0 ? -1 : 1)) + PI ) * ANGLE_FACTOR;
	}

	uint8_t data[8] = { 0 };
	data[0] = CAN_WHEEL_ROTATION;
	data[1] = (uint8_t)rotation;
	can_send(0, 0, 2, data);
}
