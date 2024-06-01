/*
 * dashboard_gui.c
 *
 *  Created on: Jun 1, 2024
 *      Author: joachim
 */


#include "dashboard_gui.h"
#include "lcd.h"

int previous_accel_bar_height = 0;
int previous_brake_bar_height = 0;

void gui_draw_accel_bar(int x, int y, int size_x, int size_y, float value)
{
	int height = (int)(value * size_y);

	if(previous_accel_bar_height > height)
	{
		draw_square(x, y-previous_accel_bar_height, size_x, previous_accel_bar_height - height, 0, 0, 0);
	}
	else
	{
		draw_square(x, y - height, size_x, height - previous_accel_bar_height, 31, 0, 0);
	}


	previous_accel_bar_height = height;
}

void gui_draw_brake_bar(int x, int y, int size_x, int size_y, float value)
{
	int height = (int)(value * size_y);

	if(previous_brake_bar_height > height)
	{
		draw_square(x, y-previous_brake_bar_height, size_x, previous_brake_bar_height - height, 0, 0, 0);
	}
	else
	{
		draw_square(x, y - height, size_x, height - previous_brake_bar_height, 0, 0, 31);
	}


	previous_brake_bar_height = height;
}
