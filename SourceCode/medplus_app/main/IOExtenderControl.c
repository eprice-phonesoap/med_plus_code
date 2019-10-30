/**
@file			  IOExtenderControl.c
@author			  JGS
@brief			  Motor control service module related functions definitions
*/


//_____I N C L U D E ________________________________________________________
// Place here all includes needed by this file starting with its own header
// showing the reason of the include.
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"

#include "I2C_Driver.h"
#include "IOExtenderControl.h"


//_____D E F I N E __________________________________________________________
// local private defines, enums, macros

//Local defines
#define IO_IORESET 33
#define i2cAddrTCA 0x22		//I2C address, 7-bit

//Commands - don't forget auto-increment state! See datasheet pg. 9 table 5
#define REG_INPUT0 0x00
#define REG_INPUT1 0x01
#define REG_INPUT2 0x02
#define REG_OUTPUT0 0x04
#define REG_OUTPUT1 0x05
#define REG_OUTPUT2 0x06
#define REG_INVERTPORT0 0x08
#define REG_INVERTPORT1 0x09
#define REG_INVERTPORT2 0x0A
#define REG_CONFPORT0 0x0C
#define REG_CONFPORT1 0x0D
#define REG_CONFPORT2 0x0E


//Port configuration
#define ioConfOutput 0
#define ioConfInput  1


//_____ T Y P E D E F _______________________________________________________
// local private typedefs, structs, unions

//_____ P R O T O T Y P E S _________________________________________________
// local function declarations
void IOEC_SendCommand(uint8_t commandByte, uint8_t dataToSend);

//_____ L O C A L  D A T A __________________________________________________
// local private data (static)

uint8_t i2c_bytes_to_send[8] = {0, 0, 0, 0, 0, 0, 0, 0};

volatile uint8_t DDR_PORT0 = 0xFF;
volatile uint8_t DDR_PORT1 = 0xFF;
volatile uint8_t DDR_PORT2 = 0xFF;

volatile uint8_t LVL_PORT0 = 0x00;
volatile uint8_t LVL_PORT1 = 0x00;
volatile uint8_t LVL_PORT2 = 0x00;

//_____ P U B L I C   F U N C T I O N S _____________________________________
// public interface functions


void IOEC_Init(void)
{
	//Local IO setup
	gpio_config_t io_conf;

	//Outputs
	io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
	//bit mask of the pins
	io_conf.pin_bit_mask = (1ULL << IO_IORESET);
	//set as input mode
	io_conf.mode = GPIO_MODE_OUTPUT;
	//enable pull-up mode
	io_conf.pull_up_en = 0;
	io_conf.pull_down_en = 0;
	gpio_config(&io_conf);

	//Init IO expander IC
	gpio_set_level(IO_IORESET, 1);		//Disable chip reset of IO expander

	//Setup IO
	DDR_PORT0 = 0x00;		//Port 0 unused
	DDR_PORT1 &= ~((1 << extIO_RFIDRSTPin) | (1 << extIO_FanPin) | (1 << extIO_PiezoPin) | (1 << extIO_OLEDRstPin) | (1 << extIO_STEPRstPin) | (1 << extIO_STEPSleepPin));
	DDR_PORT2 &= ~((1 << extIO_STEPDirPin) | (1 << extIO_STEPEnaPin) | (0 << extIO_STEPMode0Pin) | (0 << extIO_STEPMode1Pin) | (0 << extIO_STEPMode2Pin));


	IOEC_SendCommand(REG_OUTPUT0, LVL_PORT0);
	IOEC_SendCommand(REG_OUTPUT1, LVL_PORT1);
	IOEC_SendCommand(REG_OUTPUT2, LVL_PORT2);
	IOEC_SendCommand(REG_CONFPORT0, DDR_PORT0);
	IOEC_SendCommand(REG_CONFPORT1, DDR_PORT1);
	IOEC_SendCommand(REG_CONFPORT2, DDR_PORT2);
}


void IOEC_ControlExternalOutput(uint8_t ioPortToSet, uint8_t ioPinToSet, uint8_t ioLevelReq)
{
	//Set external IO on TCA6424A IO expander
	switch (ioPortToSet)
	{
		case 0:
			if (ioLevelReq)
			{
				//Set high level
				LVL_PORT0 |= (1 << ioPinToSet);
			}
			else
			{
				//Set low level
				LVL_PORT0 &= ~(1 << ioPinToSet);
			}
			IOEC_SendCommand(REG_OUTPUT0, LVL_PORT0);
		break;

		case 1:
			if (ioLevelReq)
			{
				//Set high level
				LVL_PORT1 |= (1 << ioPinToSet);
			}
			else
			{
				//Set low level
				LVL_PORT1 &= ~(1 << ioPinToSet);
			}
			IOEC_SendCommand(REG_OUTPUT1, LVL_PORT1);
		break;

		case 2:
			if (ioLevelReq)
			{
				//Set high level
				LVL_PORT2 |= (1 << ioPinToSet);
			}
			else
			{
				//Set low level
				LVL_PORT2 &= ~(1 << ioPinToSet);
			}
			IOEC_SendCommand(REG_OUTPUT2, LVL_PORT2);
		break;
	}

}
 //_____ L O C A L  F U N C T I O N S ________________________________________
 // local interface functions

void IOEC_SendCommand(uint8_t commandByte, uint8_t dataToSend)
{
	uint8_t i2cArrayOut[2] = {commandByte, dataToSend};
	I2C_write(i2cAddrTCA, i2cArrayOut, 2);
}
