/**
@file			  UVCleaningApp.c
@author			  JGS
@brief			  UV Cleaning Application related functions definitions
*/


//_____I N C L U D E ________________________________________________________
// Place here all includes needed by this file starting with its own header
// showing the reason of the include.
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "IOExtenderControl.h"
#include "I2C_Driver.h"
#include "OledDisplayDriver.h"

//_____D E F I N E __________________________________________________________
// local private defines, enums, macros

#define i2cAddrOLED 	0x3C

#define OLE_SCL  0x02 //*display ��ʾ������ַ*/
#define OLE_SCH  0x10  //*11 display ��ʾ������ַ*/

//_____ T Y P E D E F _______________________________________________________
// local private typedefs, structs, unions


//_____ P R O T O T Y P E S _________________________________________________
// local function declarations

void write_ix(uint8_t ins);
void write_dx(uint8_t dat);

//_____ L O C A L  D A T A __________________________________________________
// local private data (static)
uint8_t OLE_VIEW[8][128];
static const unsigned char OLE_PAGE[8]={0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7};
static const unsigned char OLE_TEST[6]={0x6E,0x89,0x89,0x89,0x55,0x0};

//_____ P U B L I C   F U N C T I O N S _____________________________________
// public interface functions


void OLD_Init(void)
{
  	//Taken from datasheet for display module QG-2864KSWLG01
	//Configure for internal charge pump for "backlight" control
	IOEC_ControlExternalOutput(extIO_OLEDRstPort, extIO_OLEDRstPin, 1);
	vTaskDelay(10 / portTICK_PERIOD_MS);
	IOEC_ControlExternalOutput(extIO_OLEDRstPort, extIO_OLEDRstPin, 0);
	vTaskDelay(10 / portTICK_PERIOD_MS);
	IOEC_ControlExternalOutput(extIO_OLEDRstPort, extIO_OLEDRstPin, 1);
	vTaskDelay(10 / portTICK_PERIOD_MS);

	write_ix(0xAE);    /*display off*/

	write_ix(0x02);    /*set lower column address*/
	write_ix(0x10);    /*set higher column address*/

	write_ix(0x40);    /*set display start line*/

	write_ix(0xB0);    /*set page address*/

	write_ix(0x81);    /*contract control*/
	write_ix(0xff);    /*128*/

	write_ix(0xA1);    /*set segment remap*/

	//write_ix(0xA5);		//Set entire display ON

	write_ix(0xA6);    /*normal / reverse*/

	write_ix(0xA8);    /*multiplex ratio*/
	write_ix(0x3F);    /*duty = 1/64*/

	write_ix(0xad);    /*set charge pump enable*/
	write_ix(0x8b);     /*    0x8B    ??VCC   */

	write_ix(0x33);    /*0X30---0X33  set VPP   9V */

	write_ix(0xC8);    /*Com scan direction*/

	write_ix(0xD3);    /*set display offset*/
	write_ix(0x00);   /*   0x20  */

	write_ix(0xD5);    /*set osc division*/
	write_ix(0x80);

	write_ix(0xD9);    /*set pre-charge period*/
	write_ix(0x1f);    /*0x22*/

	write_ix(0xDA);    /*set COM pins*/
	write_ix(0x12);

	write_ix(0xdb);    /*set vcomh*/
	write_ix(0x40);

	write_ix(0xAF);    /*display ON*/
}


void OLD_Refresh(void)     //��ole_view
{
	for(uint8_t i=0; i < 8; i++)
	{
		write_ix(OLE_PAGE[i]);
		write_ix(OLE_SCL);
		write_ix(OLE_SCH);
		for(uint8_t pxf=0; pxf < 128; pxf++)
		{
			write_dx(OLE_VIEW[i][pxf]);
		}
	}
}

 //_____ L O C A L  F U N C T I O N S ________________________________________
 // local interface functions
void write_ix(uint8_t ins)
{
	uint8_t i2cArrayOut[2] = {0x00, ins};
	I2C_write(i2cAddrOLED, i2cArrayOut, 2);
}

void write_dx(uint8_t dat)
{
	uint8_t i2cArrayOut[2] = {0x40, dat};
	I2C_write(i2cAddrOLED, i2cArrayOut, 2);
}
