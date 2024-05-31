#include "Ecran_LCD_ILI9341_Initial.h"
#include "lcd.h"

void ILI9341_Initial(void)
{
	lcd_write_cmd(0x01); //software reset
	delay(5);

	lcd_write_cmd(0x11);
	delay(120);

	lcd_write_cmd(0xCF);
	lcd_write_data(0x00);
	lcd_write_data(0x83);
	lcd_write_data(0X30);

 	lcd_write_cmd(0xED);
	lcd_write_data(0x64);
	lcd_write_data(0x03);
	lcd_write_data(0X12);
	lcd_write_data(0X81);

 	lcd_write_cmd(0xE8);
	lcd_write_data(0x85);
	lcd_write_data(0x01);
	lcd_write_data(0x79);

 	lcd_write_cmd(0xCB);
	lcd_write_data(0x39);
	lcd_write_data(0x2C);
	lcd_write_data(0x00);
	lcd_write_data(0x34);
	lcd_write_data(0x02);

 	lcd_write_cmd(0xF7);
	lcd_write_data(0x20);

 	lcd_write_cmd(0xEA);
	lcd_write_data(0x00);
	lcd_write_data(0x00);


 	lcd_write_cmd(0xC1);    //Power control
	lcd_write_data(0x11);   //SAP[2:0];BT[3:0]

 	lcd_write_cmd(0xC5);    //VCM control 1
	lcd_write_data(0x34);
	lcd_write_data(0x3D);

 	lcd_write_cmd(0xC7);    //VCM control 2
	lcd_write_data(0xC0);

 	lcd_write_cmd(0x36);    // Memory Access Control
	lcd_write_data(0x08);

 	lcd_write_cmd(0x3A);   	 // Pixel format
	lcd_write_data(0x55);  //16bit

 	lcd_write_cmd(0xB1);   	   // Frame rate
	lcd_write_data(0x00);
	lcd_write_data(0x1D);  //65Hz

 	lcd_write_cmd(0xB6);    // Display Function Control
	lcd_write_data(0x0A);
	lcd_write_data(0xA2);
	lcd_write_data(0x27);
	lcd_write_data(0x00);

	lcd_write_cmd(0xb7); //Entry mode
	lcd_write_data(0x07);


 	lcd_write_cmd(0xF2);    // 3Gamma Function Disable
	lcd_write_data(0x08);

 	lcd_write_cmd(0x26);    //Gamma curve selected
	lcd_write_data(0x01);


	lcd_write_cmd(0xE0); //positive gamma correction
	lcd_write_data(0x1f);
	lcd_write_data(0x1a);
	lcd_write_data(0x18);
	lcd_write_data(0x0a);
	lcd_write_data(0x0f);
	lcd_write_data(0x06);
	lcd_write_data(0x45);
	lcd_write_data(0x87);
	lcd_write_data(0x32);
	lcd_write_data(0x0a);
	lcd_write_data(0x07);
	lcd_write_data(0x02);
	lcd_write_data(0x07);
	lcd_write_data(0x05);
	lcd_write_data(0x00);

	lcd_write_cmd(0xE1); //negamma correction
	lcd_write_data(0x00);
	lcd_write_data(0x25);
	lcd_write_data(0x27);
	lcd_write_data(0x05);
	lcd_write_data(0x10);
	lcd_write_data(0x09);
	lcd_write_data(0x3a);
	lcd_write_data(0x78);
	lcd_write_data(0x4d);
	lcd_write_data(0x05);
	lcd_write_data(0x18);
	lcd_write_data(0x0d);
	lcd_write_data(0x38);
	lcd_write_data(0x3a);
	lcd_write_data(0x1f);

 	lcd_write_cmd(0x11);    //Exit Sleep
 	delay(120);
 	lcd_write_cmd(0x29);    //Display on
 	delay(50);
}
