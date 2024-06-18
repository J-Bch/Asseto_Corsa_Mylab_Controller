/*
 * touch.c
 *
 *  Created on: Jun 18, 2024
 *      Author: david
 */

#include "string.h"

#include "touch.h"
#include "lcd.h"
#include "i2c.h"
#include "stdio.h"

#define BUTTON_PADDING 10
#define TOUCH_ADDR		0x38
#define TOUCH_START_REG	0x02


touch_button_t touch_create_button(char* text, int x, int y, int width, int height, int r, int g, int b, void (handler(void))){

	draw_square(x, y, width, height, b, r, g);
	write_text_small_font(text, r, g, b, b, r, g, x + BUTTON_PADDING, y, width - (BUTTON_PADDING * 2));

	touch_button_t button = {
		.x = x,
		.y = y,
		.width = width,
		.height = height,
		.r = r,
		.g = g,
		.b = b,
		.handler = handler,
		.state = 0
	};

	return button;
}



touch_button_t* g_buttons = 0;
int g_size = 0;

void touch_init(touch_button_t* buttons, int size){
	g_buttons = buttons;
	g_size = size;
}

#define EVENT_PRESS_DOWN 0

void touch_process_int(){

	// get touches count
	uint8_t buffer[14];
	i2c_read_bytes(TOUCH_ADDR, 2, sizeof(buffer), buffer);

	int touch_count = buffer[0] & 0xf;

	int p1_event = ((buffer[1] >> 6 ) & 0x3);
	int p1_x = ((buffer[1] & 0xf) << 8) | buffer[2];
	int p1_y = ((buffer[3] & 0xf) << 8) | buffer[4];

	int p2_event = ((buffer[7] >> 6 ) & 0x3);
	int p2_x = ((buffer[7] & 0xf) << 8) | buffer[8];
	int p2_y = ((buffer[11] & 0xf) << 8) | buffer[12];


	if(p1_event == EVENT_PRESS_DOWN){
		printf("p1 %d %d %d\n", p1_event, p1_x, p1_y);
	}

	if(p2_event == EVENT_PRESS_DOWN){
		printf("p2 %d %d %d\n", p2_event, p2_x, p2_y);
	}

	// for (int i = 0; i < g_size; i++)
	// {
		
	// }
	
}
