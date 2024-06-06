/*
 * dashboard_gui.h
 *
 *  Created on: Jun 1, 2024
 *      Author: joachim
 */

#ifndef LIB_CUSTOM_GUI_H_
#define LIB_CUSTOM_GUI_H_

void gui_reset_values();
void gui_draw_accel_bar(int x, int y, int size_x, int size_y, float value);
void gui_draw_brake_bar(int x, int y, int size_x, int size_y, float value);
void gui_draw_lap_time(int x, int y, int value);
void gui_draw_speed(int x, int y, int value);
void gui_draw_screen_saver(int x, int y, char* text);
void gui_clear_screen_saver(int x, int y, char* text);
void gui_draw_speedometer(int x, int y, int radius, int speed);

#endif /* LIB_CUSTOM_GUI_H_ */
