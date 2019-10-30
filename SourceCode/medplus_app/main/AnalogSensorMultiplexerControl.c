/**
@file			  AnalogSensorMultiplexerControl.c
@author			  JGS
@brief			  Analog Sensor Management module related functions definitions
*/


//_____I N C L U D E ________________________________________________________
// Place here all includes needed by this file starting with its own header
// showing the reason of the include.

#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/rtc_io_reg.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "esp_log.h"

#include "AnalogSensorMultiplexerControl.h"

//_____D E F I N E __________________________________________________________
// local private defines, enums, macros
#define UV_MUX_SELA	GPIO_NUM_12
#define UV_MUX_SELB	GPIO_NUM_13
#define UV_MUX_SELC	GPIO_NUM_14

//_____ T Y P E D E F _______________________________________________________
// local private typedefs, structs, unions


//_____ P R O T O T Y P E S _________________________________________________
// local function declarations

//_____ L O C A L  D A T A __________________________________________________
// local private data (static)
const uint8 selc[8] = {0,0,0,0,1,1,1,1};
const uint8 selb[8] = {0,0,1,1,0,0,1,1};
const uint8 sela[8] = {0,1,0,1,0,1,0,1};

//_____ P U B L I C   F U N C T I O N S _____________________________________
// public interface functions
void ASMC_Init(void)
{
	gpio_pad_select_gpio(UV_MUX_SELA);
	gpio_pad_select_gpio(UV_MUX_SELB);
	gpio_pad_select_gpio(UV_MUX_SELC);

	 //Outputs
	gpio_set_direction(UV_MUX_SELA, GPIO_MODE_OUTPUT);
	gpio_set_direction(UV_MUX_SELB, GPIO_MODE_OUTPUT);
	gpio_set_direction(UV_MUX_SELC, GPIO_MODE_OUTPUT);
}

void ASMC_SelectMUX(uint8 channel)
{
	gpio_set_level(UV_MUX_SELA, sela[channel]);
	gpio_set_level(UV_MUX_SELB, selb[channel]);
	gpio_set_level(UV_MUX_SELC, selc[channel]);

}

 //_____ L O C A L  F U N C T I O N S ________________________________________
 // local interface functions
