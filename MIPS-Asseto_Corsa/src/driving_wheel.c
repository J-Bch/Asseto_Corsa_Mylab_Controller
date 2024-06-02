/*
 * driving_wheel.c
 *
 *  Created on: May 31, 2024
 *      Author: joachim
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "LPC17xx.h"
#include "driving_wheel.h"
#include "lib/accelerometer.h"
#include "lib/can.h"
#include "lib/callback.h"
#include "lib/custom_gui.h"
#include "lib/lcd.h"
#include "lib/i2c.h"



void can_wheel_recieve_handler();

void driving_wheel_main()
{
	lcd_init();
	can_init();
	i2c_init();
	delay(100); //wait for the accelerometer to boot
	accelerometer_init();
	accelerometer_config();

	callback_add(CAN_IRQn, &can_wheel_recieve_handler);

	float accelerometer_values_real_world[3];
//	int16_t accelerometer_can_values[3];
//	uint8_t accelerometer_can_values_splitted[6];

	while(1)
	{
		callback_do();
		accelerometer_get_real_world_value(accelerometer_values_real_world);
//		printf("%f\n", accelerometer_values_real_world[0]);
	}
}


void can_wheel_recieve_handler()
{
	uint32_t received_id;
	uint8_t* received_data;

	can_get_message(&received_id, &received_data);

	uint32_t speed = (received_data[0] + (received_data[1] << 8) + (received_data[2] << 16) + (received_data[3] << 24));
	gui_draw_speed(10, 0, speed);

//
//	printf("%f\n", speed_adjusted);
}
