/*
 * ssp.h
 *
 *  Created on: Feb 23, 2024
 *      Author: joachim
 */

#ifndef SSP_H_
#define SSP_H_

#include <stdint.h>
#include "ssp.h"
#include "LPC17xx.h"

void ssp_init(uint8_t data_size_select, uint8_t scr);
void ssp_send_buf(uint8_t *buf, uint32_t length);
void ssp_receive_buf(uint8_t *buf, uint32_t length);

#endif /* SSP_H_ */
