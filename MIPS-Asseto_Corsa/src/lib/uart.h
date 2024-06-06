/*
 * uart.h
 *
 *  Created on: Mar 1, 2024
 *      Author: joachim
 */

#ifndef UART_H_
#define UART_H_

#include <stdint.h>

void uart_init();
void uart_send(char *value, uint8_t size);
void uart_recieve();
void put_char(char c);
char uart_get_char();
int count();
void uart_clear();



#endif /* UART_H_ */
