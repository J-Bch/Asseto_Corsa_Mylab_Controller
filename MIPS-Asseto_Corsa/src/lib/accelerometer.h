/*
 * accelerometer.h
 *
 *  Created on: Mar 8, 2024
 *      Author: joachim
 */

#ifndef ACCELEROMETER_H_
#define ACCELEROMETER_H_

#include <stdint.h>

void accelerometer_init();
void ask_who_am_i();
void accelerometer_config();
void accelerometer_get_value(int16_t* results);
void accelerometer_get_real_world_value(float* results);
void accelerometer_enable_debug_mode();

#endif /* ACCELEROMETER_H_ */
