/*
 * driving_wheel.c
 *
 *  Created on: May 31, 2024
 *      Author: joachim
 */

#include "LPC17xx.h"
#include "driving_wheel.h"
#include "lib/can.h"
#include "lib/callback.h"


void can_wheel_recieve_handler();

void driving_wheel_main()
{
	can_init();

	callback_add(CAN_IRQn, &can_wheel_recieve_handler);
}


void can_wheel_recieve_handler()
{

}
