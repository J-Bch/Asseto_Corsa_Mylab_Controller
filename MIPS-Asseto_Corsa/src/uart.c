/*
 * uart.c
 *
 *  Created on: Mar 1, 2024
 *      Author: joachim
 */

#include "uart.h"
#include "LPC17xx.h"
#include <stdio.h>

#define PCUART0 3
#define PCLK_UART0 6
#define UART_TX_PINSEL 4
#define UART_RX_PINSEL 6


// LCR register
#define WORD_LENGTH 0
#define STOP_BIT 2
#define PARITY_ENALBLE 3
#define PARITY_SELECT 4
#define BREAK_CONTROL 6
#define DLAB 7

//FCR register
#define FIFO_ENABLE  0
#define RX_FIFO_RESET 1
#define TX_FIFO_RESET 2
#define DMA_MODE 3
#define RX_TRIGGER_LEVEL 6

//IER register
#define RBR_INTERRUPT_ENABLE 0
#define THRE_INTERRUPT_ENABLE 1
#define RX_LINE_STATUS_INTERRUPT_ENABLE 2
#define ABEOINTEN 8
#define ABTOINTEN 9

#define BUFFER_SIZE 256

char circular_buffer[BUFFER_SIZE];
int head = 0;
int tail = 0;


void uart_init()
{
	//Power
	LPC_SC->PCONP |= (0b1 << PCUART0);

	//Set 00 then 01 to clock = 100MHz
	LPC_SC->PCLKSEL0 &= ~(3 << PCLK_UART0);
	LPC_SC->PCLKSEL0 |= (1 << PCLK_UART0);

	//enable DLAB, word length to 8
	LPC_UART0->LCR =
			(0b11 << WORD_LENGTH) |
			(0b0 << STOP_BIT) |
			(0b0 << PARITY_ENALBLE) |
			(0b0 << PARITY_SELECT) |
			(0b0 << BREAK_CONTROL) |
			(0b1 << DLAB);

	//Baudrate config
	LPC_UART0->DLL = 50;
	LPC_UART0->DLM = 0;
	LPC_UART0->FDR = (1 & 0xf) | ((12 & 0xf) << 4); //DivAddVal = 1, MulVal = 12, res should be 115384.6154

	//disable DLAB
	LPC_UART0->LCR =
			(0b11 << WORD_LENGTH) |
			(0b0 << STOP_BIT) |
			(0b0 << PARITY_ENALBLE) |
			(0b0 << PARITY_SELECT) |
			(0b0 << BREAK_CONTROL) |
			(0b0 << DLAB);

	//enable fifo
	LPC_UART0->FCR =
			(0b1 << FIFO_ENABLE) |
			(0b0 << RX_FIFO_RESET) |
			(0b0 << TX_FIFO_RESET) |
			(0b0 << DMA_MODE) |
			(0b00 << RX_TRIGGER_LEVEL);

	//Pin mode UART
	LPC_PINCON->PINSEL0 &= ~(3 << UART_TX_PINSEL);
	LPC_PINCON->PINSEL0 |= (0b1 << UART_TX_PINSEL);

	LPC_PINCON->PINSEL0 &= ~(3 << UART_RX_PINSEL);
	LPC_PINCON->PINSEL0 |= (0b1 << UART_RX_PINSEL);


	//interrupts
	LPC_UART0->IER =
			(0b1 << RBR_INTERRUPT_ENABLE) |
			(0b0 << THRE_INTERRUPT_ENABLE) |
			(0b0 << RX_LINE_STATUS_INTERRUPT_ENABLE) |
			(0b0 << ABEOINTEN) |
			(0b0 << ABTOINTEN);

	NVIC_EnableIRQ(UART0_IRQn); // Enable UART0 global IRQ
 }

void uart_send(char *value, uint8_t size)
{

	for(int i = 0; i < size; i++)
	{
		while(((LPC_UART0->LSR & (0b1 << 5)) >> 5) != 1) //make sure thr is empty
		{}

		LPC_UART0->THR = value[i] & 0xff;
	}

}

char uart_get_char()
{
	while(tail == head); //wait if no char has been received

	NVIC_DisableIRQ(UART0_IRQn);
	char c = circular_buffer[tail];
	tail = (tail + 1) % BUFFER_SIZE;
	NVIC_EnableIRQ(UART0_IRQn);
	return c;
}

void put_char(char c)
{
	circular_buffer[head] = c;
	head = (head + 1) % BUFFER_SIZE;
}

void clear()
{
	tail = 0;
	head = 0;
}


void UART0_IRQHandler(void)
{
	put_char(LPC_UART0->RBR);
}
