/*
 * accelerometer.c
 *
 *  Created on: Mar 8, 2024
 *      Author: joachim
 */

#include "accelerometer.h"
#include "LPC17xx.h"
#include "i2c.h"
#include <stdio.h>
#include <stdbool.h>

#define nAccel_INT1_BIT 13 //p2.13
#define Accel_INT2_BIT 12 //p2.12
#define SCLH 0
#define SCLL 0
#define WHO_AM_I 0x0f
#define CTRL1 0x20
#define CTRL2 0x21
#define SLAVE_ADDR 0x1d

//CTRL1 register
#define AODR 4
#define BDU 3
#define AZEN 2
#define AYEN 1
#define AXEN 0

//CTRL2 register
#define ABW 6
#define AFS 3
#define AST 1
#define SIM 0

#define OUT_X_L_A 0x28


static bool debug_mode = false;

//The I2C interface is compliant with fast mode (400 kHz) I 2 C standards as well as with
//normal mode.

void accelerometer_init()
{
	//Pin mode to GPIO
	LPC_PINCON->PINSEL4 &= ~(3 << nAccel_INT1_BIT);
	LPC_PINCON->PINSEL4 &= ~(3 << Accel_INT2_BIT);

	//125 to both for 400 kHz
	LPC_I2C0->I2SCLH = 125;
	LPC_I2C0->I2SCLL = 125;
}

void debug_print(char* str)
{
	if(debug_mode)
	{
		printf("%s", str);
	}
}


void ask_who_am_i()
{
	debug_print("Asking who am i\n");
	uint8_t result[1];
	int status = 1;

	do
	{
		status = i2c_read_bytes(SLAVE_ADDR, WHO_AM_I, 1, result);
		if(status != 0)
		{
			char error_code[20];
			snprintf(error_code, 20, "Error code recieved : %x\n", status);
			debug_print(error_code);
		}
	}
	while(status != 0);

//	printf("Who am I = %x\n", result[0]);

	char who_am_i[20];
	snprintf(who_am_i, 20, "Who am I = %x\n", result[0]);
	debug_print(who_am_i);
}


void accelerometer_config()
{
	uint8_t data[2];

	data[0] = (0b1 << AXEN) |
			(0b1 << AYEN) |
			(0b1 << AZEN) |
			(0b0 << BDU) |
			(0b1010 << AODR); //set to full speed

	data[1] = (0b0 << SIM) |
			(0b0 << AST) |
			(0b000 << AFS) |
			(0b00 << ABW);

	int status = 1;
	do
	{
		status = i2c_write_bytes(SLAVE_ADDR, CTRL1 | (0b1 << 7), 2, data);

		if(status != 0)
		{
			char error_code[20];
			snprintf(error_code, 20, "Error code recieved : %x\n", status);
			debug_print(error_code);
		}

	}
	while(status != 0);



	debug_print("Done accelerometer config \n");
}

void accelerometer_get_value(int16_t* results)
{
	uint8_t data[6];

	int status = 1;
	do
	{
		status = i2c_read_bytes(SLAVE_ADDR, OUT_X_L_A | (0b1 << 7), 6, data);
		if(status != 0)
		{
			char error_code[20];
			snprintf(error_code, 20, "Error code recieved : %x\n", status);
			debug_print(error_code);
		}
	}
	while(status != 0);

	results[0] = (data[0] | (data[1] << 8));
	results[1] = (data[2] | (data[3] << 8));
	results[2] = (data[4] | (data[5] << 8));

	//two's complément
	for (int i = 0; i < 3; ++i)
	{
		if (results[i] & 0x8000)
		{
			results[i] = -((~results[i]) + 1);
		}
	}
}

void accelerometer_get_real_world_value(float* results)
{
	int16_t pre_results[3];

	accelerometer_get_value(pre_results);

	for(int i = 0; i < 3; i++)
	{
		results[i] = ((float)pre_results[i] / 32767) * 2;
	}
}

void accelerometer_enable_debug_mode()
{
	debug_mode = true;
}



