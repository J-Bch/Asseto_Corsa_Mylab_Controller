/*
 * lcd.h
 *
 *  Created on: Feb 29, 2024
 *      Author: joachim
 */

#ifndef LCD_H_
#define LCD_H_
#include <stdint.h>

#define SCREEN_HEIGHT 320
#define SCREEN_WIDTH 240

void lcd_init();
void lcd_write_cmd(uint8_t cmd);
void lcd_write_data(uint8_t data);
void delay(int ms);
void write_color(int r, int g, int b);
void whipe_screen();
void set_draw_window(int start_x, int start_y, int size_x, int size_y);
void write_text_small_font(char* text, int r, int g, int b, int back_r, int back_g, int back_b, int x, int y, int max_x);
void write_letter_small_font(char letter, int r, int g, int b, int back_r, int back_g, int back_b, int x, int y);
void write_line(int x_start, int x_size, int y_start, int y_size, int r, int g, int b);
void draw_square(int x, int y, int size_x, int size_y, int r, int g, int b);
void put_pixel(int x, int y, int r, int g, int b);
void set_ms_counter(int value);
void reset_ms_counter();
int get_ms_counter();
void draw_line(int x0, int y0, int x1, int y1, int r, int g, int b);
void draw_circle(uint16_t r, uint16_t g, uint16_t b , uint16_t ref_x, uint16_t ref_y, uint16_t radius, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
#endif /* LCD_H_ */
