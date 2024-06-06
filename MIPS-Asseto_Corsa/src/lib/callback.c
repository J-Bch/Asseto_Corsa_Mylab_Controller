/*
 * callback.c
 *
 *  Created on: Apr 18, 2024
 *      Author: joachim
 */

#include "callback.h"
#include <stdint.h>

#define CALLBACK_SIZE 35

typedef struct callback
{
	uint32_t callback_id; // irq number
	volatile uint32_t flag;
	void (*handler)(void); // pointer to callback function
}callback_t;

callback_t callbacks[CALLBACK_SIZE];

uint32_t callback_count = 0;


void callback_add(uint32_t callback_id, void(handler(void)))
{
	callbacks[callback_count].callback_id = callback_id;
	callbacks[callback_count].handler = handler;
	callbacks[callback_count].flag = 0;
	callback_count++;
}


void callback_setflag(uint32_t callback_id)
{
	int i;
	for(i = 0; i < CALLBACK_SIZE; i++)
	{
		if(callbacks[i].callback_id == callback_id)
		{
			callbacks[i].flag = 1;
		}
	}
}

void callback_do()
{
	int i;
	for(i = 0; i < CALLBACK_SIZE; i++)
	{
		if(callbacks[i].flag)
		{
			callbacks[i].flag = 0;
			callbacks[i].handler();
		}
	}
}
