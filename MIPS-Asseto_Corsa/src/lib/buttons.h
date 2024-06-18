/*
 * button.h
 *
 *  Created on: Jun 6, 2024
 *      Author: joachim
 */

#ifndef LIB_BUTTONS_H_
#define LIB_BUTTONS_H_

#define BTN_A_RISING_CALLBACK 2000
#define BTN_A_FALLING_CALLBACK 2001
#define BTN_B_RISING_CALLBACK 2002
#define BTN_B_FALLING_CALLBACK 2003

void btns_init();
void buttons_irqhandler();

#endif /* LIB_BUTTONS_H_ */
