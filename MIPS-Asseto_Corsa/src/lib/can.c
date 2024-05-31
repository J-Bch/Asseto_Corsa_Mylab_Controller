/*
 * CAN.c
 *
 *  Created on: Apr 12, 2024
 *      Author: joachim
 */


#include "can.h"
#include "LPC17xx.h"
#include "callback.h"
#include <stdio.h>


#define PCCAN1 13 //power to can1

//PCLKSEL
#define PCLK_CAN1 26
#define PCLK_CAN2 28
#define PCLK_ACF 30

//PINSEL
#define RD1 10 //RD1, RD2 Input Serial Inputs. From CAN transceivers.
#define TD1 12 //TD1, TD2 Output Serial Outputs. To CAN transceivers.

//CAN1MOD register
#define RM 0
#define LOM 1
#define STM 2
#define TPM 3
#define SM 4
#define RPM 5
#define TM 7


//CAN1CMR register
#define TR 0
#define AT 1
#define RRB 2
#define CDO 3
#define SRR 4
#define STB1 5
#define STB2 6
#define STB3 7

//CAN1GSR
#define RBS 0
#define DOS 1
#define TBS 2
#define TCS 3
#define RS 4
#define TS 5
#define ES 6
#define BS 7
#define RXERR 16
#define TXERR 24

//CAN1BTR
#define BRP 0
#define SJW 14
#define TESG1 16
#define TESG2 20
#define SAM 23


//CAN1RDA
#define DATAR1 0
#define DATAR2 8
#define DATAR3 16
#define DATAR4 24
#define DARAR5 0
#define DATAR6 8
#define DATAR7 16
#define DATAR8 24

//CAN1TDA
#define DATAT1 0
#define DATAT2 8
#define DATAT3 16
#define DATAT4 24
#define DARAT5 0
#define DATAT6 8
#define DATAT7 16
#define DATAT8 24

//CAN1IER
#define RIE 0


//CAB1TFI
#define PRIO 0
#define DLC 16
#define RTR 30
#define FF 31

//CAN1SR
#define TBS1 2
#define TBS2 10
#define TBS3 18

//CAN1TID
#define ID 0

//AFMR
#define AccBP 1

#define CIRCULAR_BUFFER_SIZE 10


typedef struct _can_msg {
	uint32_t id;
	uint8_t rtr;
	uint8_t type; /* Ext/Std frame */ //11 or 29 bit
	uint8_t dlc;
	uint8_t error; /* Optional error code */
	uint8_t data[8];
}can_msg;

can_msg last_can_msg;
can_msg circular_can_buffer[CIRCULAR_BUFFER_SIZE];
uint8_t head_can = 0;
uint8_t tail_can = 0;
uint8_t message_to_read_number = 0;


void can_init()
{
	//Power
	LPC_SC->PCONP |= (0b1 << PCCAN1);

	//select PCLK_CAN1,PCLK_CAN2, and, for the acceptance filter, PCLK_ACF. Note that these must all be the same value.
	LPC_SC->PCLKSEL0 &= ~(3 << PCLK_CAN1);
	LPC_SC->PCLKSEL0 &= ~(3 << PCLK_CAN2); //PCLK_CAN1 and PCLK_CAN2 must have the same PCLK divide value when the CAN function is used.
	LPC_SC->PCLKSEL0 &= ~(3 << PCLK_ACF);


	LPC_PINCON->PINSEL1 |= (0b11 << RD1);
	LPC_PINCON->PINSEL1 |= (0b11 << TD1);

	LPC_CAN1->MOD |= (0b1 << RM); //enable reset mod to be able to edit timings


	//Timing calculations, see slide 25
	LPC_CAN1->BTR = (4 << BRP) |
					(0b00 <<  SJW) |
					(13 << TESG1) | //13 + 1 = 14 for Phase Seg1 14tq
					(4 << TESG2) | // Phase Seg2 5tq
					(0b0 << SAM); //default value for sampling

	LPC_CAN1->MOD &= ~(0b1 << RM);


	LPC_CANAF->AFMR = (0b1 << AccBP); //set rx to all ID

	LPC_CAN1->IER = (0b1 << RIE); //enable recieve interrupt

	NVIC_EnableIRQ(CAN_IRQn);
}

void can_send(uint32_t id, uint8_t ext_id, uint8_t data_len, uint8_t data[data_len])
{
	while(1)
	{
		if(((LPC_CAN1->SR >> TBS1) & 0b1) != 0)
		{
			LPC_CAN1->TDA1 = (data[0] << DATAT1) |
							(data[1] << DATAT2) |
							(data[2] << DATAT3) |
							(data[3] << DATAT4);

			LPC_CAN1->TDB1 = (data[4] << DATAT1) |
							(data[5] << DATAT2) |
							(data[6] << DATAT3) |
							(data[7] << DATAT4);

			LPC_CAN1->TFI1 = (0 << PRIO) |
							(data_len << DLC) |
							(0 << RTR) | //send, not request
							(ext_id << FF); //if ID is 29b or 11b

			LPC_CAN1->TID1 = (id << ID);


			//activate send data for buffer 1
			LPC_CAN1->CMR = (0b1 << TR) |
							(0b1 << STB1);

			break;
		}
		else if(((LPC_CAN1->SR >> TBS2) & 0b1) != 0)
		{
			LPC_CAN1->TDA2 = (data[0] << DATAT1) |
							(data[1] << DATAT2) |
							(data[2] << DATAT3) |
							(data[3] << DATAT4);

			LPC_CAN1->TDB2 = (data[4] << DATAT1) |
							(data[5] << DATAT2) |
							(data[6] << DATAT3) |
							(data[7] << DATAT4);

			LPC_CAN1->TFI2 = (0 << PRIO) |
							(data_len << DLC) |
							(0 << RTR) | //send, not request
							(ext_id << FF); //if ID is 29b or 11b

			LPC_CAN1->TID2 = (id << ID);


			//activate send data for buffer 1
			LPC_CAN1->CMR = (0b1 << TR) |
							(0b1 << STB2);

			break;
		}
		else if(((LPC_CAN1->SR >> TBS3) & 0b1) != 0)
		{
			LPC_CAN1->TDA3 = (data[0] << DATAT1) |
							(data[1] << DATAT2) |
							(data[2] << DATAT3) |
							(data[3] << DATAT4);

			LPC_CAN1->TDB3 = (data[4] << DATAT1) |
							(data[5] << DATAT2) |
							(data[6] << DATAT3) |
							(data[7] << DATAT4);

			LPC_CAN1->TFI3 = (0 << PRIO) |
							(data_len << DLC) |
							(0 << RTR) | //send, not request
							(ext_id << FF); //if ID is 29b or 11b

			LPC_CAN1->TID3 = (id << ID);


			//activate send data for buffer 1
			LPC_CAN1->CMR = (0b1 << TR) |
							(0b1 << STB3);

			break;
		}
	}

}

void CAN_IRQHandler(void)
{
	if(((LPC_CAN1->RFS >> FF) & 0b1) == 1) //if ff bit of CAN1RFS, id is 29b
	{
		circular_can_buffer[head_can].id = LPC_CAN1->RID & 0x1FFFFFFF;
	}
	else
	{
		circular_can_buffer[head_can].id = LPC_CAN1->RID & 0x7FF;
	}

	circular_can_buffer[head_can].dlc = ((LPC_CAN1->RFS >> DLC) & 0b1111);

	for(int i = 0; i < circular_can_buffer[head_can].dlc; i++)
	{
		if(i < 4)
		{
			circular_can_buffer[head_can].data[i] = (LPC_CAN1->RDA >> 8*i) & 0xFF;
		}
		else
		{
			circular_can_buffer[head_can].data[i] = (LPC_CAN1->RDB >> 8*(i-4)) & 0xFF;
		}
	}

	head_can = (head_can + 1) % CIRCULAR_BUFFER_SIZE;
	message_to_read_number++;

	LPC_CAN1->CMR = (0b1 << RRB); //clear interrupt

	callback_setflag(CAN_IRQn);
}



uint32_t can_get_last_id()
{
	return circular_can_buffer[tail_can].id;
}

uint8_t* can_get_last_data()
{
	return last_can_msg.data;
}

void can_get_message(uint32_t* recieved_id, uint8_t** recieved_data)
{
	*recieved_id = circular_can_buffer[tail_can].id;
	*recieved_data = circular_can_buffer[tail_can].data;
	tail_can = (tail_can + 1) % CIRCULAR_BUFFER_SIZE;
	message_to_read_number--;
}

uint8_t can_get_message_to_read_number()
{
	return message_to_read_number;
}






