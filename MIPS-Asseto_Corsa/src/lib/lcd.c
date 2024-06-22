/*
 * lcd.c
 *
 *  Created on: Feb 29, 2024
 *      Author: joachim
 */

#include <math.h>

#include "ssp.h"
#include "lcd.h"
#include "Ecran_LCD_ILI9341_Initial.h"
#include "LPC17xx.h"
#include "fonts.h"

#define SSP0_CLK 15
#define SSP0_MOSI 18
#define SSP0_MISO 17
#define LCD_DC 30
#define nCS_LCD 16
#define BL_ctrl 18
#define CIRCLE_THRESHOLD 3

int ms_counter = 0;


void lcd_init()
{
	LPC_GPIO1->FIODIR |=
			(0b1 << LCD_DC) |
			(0b1 << BL_ctrl);

	LPC_GPIO0->FIODIR |= (0b1 << nCS_LCD);

	LPC_GPIO1->FIOPIN |= (0b1 << BL_ctrl);

	//chip select
	LPC_GPIO0->FIOPIN &= ~(0b1 << nCS_LCD);

	LPC_TIM1->MR0 = 25000; /* [ticks] */ //(1*10^-3)/(1/25*10^6)
	LPC_TIM1->MCR = 3; /* Interrupt and Reset when TC = MR0 */
	NVIC_EnableIRQ(TIMER1_IRQn); /* enable timerx line of VIC */

	LPC_TIM1->TCR = 1;

	//7 is 8b, 0 because scr +1 by default
	ssp_init(7, 0);
	ILI9341_Initial();

	wipe_screen();
}

void lcd_write_cmd(uint8_t cmd)
{
	//Busy because buffered so can change mode of previous data if done wrong
	while((LPC_SSP0->SR & (0b1 << 4))){}

	LPC_GPIO1->FIOCLR = 1 << LCD_DC;

	ssp_send_buf(&cmd, 1);
}

void lcd_write_data(uint8_t data)
{
	//Busy
	while((LPC_SSP0->SR & (0b1 << 4))){}

	LPC_GPIO1->FIOSET = 1 << LCD_DC;

	ssp_send_buf(&data, 1);
}

void delay(int ms)
{
	int start = ms_counter;

	while(ms_counter - start < ms)
	{
	}
}

void TIMER1_IRQHandler() /* timer x interrupt handler */
{
	LPC_TIM1->IR = 1;/* clear timer interrupt flag */
	ms_counter++;
}

//R:5b G:6b B:5b
void write_color(int r, int g, int b)
{
	uint8_t first_data = r << 3;

	first_data |= g >> 3;

	uint8_t second_data = g << 5;

	second_data |= b;

	lcd_write_data(first_data);
	lcd_write_data(second_data);
}

void wipe_screen()
{
	set_draw_window(0, 0, 240, 320);

	lcd_write_cmd(0x2c);

	for(int i = 0; i < 240; i++)
	{
		for(int j = 0; j < 320; j++)
		{
			write_color(0, 0, 0);
		}
	}
}

void set_draw_window(int start_x, int start_y, int size_x, int size_y)
{
	//page 110 doc screen 16 bits for line and 16 for column
	lcd_write_cmd(0x2a);
	lcd_write_data((start_x) >> 8);
	lcd_write_data(start_x & 0x00ff);

	int end_x = start_x + size_x -1;

	lcd_write_data(end_x >> 8);
	lcd_write_data(end_x & 0x00ff);

	lcd_write_cmd(0x2b);
	lcd_write_data((start_y) >> 8);
	lcd_write_data(start_y & 0x00ff);

	int end_y = start_y + size_y -1;

	lcd_write_data(end_y >> 8);
	lcd_write_data(end_y & 0x00ff);
}

void write_letter_small_font(char letter, int r, int g, int b, int back_r, int back_g, int back_b, int x, int y)
{
	set_draw_window(x, y, 8, 12);

	lcd_write_cmd(0x2c);

	int shifted_letter = letter -32;

	for(int i = shifted_letter*12 + 4; i < shifted_letter*12 + 16; i++)
	{
		for(int j = 8; j > 0; j--)
		{
			if(SmallFont[i] & (1 << (j-1)))
			{
				write_color(r, g, b);
			}
			else
			{
				write_color(back_r, back_g, back_b);
			}

		}
	}


}

void write_text_small_font(char* text, int r, int g, int b, int back_r, int back_g, int back_b, int x, int y, int max_x)
{
	int new_x = x;
	int new_y = y;

	int i = 0;
	while(text[i] != '\0')
	{
		//dont add padding if first letter
		if(i != 0)
		{
			new_x += 8;
		}

		if(new_x > max_x-8)
		{
			new_x = x;
			new_y += 12;
		}

		write_letter_small_font(text[i], r, g, b, back_r, back_g, back_b, new_x, new_y);
		i++;
	}
}

void write_line(int x_start, int x_size, int y_start, int y_size, int r, int g, int b)
{
	set_draw_window(x_start, y_start, x_size, y_size);

	lcd_write_cmd(0x2c);

	for(int i = 0; i < x_size; i++)
	{
		for(int j = 0; j < y_size; j++)
		{
			write_color(r, g, b);
		}
	}
}

void draw_square(int x, int y, int size_x, int size_y, int r, int g, int b)
{
	set_draw_window(x, y, size_x, size_y);

	lcd_write_cmd(0x2c);

	for(int i = 0; i < size_x; i++)
	{
		for(int j = 0; j < size_y; j++)
		{
			write_color(r, g, b);
		}
	}
}

void put_pixel(int x, int y, int r, int g, int b)
{
	set_draw_window(x, y, 1, 1);
	lcd_write_cmd(0x2c);
	write_color(r, g, b);
}

void set_ms_counter(int value)
{
	ms_counter = value;
}

void reset_ms_counter()
{
	ms_counter = 0;
	LPC_TIM1->TCR = 1; //resets the counter
}

int get_ms_counter()
{
	return ms_counter;
}

//Bresenham’s Line Drawing Algorithm
void draw_line(int x0, int y0, int x1, int y1, int r, int g, int b)
{
    int dx = fabs(x1 - x0);
    int dy = fabs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1; //direction of the line
    int sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;
    int e2;

    while(x0 != x1 || y0 != y1)
    {
        draw_square(x0, y0, 1, 1, r, g, b);

        e2 = err;
        if (e2 > -dx) // if error is too big, increment value (it means we have to put another pixel to reduce error
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dy)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void draw_circle(uint16_t r, uint16_t g, uint16_t b , uint16_t ref_x, uint16_t ref_y, uint16_t radius, uint16_t x, uint16_t y, uint16_t w, uint16_t h){

	for (int u = 0; u < w; ++u) {
		for (int v = 0; v < h; ++v) {

			int _u = u + x - ref_x;
			int _v = v + y - ref_y;

			uint16_t dist = (uint16_t)sqrt(_u * _u + _v * _v);

			if(dist < (radius + CIRCLE_THRESHOLD) && dist > (radius - CIRCLE_THRESHOLD)){
				put_pixel(x + u, y + v, r, g, b);
			}
		}
	}

}






