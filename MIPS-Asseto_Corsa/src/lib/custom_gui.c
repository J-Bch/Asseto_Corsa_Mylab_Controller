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
#include "fonts.h"
#include "LPC17xx.h"

int previous_accel_bar_height = 0;
int previous_brake_bar_height = 0;
int previous_lap_time = 0;
int previous_speed = 0;

int previous_x_end_speedo = 0;
int previous_y_end_speedo = 0;
int previous_RPM_value = 0;

bool first_draw_speedo = true;
bool first_draw_lap_time = true;
bool first_draw_accel_bar = true;
bool first_draw_speed = true;
bool first_display_leds = true;

void gui_reset_values()
{
	previous_accel_bar_height = 0;
	previous_brake_bar_height = 0;
	previous_lap_time = 0;
	previous_speed = 0;

	previous_x_end_speedo = 0;
	previous_y_end_speedo = 0;

	previous_RPM_value = 0;
	first_draw_speedo = true;
	first_draw_lap_time = true;
	first_draw_accel_bar = true;
	first_draw_speed = true;
	first_display_leds = true;
}

void gui_draw_accel_bar(int x, int y, int size_x, int size_y, float value)
{

	int height = (int)(value * size_y);

	if(first_draw_accel_bar)
	{
		first_draw_accel_bar = false;

		write_text_small_font("0", 31, 63, 31, 0, 0, 0, x-SMALL_FONT_WIDTH, y - SMALL_FONT_HEIGHT, 240);
		write_text_small_font("50", 31, 63, 31, 0, 0, 0, x-2*SMALL_FONT_WIDTH, y- (size_y/2)- (SMALL_FONT_HEIGHT/2), 240);
		write_text_small_font("100", 31, 63, 31, 0, 0, 0, x-3*SMALL_FONT_WIDTH, y- size_y, 240);
	}

	if(previous_accel_bar_height > height)
	{
		draw_square(x, y-previous_accel_bar_height, size_x, previous_accel_bar_height - height, 0, 0, 0);
	}
	else
	{
		draw_square(x, y - height, size_x, height - previous_accel_bar_height, 0, 63, 0);
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
		draw_square(x, y - height, size_x, height - previous_brake_bar_height, 31, 0, 0);
	}


	previous_brake_bar_height = height;
}

void gui_draw_lap_time(int x, int y, int value)
{
	if(first_draw_lap_time)
	{
		first_draw_lap_time = false;

		write_text_small_font("Lap time :", 31, 63, 31, 0, 0, 0, x, y, 240);
	}


	float value_seconds = value/1000;

	if(value_seconds < previous_lap_time) //erase the previous counter if for example lap finished, to clear numbers
		draw_square(x + 11*SMALL_FONT_WIDTH, y, 6*SMALL_FONT_WIDTH, 12, 0, 0, 0); //x + 11*8 because of the Lap time text

	char text[6] = {0};
	itoa(value_seconds, text, 10);
	write_text_small_font(text, 31, 0, 0, 0, 0, 0, x + 11*SMALL_FONT_WIDTH, y, 240);
	previous_lap_time = value_seconds;
}

void gui_draw_speed(int x, int y, int value)
{
	if(first_draw_speed)
	{
		first_draw_speed = false;
		write_text_small_font("SPEED :", 31, 63, 31, 0, 0, 0, x, y, 240);
	}

	if(value < previous_speed) //erase the previous counter if value changed number of digits
		draw_square(8*SMALL_FONT_WIDTH+x, y, 6*SMALL_FONT_WIDTH, SMALL_FONT_HEIGHT, 0, 0, 0);

	char text[6] = {0};
	itoa(value, text, 10);
	write_text_small_font(text, 31, 63, 31, 0, 0, 0, 8*SMALL_FONT_WIDTH+x, y, 240);
	previous_speed = value;
}


void gui_draw_screen_saver(int x, int y, char* text)
{
	write_text_small_font(text, 31, 0, 0, 0, 0, 0, x, y, 240);
}

void gui_clear_screen_saver(int x, int y, char* text)
{
	draw_square(x, y, strlen(text)*SMALL_FONT_WIDTH, SMALL_FONT_HEIGHT, 0, 0, 0);
}

void calculate_end_point(int x, int y, int radius, int max_v, int v, int *x_end, int *y_end)
{
    float theta = ((float)v / max_v -1) * 3.14; //calculate the ratio between v and vmax and convert it in a rad angle

    *x_end = x + radius * cos(theta);
    *y_end = y + radius * sin(theta);
}

void gui_draw_speedometer(int x, int y, int radius, int speed)
{
	int x_end;
	int y_end;

	draw_line(x, y, previous_x_end_speedo, previous_y_end_speedo, 0, 0, 0);

	calculate_end_point(x, y, radius, 200, speed, &x_end, &y_end);
    draw_line(x, y, (int)x_end, (int)y_end, 31, 63, 31);

    previous_x_end_speedo = x_end;
    previous_y_end_speedo = y_end;

    if(first_draw_speedo)
	{
		first_draw_speedo = false;

		write_text_small_font("0", 31, 63, 31, 0, 0, 0, (x - radius) - 2*SMALL_FONT_WIDTH, y - (SMALL_FONT_HEIGHT / 2), 240);
		write_text_small_font("200", 31, 63, 31, 0, 0, 0, (x + radius) + SMALL_FONT_WIDTH, y - (SMALL_FONT_HEIGHT / 2), 240);

		write_text_small_font("100", 31, 63, 31, 0, 0, 0, x - (1.5*SMALL_FONT_WIDTH), (y - radius) - 1.5*SMALL_FONT_HEIGHT , 240);
		write_text_small_font("300", 31, 63, 31, 0, 0, 0, x - (1.5*SMALL_FONT_WIDTH), (y + radius) + SMALL_FONT_HEIGHT, 240);

		write_text_small_font("50", 31, 0, 0, 0, 0, 0, (x - (float)radius*0.707) - 2.5*SMALL_FONT_WIDTH, (y - (float)radius*0.707) -SMALL_FONT_HEIGHT, 240);
		write_text_small_font("150", 31, 0, 0, 0, 0, 0, (x + (float)radius*0.707) + 1.5*SMALL_FONT_WIDTH, (y - (float)radius*0.707) -SMALL_FONT_HEIGHT, 240);

		write_text_small_font("250", 31, 0, 0, 0, 0, 0, (x + (float)radius*0.707) + 1.5*SMALL_FONT_WIDTH, (y + (float)radius*0.707), 240);
		write_text_small_font("350", 31, 0, 0, 0, 0, 0, (x - (float)radius*0.707) -4*SMALL_FONT_WIDTH, (y + (float)radius*0.707), 240);

		int circle_radius = radius + 5;
		draw_circle(10, 0, 0, x, y, circle_radius, x-circle_radius, y-circle_radius, 2*circle_radius +1, 2*circle_radius+1);

	}
}

void gui_display_shift_indicator_leds(int engine_RPM, int max_engine_RPM)
{
	if(first_display_leds)
	{
		first_display_leds = false;

		LPC_GPIO2->FIODIR |= (0b11111111); //Set all leds to outputs
		LPC_GPIO2->FIOPIN &= ~(0b11111111); //Turn off all leds
	}

	float ratio = (float)engine_RPM / (max_engine_RPM -500); //-500 to display last led, even if not on the max rpm

	int value = ratio*7;

	LPC_GPIO2->FIOMASK0 = (uint8_t)~(0xff);
	if(value != 0)
		LPC_GPIO2->FIOPIN0 = ((0b1 << (value +1)) -1); //Turn leds based on the value
	else
		LPC_GPIO2->FIOPIN0 = 0;

	LPC_GPIO2->FIOMASK0 = ~(0);


	previous_RPM_value = value;

}



