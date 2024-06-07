/*
 * usb.c
 *
 *  Created on: May 31, 2024
 *      Author: david
 */

#include "LPC17xx.h"

#define PCUSB 			31

#define USB_DP 			26
#define USB_DN 			28
#define VBUS			28
#define USB_UP_LED		4
#define USB_CONNECT		18

#define DEV_CLK 		1
#define AHB_CLK 		4

#define EP_RLZED 		8
#define CDFULL 			5



#define INT_FRAME 		(1 << 0)
#define INT_EP_FAST 	(1 << 1)
#define INT_EP_SLOW 	(1 << 2)
#define INT_DEV_STAT 	(1 << 3)
#define INT_CCEMPTY 	(1 << 4)
#define INT_CDFULL	 	(1 << 5)
#define INT_RXENDPKT 	(1 << 6)
#define INT_TXENDPKT 	(1 << 7)
#define INT_EP_RLZED 	(1 << 8)
#define INT_ERR_INT 	(1 << 9)


void USB_IRQHandler(){


	if(LPC_USB->USBDevIntSt & INT_EP_FAST) {
		printf("INT_EP_FAST\n");
		LPC_USB->USBDevIntClr = INT_EP_FAST;
	}
	if(LPC_USB->USBDevIntSt & INT_EP_SLOW) {
//		printf("INT_EP_SLOW\n");
		LPC_USB->USBDevIntClr = INT_EP_SLOW;
	}

	if(LPC_USB->USBDevIntSt & INT_DEV_STAT) {
		printf("INT_DEV_STAT\n");
		LPC_USB->USBDevIntClr = INT_DEV_STAT;
	}

	if(LPC_USB->USBDevIntSt & INT_CDFULL) {
		printf("INT_CCFULL\n");
		LPC_USB->USBDevIntClr = INT_CDFULL;
	}

	//	if(LPC_USB->USBDevIntSt & INT_CCEMPTY) {
	//		LPC_USB->USBDevIntClr = INT_CCEMPTY;
	//	}
	//	if(LPC_USB->USBDevIntSt & INT_CCFULL) {
	//		LPC_USB->USBDevIntClr = INT_CCFULL;
	//	}
	//	if(LPC_USB->USBDevIntSt & INT_FRAME) {
	//		LPC_USB->USBDevIntClr = INT_FRAME;
	//	}
	//	if(LPC_USB->USBDevIntSt & INT_RXENDPKT) {}
	//	if(LPC_USB->USBDevIntSt & INT_TXENDPKT) {}
	//	if(LPC_USB->USBDevIntSt & INT_EP_RLZED) {}
	//	if(LPC_USB->USBDevIntSt & INT_ERR_INT) {}

}


#define SIE_COMMAND 0x0500
#define SIE_READ 	0x0200
#define SIE_WRITE	0x0100

#define SET_ADDRESS 				0xD0 // Write 1 byte
#define CONFIGURE_DEVICE 			0xD8 // Write 1 byte
#define SET_MODE					0xF3 // Write 1 byte
#define READ_CURRENT_FRAME_NUMBER 	0xF5 // Read 1 or 2 bytes
#define READ_TEST_REGISTER  		0xFD // Read 2 bytes
#define SET_DEVICE_STATUS  			0xFE // Write 1 byte
#define GET_DEVICE_STATUS 			0xFE // Read 1 byte
#define GET_ERROR_CODE 				0xFF // Read 1 byte
#define READ_ERROR_STATUS 			0xFB // Read 1 byte


void _sie_command(char command){

	// clear interrupts

	LPC_USB->USBDevIntClr = INT_CCEMPTY | INT_CDFULL;

	// command

	LPC_USB->USBCmdCode = 0x500 | (command << 16);

	// clear interrupts
	while (!(LPC_USB->USBDevIntSt & INT_CCEMPTY));
	LPC_USB->USBDevIntClr = INT_CCEMPTY;

}

void _sie_write(char command, char byte){

	_sie_command(command);

	// write command, write or read instruction

	LPC_USB->USBCmdCode = 0x100 | (byte << 16);

	// wait until command was written

	while (!(LPC_USB->USBDevIntSt & INT_CCEMPTY));
	LPC_USB->USBDevIntClr = INT_CCEMPTY;

}

void _sie_read(char command, char* bytes, char len){

	_sie_command(command);

	for (int i = 0; i < len; ++i) {

		// write command, write or read instruction

		LPC_USB->USBCmdCode = 0x200 | (command << 16);

		// wait until data is ready to be read
		while (!(LPC_USB->USBDevIntSt & INT_CDFULL));
		LPC_USB->USBDevIntClr = INT_CDFULL; // cleared by interrupt
		bytes[i] = LPC_USB->USBCmdData;

	}
}



void usb_write(){
	LPC_USB->USBCtrl = 0b10;


	while(1){
		LPC_USB->USBTxPLen = 4; // bytes count

		for (int i = 0; i < 1; ++i) {
			LPC_USB->USBTxData = 0x01020304;
		}
	}
}


// VBUS 		I 	VBUS status input. When this function is not enabled via its corresponding PINSEL register, it is driven HIGH internally.
// USB_CONNECT 	O 	SoftConnect control signal.
// USB_UP_LED 	O 	GoodLink LED control signal.
// USB_D+ 		I/O Positive differential data.
// USB_D- 		I/O Negative differential data


void usb_init(){

	LPC_SC->PCONP |= 1 << PCUSB;

	LPC_USB->USBClkCtrl = (1 << DEV_CLK) | (1 << AHB_CLK);
	while(!(LPC_USB->USBClkSt & 0b10010));

	LPC_PINCON->PINSEL1 |= (0b01 << USB_DP) | (0b01 << USB_DN);

	// this code make sure we're monitoring correct pins and the force state will show activity from host
	//	LPC_GPIO0->FIODIR |= (1 << 29);
	//	LPC_GPIO0->FIOSET = (1 << 29);
	//	LPC_GPIO0->FIODIR |= (1 << 30);

	LPC_PINCON->PINSEL3 |= (0b10 << VBUS);
	LPC_PINCON->PINSEL4 |= (0b01 << USB_CONNECT);

//	LPC_PINCON->PINMODE3 = 0b10 << VBUS;


	NVIC_EnableIRQ(USB_IRQn);
	// test clocks, chapter 11.12.6, page 257
	int tmp = 0;
	_sie_read(READ_TEST_REGISTER, (char*)&tmp, 2);
	printf("READ_TEST_REGISTER: 0x%X (should be 0xA50F)\n", tmp);

	tmp = 0;
	_sie_read(GET_DEVICE_STATUS, (char*)&tmp, 1);
	printf("GET_DEVICE_STATUS: %x\n", tmp);


	// Set maxsize
	//	LPC_USB->USBEpInd = 0;
	//	LPC_USB->USBMaxPSize = 256;
	//	while(!((LPC_USB->USBDevIntSt >> EP_RLZED) & 1));
	//
	//	LPC_USB->USBEpInd = 1;
	//	LPC_USB->USBMaxPSize = 256;
	//	while(!((LPC_USB->USBDevIntSt >> EP_RLZED) & 1));


	// Clear all endpoint interrupts using USBEpIntClr.
//	LPC_USB->USBDevIntSet = INT_CDFULL;
//	for (int i = 0; i < 31; ++i) {
//		LPC_USB->USBEpIntClr = 1 << i;
////		while(!(LPC_USB->USBDevIntSt & INT_CDFULL));
//
//		LPC_USB->USBDevIntClr = INT_CDFULL;
//	}

	// Clear any device interrupts using USBDevIntClr
	LPC_USB->USBDevIntClr = 0xffffffff;


	// Enable Slave mode for the desired endpoints by setting the corresponding bits in	USBEpIntEn.
	LPC_USB->USBEpIntEn = 0xffffffff;


	LPC_USB->USBDevIntEn =  INT_CDFULL | INT_DEV_STAT | INT_EP_FAST | INT_EP_SLOW;

	// test interrupts
	LPC_USB->USBDevIntSet = 6;


	_sie_write(SET_ADDRESS, 0);
	_sie_write(SET_DEVICE_STATUS, 1);

	char tmp2 = 0;
	_sie_read(GET_DEVICE_STATUS, &tmp2, 1);
	printf("GET_DEVICE_STATUS: %x\n", tmp2);



	usb_write();
}

