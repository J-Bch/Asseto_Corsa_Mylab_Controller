/*
 * touch.h
 *
 *  Created on: Jun 18, 2024
 *      Author: david
 */

#ifndef LIB_TOUCH_H_
#define LIB_TOUCH_H_

#define TOUCH_CALLBACK 	42
#define TOUCH_ADDR		0x38
#define TOUCH_INT_GPIO	(1 << 11)

typedef struct _touch_button_t {
	int x;
	int y;
	int width;
	int height;
	int r;
	int g;
	int b;
	void (*handler)(int state);
	char text[42];
	int state;
} touch_button_t;

#define GET_BUTTONS_COUNT(buttons)(sizeof(buttons) / sizeof(touch_button_t))

touch_button_t touch_create_button(char* text, int x, int y, int width, int height, int r, int g, int b, void (handler(int state)));
void touch_init(touch_button_t* buttons, int size);
void touch_process_int();

#endif /* LIB_TOUCH_H_ */
