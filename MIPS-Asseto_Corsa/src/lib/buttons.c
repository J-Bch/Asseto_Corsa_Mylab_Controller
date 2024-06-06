/*
 * buttons.c
 *
 *  Created on: Jun 6, 2024
 *      Author: joachim
 */

#ifndef LIB_BUTTONS_C_
#define LIB_BUTTONS_C_

#include <stdio.h>
#include "LPC17xx.h"


#define BTN_B 19 // P0.19
#define BTN_A 10 // P2.10

//if(((LPC_GPIO0->FIOPIN >> BTN_B) & 0b1) == 0) // BTN B pressed

void btns_init()
{
	LPC_GPIO0->FIODIR &= (~(0b1) << BTN_B); //set btn b to input
	LPC_GPIO2->FIODIR &= (~(0b1) << BTN_A); //set btn a to input

	//clear previous interrupts
	LPC_GPIOINT->IO0IntClr = (0b1 << BTN_B);
	LPC_GPIOINT->IO2IntClr = (0b1 << BTN_A);

	LPC_GPIOINT->IO0IntEnR = (0b1 << BTN_B); //BTN B rising interrupt enable
	LPC_GPIOINT->IO0IntEnF = (0b1 << BTN_B); //BTN B falling interrupt enable

	LPC_GPIOINT->IO2IntEnR = (0b1 << BTN_A); //BTN A rising interrupt enable
	LPC_GPIOINT->IO2IntEnF = (0b1 << BTN_A); //BTN A falling interrupt enable

	NVIC_EnableIRQ(EINT3_IRQn);
}



void EINT3_IRQHandler()
{

	//BTN B rising interrupt
	if(LPC_GPIOINT->IO0IntStatR == (0b1 << BTN_B))
	{
		LPC_GPIOINT->IO0IntClr = (0b1 << BTN_B);

		printf("B rising\n");
	}

	//BTN B falling interrupt
	if(LPC_GPIOINT->IO0IntStatF == (0b1 << BTN_B))
	{
		LPC_GPIOINT->IO0IntClr = (0b1 << BTN_B);

		printf("B falling\n");
	}



	//BTN A rising interrupt
	if(LPC_GPIOINT->IO2IntStatR == (0b1 << BTN_A))
	{
		LPC_GPIOINT->IO2IntClr = (0b1 << BTN_A);

		printf("A rising\n");
	}

	//BTN A falling interrupt
	if(LPC_GPIOINT->IO2IntStatF == (0b1 << BTN_A))
	{
		LPC_GPIOINT->IO2IntClr = (0b1 << BTN_A);

		printf("A falling\n");
	}

}

#endif /* LIB_BUTTONS_C_ */
