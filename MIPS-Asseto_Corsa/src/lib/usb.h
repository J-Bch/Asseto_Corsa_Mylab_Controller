// /*
//  * usb.h
//  *
//  *  Created on: May 31, 2024
//  *      Author: david
//  */

// #ifndef LIB_USB_H_
// #define LIB_USB_H_


// void USB_IRQHandler();
// void usb_init();

// #endif /* LIB_USB_H_ */


///////////////////////////////////////////////////////////
// This was my attempt to implement the usb protocol
// It wasn't really sucessfull.. 
// I managed to send the device descriptor to the computer,
// that I could see with usbmon, but I then ended up stuck
// on the set address with an error -62 in dmesg and a message
// saying that the device didn't accept the address
// 
// We ended up using a serial transmission with a python
// script that managed a virtual device through the module
// uinput
//
// This two files (usb.c & usb.h) are kept as proof that I tried
//////////////////////////////////////////////////////////