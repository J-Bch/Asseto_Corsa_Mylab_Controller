/*
 * i2c.h
 *
 *  Created on: Mar 8, 2024
 *      Author: joachim
 */

#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>

void i2c_init();
int i2c_read_bytes(uint8_t addr, uint8_t reg, uint8_t len, uint8_t* result_data);
int i2c_write_bytes(uint8_t addr, uint8_t reg, uint8_t len, uint8_t* source_data);
int i2c_wait_for_end_transaction();
void i2c_enable_debug_mode();


#endif /* I2C_H_ */
