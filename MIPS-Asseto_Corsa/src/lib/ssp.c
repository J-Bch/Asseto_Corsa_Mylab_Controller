/*
 * ssp.c
 *
 *  Created on: Feb 23, 2024
 *      Author: joachim
 */

#include "ssp.h"

#define PCSSP0 21
#define SSP0_CLK 30
#define SSP0_MOSI 4
#define SSP0_MISO 2
#define PCLK_SSP0 10
#define SSE 1

void ssp_init(uint8_t data_size_select, uint8_t scr)
{
	LPC_SC->PCONP |= (0b1 << PCSSP0);

	// Config SSP0 clock to 10MHz
	LPC_SC->PCLKSEL1 = (LPC_SC->PCLKSEL1 & (~(3 << 10))) | (1 << 10);
	LPC_SSP0->CPSR = 10; // SSP0 clock = PCLK_SSP0 / 5 = 50e6 / 5 = 10e6

	LPC_PINCON->PINSEL0 =  ((LPC_PINCON->PINSEL0 & ( ~(3 << SSP0_CLK) )) | (0b10 << SSP0_CLK));
	LPC_PINCON->PINSEL1 =  ((LPC_PINCON->PINSEL1 & ( ~(3 << SSP0_MISO) )) | (0b10 << SSP0_MISO));
	LPC_PINCON->PINSEL1 =  ((LPC_PINCON->PINSEL1 & ( ~(3 << SSP0_MOSI) )) | (0b10 << SSP0_MOSI));
	LPC_SSP0->CR0 = (data_size_select & 15) | (scr << 8);

	LPC_SSP0->CR1 = (0b1 << SSE);
}

void ssp_send_buf(uint8_t *buf, uint32_t length)
{
	for(int i = 0; i < length; i++)
	{
		//transmit fifo full
		while(!(LPC_SSP0->SR & (0b1 << 1))){}

		LPC_SSP0->DR = buf[i];
	}

}

void ssp_receive_buf(uint8_t *buf, uint32_t length)
{
	for(int i = 0; i < length; i++)
	{
			while(!(LPC_SSP0->SR & (0b1 << 4))){}

			buf[i] = LPC_SSP0->DR;
	}
}


