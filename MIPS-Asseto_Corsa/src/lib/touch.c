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

#define EVENT_PRESS_DOWN 0
#define LIFT_UP			 1
#define TOUCH_MAX_COUNT 2


typedef struct _touch_event_t {
	int type;
	int x;
	int y;
} touch_event_t;


// global array of the registered buttons, initialised by touch_init()
touch_button_t* g_buttons = 0;
// global count of buttons
int g_size = 0;


void touch_print_button(touch_button_t* button){

	int a = button->state ? (button->r / 16) : button->r;
	int b = button->state ? (button->g / 16) : button->g;
	int c = button->state ? (button->b / 16) : button->b;

	draw_square(button->x, button->y, button->width, button->height, a, b, c);
	write_text_small_font(button->text,
			0,0,0,
			a,b,c,
			button->x + BUTTON_PADDING,
			button->y + (button->height/ 2) - 7,
			button->x + button->width - (BUTTON_PADDING * 2));


}


void touch_print_all_buttons(){
	for (int i = 0; i < g_size; ++i) {
		touch_print_button(&g_buttons[i]);
	}
}

touch_button_t touch_create_button(char* text, int x, int y, int width, int height, int r, int g, int b, void (handler(int state))){

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

	memcpy(button.text, text, strlen(text));

	return button;
}


void touch_init(touch_button_t* buttons, int size){
	g_buttons = buttons;
	g_size = size;

	touch_print_all_buttons();
}


uint8_t buffer[14];
touch_event_t touchs_event[TOUCH_MAX_COUNT];

void touch_process_int(){

	// get touches count

	i2c_read_bytes(TOUCH_ADDR, 2, sizeof(buffer), buffer);


	touchs_event[0].type = ((buffer[1] >> 6 ) & 0x3);
	touchs_event[0].x = SCREEN_WIDTH - (((buffer[1] & 0xf) << 8) | buffer[2]);
	touchs_event[0].y = SCREEN_HEIGHT - (((buffer[3] & 0xf) << 8) | buffer[4]);

	touchs_event[1].type = ((buffer[7] >> 6 ) & 0x3);
	touchs_event[1].x = SCREEN_WIDTH - (((buffer[7] & 0xf) << 8) | buffer[8]);
	touchs_event[1].y = SCREEN_HEIGHT - (((buffer[11] & 0xf) << 8) | buffer[12]);


	 for (int i = 0; i < g_size; i++){

		for (int j = 0; j < TOUCH_MAX_COUNT; ++j) {

			if((touchs_event[j].type == EVENT_PRESS_DOWN) || (touchs_event[j].type == LIFT_UP)){

				if(
						(touchs_event[j].x > g_buttons[i].x) &&
						(touchs_event[j].x < (g_buttons[i].x + g_buttons[i].width)) &&
						(touchs_event[j].y > g_buttons[i].y) &&
						(touchs_event[j].y < (g_buttons[i].y + g_buttons[i].height))
				  )
				{
					g_buttons[i].state = !touchs_event[j].type;
					g_buttons[i].handler(g_buttons[i].state);
					touch_print_button(&g_buttons[i]);
				}

			}

		}

	 }
	
}
