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

#include "LPC17xx.h"
#include "driving_wheel.h"
#include "lib/accelerometer.h"
#include "lib/can.h"
#include "lib/callback.h"
#include "lib/custom_gui.h"
#include "lib/lcd.h"
#include "lib/i2c.h"
#include "lib/buttons.h"



void can_wheel_recieve_handler();
void btn_a_rising_handler();
void btn_b_rising_handler();
void btn_a_falling_handler();
void btn_b_falling_handler();

bool is_screen_saver_wheel_displaying = false;

void driving_wheel_main()
{
	btns_init();
	lcd_init();
	can_init();
	i2c_init();
	delay(100); //wait for the accelerometer to boot
	accelerometer_init();
	accelerometer_config();

	callback_add(CAN_IRQn, &can_wheel_recieve_handler);
	callback_add(BTN_A_RISING_CALLBACK, &btn_a_rising_handler);
	callback_add(BTN_B_RISING_CALLBACK, &btn_b_rising_handler);
	callback_add(BTN_A_RISING_CALLBACK, &btn_a_falling_handler);
	callback_add(BTN_B_RISING_CALLBACK, &btn_b_falling_handler);

	float accelerometer_values_real_world[3];
//	int16_t accelerometer_can_values[3];
//	uint8_t accelerometer_can_values_splitted[6];

	gui_draw_screen_saver(50, 170, "Driving Wheel");
	is_screen_saver_wheel_displaying = true;

	while(1)
	{
		callback_do();
		accelerometer_get_real_world_value(accelerometer_values_real_world);
//		printf("%f\n", accelerometer_values_real_world[0]);
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

	if(received_data[0] == CAN_RESET_CMD_NUMBER)
	{
		whipe_screen();
		gui_draw_screen_saver(50, 170, "Driving Wheel");
		is_screen_saver_wheel_displaying = true;
		return;
	}
	else if(received_data[0] == CAN_SPEED_DATA_NUMBER)
	{
		uint32_t speed = (received_data[1] + (received_data[2] << 8) + (received_data[3] << 16) + (received_data[4] << 24));
		gui_draw_speed(10, 0, speed);
	}

//
//	printf("%f\n", speed_adjusted);
}

void btn_a_rising_handler()
{
	printf("A rising\n");
}

void btn_b_rising_handler()
{
	printf("B rising\n");
}

void btn_a_falling_handler()
{
	printf("A falling\n");
}

void btn_b_falling_handler()
{
	printf("B falling\n");
}
