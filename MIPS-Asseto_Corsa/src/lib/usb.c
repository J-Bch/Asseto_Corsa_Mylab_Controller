/*
 * usb.c
 *
 *  Created on: May 31, 2024
 *      Author: david
 */

#include "LPC17xx.h"
#include "circular_buffer.h"
#include <stdio.h>

/*
 * Power
 */


#define PCUSB 			31

/*
 * Pins
 *
 * VBUS 		I 	VBUS status input. When this function is not enabled via its corresponding PINSEL register, it is driven HIGH internally.
 * USB_CONNECT 	O 	SoftConnect control signal.
 * USB_UP_LED 	O 	GoodLink LED control signal.
 * USB_D+ 		I/O Positive differential data.
 * USB_D- 		I/O Negative differential data
 *
 */

#define USB_DP 			26
#define USB_DN 			28
#define VBUS			28
#define USB_UP_LED		4
#define USB_CONNECT		18


/*
 * Clocks
 */

#define DEV_CLK 		1
#define AHB_CLK 		4


/*
 * Transfers
 */

#define RX_READY		(1 << 11)
#define RX_VALID		(1 << 10)
#define RX_LEN_MASK		((1 << 10) - 1)

/*
 * Interrupts
 */

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


/*
 * SIE command
 */

#define SIE_COMMAND 				0x0500
#define SIE_READ 					0x0200
#define SIE_WRITE					0x0100

#define SET_ADDRESS 				0xD0 // Write 1 byte
#define CONFIGURE_DEVICE 			0xD8 // Write 1 byte
#define SET_MODE					0xF3 // Write 1 byte
#define READ_CURRENT_FRAME_NUMBER 	0xF5 // Read 1 or 2 bytes
#define READ_TEST_REGISTER  		0xFD // Read 2 bytes
#define SET_DEVICE_STATUS  			0xFE // Write 1 byte
#define GET_DEVICE_STATUS 			0xFE // Read 1 byte
#define GET_ERROR_CODE 				0xFF // Read 1 byte
#define READ_ERROR_STATUS 			0xFB // Read 1 byte

#define SELECT_ENDPOINT				0x00 // Read 1 byte (optional)
#define CLEAR_BUFFER				0xF2 // Read 1 byte (optional)
#define SET_ENDPOINT_STATUS			0x40 // Read 1 byte (optional)

#define VALIDATE_BUFFER 			0xFA

DEFINE_CIRCULAR_BUFFER(uint32_t, usb_buffer, 10);


/*
 * HID defines
 */

#define HID_REQ_CLEAR_FEATURE		1

#define HID_REQ_GET_CONFIGURATION	8
#define HID_REQ_GET_DESCRIPTOR		6
#define HID_REQ_GET_INTERFACE		10
#define HID_REQ_GET_STATUS			0

#define HID_REQ_SET_ADDRESS			5
#define HID_REQ_SET_CONFIGURATION	9
#define HID_REQ_SET_DESCRIPTOR		7
#define HID_REQ_SET_FEATURE			3
#define HID_REQ_SET_INTERFACE		11

#define HID_REQ_SYNCH_FRAME			12

/*
 * HID structs
 */

typedef struct  __attribute__ ((__packed__)) _setup_paquet {

//  uint8_t bmRequestType_rec: 5;
//	uint8_t bmRequestType_typ: 2;
//	uint8_t bmRequestType_dir: 1;
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
} setup_paquet_t;


typedef struct  __attribute__ ((__packed__)) _descriptor {
	uint8_t  bLength;				// Size of this descriptor in bytes
	uint8_t  bDescriptorType;		// DEVICE descriptor type (= 1)
	uint16_t bcdUSB;				// USB Spec release number
	uint8_t  bDeviceClass;			// Class code assigned by USB-IF
									// 00h means each interface defines its own class
									// FFh means vendor-defined class
									// Any other value must be a class code
	uint8_t  bDeviceSubClass;		// SubClass Code assigned by USB-IF
	uint8_t  bDeviceProtocol;		// Protocol Code assigned by USB-IF
	uint8_t  bMaxPacketSize0;		// Max packet size for endpoint 0.
									// Must be 8, 16, 32 or 64
	uint16_t  idVendor;				// Vendor ID - must be obtained from USB-IF
	uint16_t idProduct;				// Product ID - assigned by the manufacturer
	uint16_t bcdDevice;				// Device release number in binary coded decimal
	uint8_t  iManufacturer;			// Index of string descriptor describing manufacturer - set to 0 if no string
	uint8_t  iProduct;				// Index of string descriptor describing product - set to 0 if no string
	uint8_t  iSerialNumber;			// Index of string descriptor describing device serial number - set to 0 if no string
	uint8_t  bNumConfigurations;	// Number of possible configurations
} descriptor_t;

/**
 * HID globals
 */

#define MAX_SIZE 64


descriptor_t device_descriptor = {
	.bLength = 18,
	.bDescriptorType = 1,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0x00,
	.bDeviceSubClass = 0x0,
	.bDeviceProtocol = 0x0,
	.bMaxPacketSize0 = MAX_SIZE,
	.idVendor = 0x0F62,
	.idProduct = 0x1001,
	.bcdDevice = 0x0001,
	.iManufacturer = 0,
	.iProduct = 0,
	.iSerialNumber = 0,
	.bNumConfigurations = 1
};


int status = 0;
int read_flag = 0;
int write_flag = 0;
int t_flag = 0;

setup_paquet_t setup_paquet;
usb_buffer_circular_buffer usb_buff;

/**
 *
 */

#define USB_READ_ENDPOINT_MODE(endpoint) (LPC_USB->USBCtrl = 0b01 | (endpoint << 2));
#define USB_WRITE_ENDPOINT_MODE(endpoint) (LPC_USB->USBCtrl = 0b10 | (endpoint << 2));


typedef enum _sie_read_len_t {
	NONE,
	ONE,
	TWO
} sie_read_len_t;

void _sie_command(char command){

	// clear interrupts

	LPC_USB->USBDevIntClr = INT_CCEMPTY | INT_CDFULL;

	// command

	LPC_USB->USBCmdCode = 0x500 | (command << 16);

	// clear interrupts
	while (!(LPC_USB->USBDevIntSt & INT_CCEMPTY));
	LPC_USB->USBDevIntClr = INT_CCEMPTY;

}

uint16_t _sie_read(char command, sie_read_len_t len){

	uint8_t bytes[2] = {0};
	_sie_command(command);

	for (int i = 0; i < len; ++i) {

		// write command, write or read instruction

		LPC_USB->USBCmdCode = 0x200 | (command << 16);

		// wait until data is ready to be read
		while (!(LPC_USB->USBDevIntSt & INT_CDFULL));
		LPC_USB->USBDevIntClr = INT_CDFULL; // cleared by interrupt
		bytes[i] = LPC_USB->USBCmdData;

	}

	return bytes[0] + (bytes[1] << 8);
}

uint16_t tmp;

void USB_IRQHandler(){


	if(LPC_USB->USBDevIntSt & INT_EP_SLOW) {
		int a = LPC_USB->USBEpIntSt ;
		if(a & 1){
			LPC_USB->USBEpIntClr = 1;
			read_flag = 1;
		}
		if(a & 2){
			LPC_USB->USBEpIntClr = 2;
			write_flag = 1;
		}

		LPC_USB->USBDevIntClr = INT_EP_SLOW;


	}

	if(LPC_USB->USBDevIntSt & INT_RXENDPKT) {
		LPC_USB->USBDevIntClr = INT_RXENDPKT;
	}

	if(LPC_USB->USBDevIntSt & INT_TXENDPKT) {
		LPC_USB->USBDevIntClr = INT_TXENDPKT;
		t_flag = 1;
		printf("t\n");//
	}

	if(LPC_USB->USBDevIntSt & INT_DEV_STAT) {
		printf("d\n");
		LPC_USB->USBDevIntClr = INT_DEV_STAT;
	}

	if(LPC_USB->USBDevIntSt & INT_ERR_INT) {
		printf("e\n");
		LPC_USB->USBDevIntClr = INT_ERR_INT;
	}

}



void _sie_write(char command, char byte){

	_sie_command(command);

	// write command, write or read instruction

	LPC_USB->USBCmdCode = 0x100 | (byte << 16);

	// wait until command was written

	while (!(LPC_USB->USBDevIntSt & INT_CCEMPTY));
	LPC_USB->USBDevIntClr = INT_CCEMPTY;

}



void clear_endpoint(int endpoint) {
	_sie_read(SELECT_ENDPOINT + endpoint, 0);
	_sie_read(CLEAR_BUFFER, 0);
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

/*
 *  set address and if *addr* isn't 0, enable dev_en (bit 7)
 */
void usb_set_address(uint8_t addr){

	_sie_write(SET_ADDRESS, addr | ((addr ? 1 : 0) << 7));

}



void usb_reset(){
	_sie_write(SET_ADDRESS, 0);
}

void usb_init(){

	usb_buffer_circular_buffer_init(&usb_buff);


	LPC_PINCON->PINSEL1 |= (0b01 << USB_DP) | (0b01 << USB_DN);
	LPC_PINCON->PINSEL3 |= (0b10 << VBUS) | ((0b01) << USB_UP_LED);
	LPC_PINCON->PINSEL4 |= (0b01 << USB_CONNECT);

	LPC_SC->PCONP |= 1 << PCUSB;

	LPC_USB->USBClkCtrl = (1 << DEV_CLK) | (1 << AHB_CLK);
	while(!(LPC_USB->USBClkSt & 0b10010));


	// test clocks, chapter 11.12.6, page 257
	uint16_t tmp = _sie_read(READ_TEST_REGISTER, 2);
	printf("READ_TEST_REGISTER: 0x%X (should be 0xA50F)\n", tmp);

	tmp = _sie_read(GET_DEVICE_STATUS, 1);
	printf("GET_DEVICE_STATUS: %x\n", tmp);
//
//	LPC_USB->USBEpInd = 1 << 0x80;
//	LPC_USB->USBMaxPSize = MAX_SIZE;
//	while(!(LPC_USB->USBDevIntSt & INT_EP_RLZED));

	// Clear any device interrupts using USBDevIntClr
	LPC_USB->USBDevIntClr = 0xffffffff;

	// Set maxsize
	LPC_USB->USBEpInd = 0;
	LPC_USB->USBMaxPSize = MAX_SIZE;
	while(!(LPC_USB->USBReEp & 3));

	LPC_USB->USBDevIntClr = INT_EP_RLZED;


	// Enable Slave mode for the desired endpoints by setting the corresponding bits in	USBEpIntEn.
	LPC_USB->USBEpIntEn = 0x3;


	LPC_USB->USBDevIntEn =  INT_DEV_STAT | INT_EP_FAST | INT_EP_SLOW | INT_RXENDPKT | INT_TXENDPKT  | INT_ERR_INT;


//	clear_endpoint(0);

	usb_set_address(0 | (1 << 7));
	_sie_write(SET_DEVICE_STATUS, 1);
	_sie_write(SET_MODE, 0x7f); // interrupts on nack and ack
	NVIC_EnableIRQ(USB_IRQn);



	while(1){

//		tmp = _sie_read(GET_ERROR_CODE, 1);
//
//		if(tmp & (1 << 4)){
//			printf("get err: 0x%x\n", tmp & 0xf);
//
//			tmp = _sie_read(READ_ERROR_STATUS, 1);
//
//			printf("read err: 0x%x\n", tmp & 0xff);
//
//		}
//

		// EP0RX
		if(read_flag){
//		if(LPC_USB->USBEpIntSt & 1){

			tmp = _sie_read(0, 1);

			// READ SETUP
			LPC_USB->USBCtrl = 0b01 | (0 << 2);

			while(!(LPC_USB->USBRxPLen & RX_READY));

			int len = LPC_USB->USBRxPLen;
			((uint32_t*) &setup_paquet)[0] = LPC_USB->USBRxData;
		    ((uint32_t*) &setup_paquet)[1] = LPC_USB->USBRxData;

//			tmp = _sie_read(0, 1);
//			tmp = _sie_read(CLEAR_BUFFER, 1);


			_sie_command(0);

			if(setup_paquet.bRequest == HID_REQ_GET_DESCRIPTOR){
				LPC_USB->USBCtrl = 0b10 | (0 << 2);

				LPC_USB->USBTxPLen = 0x64;

				int i = 0;
				while (LPC_USB->USBCtrl & 0b10) {
					LPC_USB->USBTxData = ((uint32_t*)&device_descriptor)[i++];
				}

				_sie_command(VALIDATE_BUFFER);

				t_flag = 0;

			}



			if(setup_paquet.bRequest == HID_REQ_SET_ADDRESS)
				usb_set_address(setup_paquet.wValue);


			if(!(len & RX_VALID)){
				printf("Invalid\n");
			}

			read_flag = 0;
//			printf("%d bytes received\n", len & 0x3ff);
//			printf("Received:       0x%X%X\n", 	setup_paquet);
//			printf("bmRequestType:  0x%x\n", 	setup_paquet.bmRequestType);
//			printf("bRequest:       0x%x\n", 	setup_paquet.bRequest);
//			printf("wValue:         0x%x\n", 	setup_paquet.wValue);
//			printf("wIndex:         0x%x\n", 	setup_paquet.wIndex);
//			printf("wLength:        0x%x\n", 	setup_paquet.wLength);
		}

		// EP0TX

		if(write_flag){
			tmp = _sie_read(GET_ERROR_CODE, 1);
//			printf("er: %x\n", );
			write_flag = 0;
		}



	}


//	usb_write();
}

