/*
 * i2c.c
 *
 *  Created on: Mar 8, 2024
 *      Author: joachim
 */


#include "i2c.h"
#include "LPC17xx.h"
#include <stdio.h>
#include <stdbool.h>



#define PCI2C0 7
#define PCLK_I2C0_BIT 14
#define SDA0_BIT 22
#define SCL0_BIT 24


//I2CONSET register
#define AA 2
#define SI 3
#define STO 4
#define STA 5
#define I2EN 6 //i2c enable

#define WRITE_BIT 0
#define READ_BIT 1

//Master transmitter mode
#define STATUS_ERROR -1
#define STATUS_DONE 0
#define STATUS_START 0x8
#define STATUS_REPEATED_START 0x10
#define STATUS_SLA_W_ACK 0x18
#define STATUS_SLA_W_NACK 0x20
#define STATUS_DATA_TR_ACK 0x28
#define STATUS_DATA_TR_NACK 0x30
#define STATUS_ARBITRATION_LOST 0x38


//Master reciever mode
#define STATUS_SLA_R_ACK 0x40
#define STATUS_SLA_R_NACK 0x48
#define STATUS_DATA_RC_ACK 0x50
#define STATUS_DATA_RC_NACK 0x58

typedef struct _i2c_transaction
{
	uint8_t slave_addr;
	uint8_t write_len;
	uint8_t read_len;
	uint8_t data_index;
	uint8_t status;
	uint8_t data[32];
}i2c_transaction;

void i2c_start_transaction(i2c_transaction t);
static void debug_print(char* str);

i2c_transaction next_transaction;
static bool debug_mode = false;

void i2c_init()
{
	//clear all because sta might be still on
	LPC_I2C0->I2CONCLR =
			(0b1 << AA) |
			(0b1 << SI) |
			(0b1 << STO) |
			(0b1 << STA) |
			(0b1 << I2EN);

	//Power
	LPC_SC->PCONP |= (0b1 << PCI2C0);

	//Peripheral clock to 100MHz
	LPC_SC->PCLKSEL0 &= ~(3 << PCLK_I2C0_BIT);
	LPC_SC->PCLKSEL0 |= (1 << PCLK_I2C0_BIT);

	//Pin mode to I2C
	LPC_PINCON->PINSEL1 &= ~(3 << SDA0_BIT);
	LPC_PINCON->PINSEL1 |= (0b1 << SDA0_BIT);

	LPC_PINCON->PINSEL1 &= ~(3 << SCL0_BIT);
	LPC_PINCON->PINSEL1 |= (0b1 << SCL0_BIT);

	LPC_I2C0->I2CONSET =
			(0b0 << AA) |
			(0b0 << SI) |
			(0b0 << STO) |
			(0b0 << STA) |
			(0b1 << I2EN); 	//Enable i2c0



	//Enable I2C interruptions
	NVIC_EnableIRQ(I2C0_IRQn);
}

void i2c_start_transaction(i2c_transaction t)
{
	next_transaction = t;
	next_transaction.status = STATUS_START;

	LPC_I2C0->I2CONSET =
			(0b0 << AA) |
			(0b0 << SI) |
			(0b0 << STO) |
			(0b1 << STA) |
			(0b1 << I2EN); 	//Enable i2c0;
}

int i2c_wait_for_end_transaction()
{
	while(next_transaction.status != STATUS_DONE && next_transaction.status != STATUS_ERROR)
	{}

	return next_transaction.status;
}

int i2c_read_bytes(uint8_t addr, uint8_t reg, uint8_t len, uint8_t* result_data)
{
	i2c_transaction t;
	t.slave_addr = addr;
	t.data[0] = reg;
	t.write_len = 1;
	t.read_len = len;

	i2c_start_transaction(t);

	int status = i2c_wait_for_end_transaction();

	for(int i = 0; i < len; i++)
	{
		result_data[i] = next_transaction.data[i];
	}

	return status;
}

int i2c_write_bytes(uint8_t addr, uint8_t reg, uint8_t len, uint8_t* source_data)
{
	i2c_transaction t;
	t.slave_addr = addr;
	t.data[0] = reg;
	t.write_len = len+1;
	t.read_len = 0;

	for(int i = 0; i < len; i++)
	{
		t.data[i+1] = source_data[i];
	}

	i2c_start_transaction(t);

	int status = i2c_wait_for_end_transaction();

	return status;
}

void debug_print(char* str)
{
	if(debug_mode)
	{
		printf("%s", str);
	}
}

void i2c_enable_debug_mode()
{
	debug_mode = true;
}

void I2C0_IRQHandler()
{
	int transaction_status = LPC_I2C0->I2STAT;
	char status[20];
	snprintf(status, 20, "Status : %x\n", transaction_status);
	debug_print(status);
	int read_write_bit = READ_BIT;

	switch(transaction_status)
	{
		case STATUS_START:
		case STATUS_REPEATED_START:

			LPC_I2C0->I2CONCLR = (0b1 << STA); //clear start

			if(next_transaction.write_len > 0)
			{
				debug_print("Write mode\n");
				read_write_bit = WRITE_BIT;
			}
			else
			{
				debug_print("Read mode\n");
				read_write_bit = READ_BIT;
			}

			LPC_I2C0->I2DAT = (next_transaction.slave_addr << 1) + read_write_bit;
			break;

		case STATUS_SLA_W_ACK:
			next_transaction.data_index = 0;
		case STATUS_DATA_TR_ACK:
			if(next_transaction.write_len > 0)
			{
				read_write_bit = WRITE_BIT;
				LPC_I2C0->I2DAT = next_transaction.data[next_transaction.data_index];
				next_transaction.data_index += 1;
				next_transaction.write_len -= 1;
			}
			else if(next_transaction.read_len > 0) //done writing, must read now
			{
				LPC_I2C0->I2CONSET = (0b1 << STA); //launch restart
			}
			else
			{
				debug_print("Nothing to do anymore, stopping communication\n\n");
				LPC_I2C0->I2CONSET = (0b1 << STO); //stop communication
				next_transaction.status = STATUS_DONE;
			}

			break;

		case STATUS_SLA_R_ACK:
			next_transaction.data_index = 0;

			if(next_transaction.read_len == 1)
			{
				debug_print("Send NACK for read\n");
				LPC_I2C0->I2CONCLR = (0b1 << AA); //send NACK to next read
			}
			else
			{
				debug_print("Send ACK for read\n");
				LPC_I2C0->I2CONSET = (0b1 << AA); //send NACK to next read
			}

			break;

		case STATUS_DATA_RC_ACK:
			if(next_transaction.read_len > 0)
			{
				next_transaction.data[next_transaction.data_index] = LPC_I2C0->I2DAT;
				next_transaction.data_index += 1;
				next_transaction.read_len -= 1;

				if(next_transaction.read_len == 1)
				{
					debug_print("Send NACK for read\n");
					LPC_I2C0->I2CONCLR = (0b1 << AA); //send NACK to next read
				}
				else
				{
					debug_print("Send ACK for read\n");
					LPC_I2C0->I2CONSET = (0b1 << AA); //send NACK to next read
				}
			}
			break;

		case STATUS_DATA_RC_NACK:
			next_transaction.data[next_transaction.data_index] = LPC_I2C0->I2DAT;
			next_transaction.data_index += 1;
			next_transaction.read_len -= 1;
			debug_print("Nack recieved, stopping communication\n\n");
			LPC_I2C0->I2CONSET = (0b1 << STO); //stop communication
			next_transaction.status = STATUS_DONE;
			break;

		case STATUS_ARBITRATION_LOST:
		case STATUS_DATA_TR_NACK:
		case STATUS_SLA_W_NACK:
		default: //if the code in unknown -> error
			debug_print("Error : nack recieved, stopping communication\n\n");
			LPC_I2C0->I2CONSET = (0b1 << STO); //stop communication
			next_transaction.status = STATUS_ERROR;
			break;
	}

	//Clear interrupt, always do at last
	LPC_I2C0->I2CONCLR = (0b1 << SI);
}
