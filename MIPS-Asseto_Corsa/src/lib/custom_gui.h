/*
 * dashboard_gui.h
 *
 *  Created on: Jun 1, 2024
 *      Author: joachim
 */

#ifndef LIB_CUSTOM_GUI_H_
#define LIB_CUSTOM_GUI_H_

void gui_draw_accel_bar(int x, int y, int size_x, int size_y, float value);
void gui_draw_brake_bar(int x, int y, int size_x, int size_y, float value);
void gui_draw_lap_time(int x, int y, int value);
void gui_draw_speed(int x, int y, int value);

#endif /* LIB_CUSTOM_GUI_H_ */
