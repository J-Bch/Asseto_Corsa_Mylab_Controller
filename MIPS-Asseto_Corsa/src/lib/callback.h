/*
 * callback.h
 *
 *  Created on: Apr 18, 2024
 *      Author: joachim
 */

#ifndef CALLBACK_H_
#define CALLBACK_H_

#include <stdint.h>

void callback_add(uint32_t callback_id, void(handler(void)));
void callback_setflag(uint32_t callback_id);
void callback_do();

#endif /* CALLBACK_H_ */
