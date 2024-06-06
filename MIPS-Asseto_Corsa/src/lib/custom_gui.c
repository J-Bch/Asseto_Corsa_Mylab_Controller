/*
 * dashboard_gui.c
 *
 *  Created on: Jun 1, 2024
 *      Author: joachim
 */

#include "custom_gui.h"

#include <stdlib.h>
#include "lcd.h"
#include <string.h>
#include <math.h>
#include <stdbool.h>

int previous_accel_bar_height = 0;
int previous_brake_bar_height = 0;
int previous_lap_time = 0;
int previous_speed = 0;

int previous_x_end_speedo = 0;
int previous_y_end_speedo = 0;

bool first_draw_speedo = true;

void gui_reset_values()
{
	previous_accel_bar_height = 0;
	previous_brake_bar_height = 0;
	previous_lap_time = 0;
	previous_speed = 0;

	previous_x_end_speedo = 0;
	previous_y_end_speedo = 0;
	first_draw_speedo = true;
}

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

void gui_draw_lap_time(int x, int y, int value)
{
	float value_seconds = value/1000;

	if(value_seconds < previous_lap_time) //erase the previous counter if for example lap finished, to clear numbers
		draw_square(x, y, 6*8, 12, 0, 0, 0);

	char text[6] = {0};
	itoa(value_seconds, text, 10);
	write_text_small_font(text, 31, 0, 0, 0, 0, 0, x, y, 240);
	previous_lap_time = value_seconds;
}

void gui_draw_speed(int x, int y, int value)
{

	if(value < previous_speed) //erase the previous counter if value changed number of digits
		draw_square(x, y, 6*8, 12, 0, 0, 0);

	char text[6] = {0};
	itoa(value, text, 10);
	write_text_small_font(text, 31, 0, 0, 0, 0, 0, x, y, 240);
	previous_speed = value;
}


void gui_draw_screen_saver(int x, int y, char* text)
{
	write_text_small_font(text, 31, 0, 0, 0, 0, 0, x, y, 240);
}

void gui_clear_screen_saver(int x, int y, char* text)
{
	draw_square(x, y, strlen(text)*8, 12, 0, 0, 0);
}

void calculate_end_point(double x, double y, double radius, double max_v, double v, double *x_end, double *y_end)
{
    // Calcul de l'angle theta en radians, ajusté pour varier de -pi/2 à pi/2
    double theta = (v / max_v -1) * 3.14;

    // Calcul des coordonnées du point d'arrivée
    *x_end = x + radius * cos(theta);
    *y_end = y + radius * sin(theta);
}

void gui_draw_speedometer(int x, int y, int radius, int speed)
{
	if(first_draw_speedo)
	{
		first_draw_speedo = false;

		write_text_small_font("0", 31, 0, 0, 0, 0, 0, (x - radius) - 5, y - 5 , 240);
		write_text_small_font("200", 31, 0, 0, 0, 0, 0, (x + radius) + 5, y -5 , 240);

		write_text_small_font("100", 31, 0, 0, 0, 0, 0, x - 5, (y - radius) - 10 , 240);
		write_text_small_font("300", 31, 0, 0, 0, 0, 0, x - 5, (y + radius) + 10, 240);
	}

	double x_end;
	double y_end;

	draw_line(x, y, previous_x_end_speedo, previous_y_end_speedo, 0, 0, 0);

	calculate_end_point(x, y, radius, 200, speed, &x_end, &y_end);
    draw_line(x, y, (int)x_end, (int)y_end, 0, 0, 31);

    previous_x_end_speedo = x_end;
    previous_y_end_speedo = y_end;
}



