/*
 * usb.c
 *
 *  Created on: May 31, 2024
 *      Author: david
 */

#include "LPC17xx.h"

#define PCUSB 		31
#define USB_DP 		26
#define USB_DN 		28

#define DEV_CLK_EN 	1
#define AHB_CLK_EN 	4
#define VBUS		28

#define EP_RLZED 	8
#define CDFULL 		5
//#define PLL1_ENABLE 1
//#define PLL1_CONNECT 2
//#define PLOCK1 10
//#define PLLE1_STAT 8
//#define PLLC1_STAT 9
//#define MSEL1 0
//#define PSEL1 5
//#define PLL1_FEED LPC_SC->PLL1FEED = 0xAA; LPC_SC->PLL1FEED = 0x55;

void USB_IRQHandler(){

}

void _set_address(short addr){
	LPC_USB->USBDevIntClr = 0x10; 							// Clear CCEMPTY.
	LPC_USB->USBCmdCode = 0x00D00500; 						// CMD_CODE=0xD0, CMD_PHASE=0x05(Command)
	while (!(LPC_USB->USBDevIntSt & 0x10)); 				// Wait for CCEMPTY.
	LPC_USB->USBDevIntClr = 0x10; 							// Clear CCEMPTY.
	LPC_USB->USBCmdCode = 0x00800100 | (addr << 16); 		// CMD_WDATA=0x8A(DEV_EN=1, DEV_ADDR=0xA),
	// CMD_PHASE=0x01(Write)
	while (!(LPC_USB->USBDevIntSt & 0x10)); 				// Wait for CCEMPTY.
	LPC_USB->USBDevIntClr = 0x10; 							// Clear CCEMPTY.
}

void usb_init(){

	LPC_SC->PCONP |= 1 << PCUSB;

	LPC_USB->USBClkCtrl = (1 << DEV_CLK_EN) | (1 << AHB_CLK_EN);

	LPC_PINCON->PINSEL1 |= (0b01 << USB_DP) | (0b01 << USB_DN);

	LPC_PINCON->PINMODE3 = 10 << VBUS;

	LPC_USB->USBEpInd = 0;
	LPC_USB->USBMaxPSize = 1024;
	while((LPC_USB->USBDevIntSt >> EP_RLZED) & 1);

	LPC_USB->USBEpInd = 1;
	LPC_USB->USBMaxPSize = 1024;
	while((LPC_USB->USBDevIntSt >> EP_RLZED) & 1);

	// Clear all endpoint interrupts using USBEpIntClr.
	for (int i = 0; i < 31; ++i) {
		while(!((LPC_USB->USBDevIntSt >> CDFULL) & 1))
		LPC_USB->USBEpIntClr = 1 << i;
	}

	// Clear any device interrupts using USBDevIntClr
	LPC_USB->USBDevIntClr = 0xffffffff;


	// Enable Slave mode for the desired endpoints by setting the corresponding bits in	USBEpIntEn.
	LPC_USB->USBEpIntEn = 0b1111;

//	– Set the priority of each enabled interrupt using USBEpIntPri.
//	– Configure the desired interrupt mode using the SIE Set Mode command.

//	– Enable device interrupts using USBDevIntEn (normally DEV_STAT, EP_SLOW,
//	and possibly EP_FAST).
	// enable EP_FAST EP_SLOW
	LPC_USB->USBDevIntEn = 0b110;

	NVIC_EnableIRQ(USB_IRQn);

	_set_address(0);
}
