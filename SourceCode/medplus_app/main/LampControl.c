/**
@file			  LampControl.c
@author			  JGS
@brief			  Lamp control service module related functions definitions

@copyright		  PSDI

@attention		  The information contained herein is confidential property of
                  PSDI. The use, copy, transfer or disclosure
                  of such information is prohibited except by express written
                  agreement with Directed Electronics Inc.
*/


//_____I N C L U D E ________________________________________________________
// Place here all includes needed by this file starting with its own header
// showing the reason of the include.
#include "driver/gpio.h"
#include "LampControl.h"
#include "IOExtenderControl.h"

//_____D E F I N E __________________________________________________________
// local private defines, enums, macros
#define LMPCTRL_UVC_ENABLE_PORT		GPIO_NUM_16

//_____ T Y P E D E F _______________________________________________________
// local private typedefs, structs, unions


//_____ P R O T O T Y P E S _________________________________________________
// local function declarations

//_____ L O C A L  D A T A __________________________________________________
// local private data (static)
static int timecounter=0;

//_____ P U B L I C   F U N C T I O N S _____________________________________
// public interface functions


/**
  @brief            LampCtrl_Init
  @details          Initialize Lamp control service module
  @param[in]        None
  @param[out]       None
  @param[in,out]    None
  @retval  type     None
*/
 void LampCtrl_Init(void)
 {
	 gpio_set_direction(LMPCTRL_UVC_ENABLE_PORT, GPIO_MODE_OUTPUT);
	 LampCtrl_TurnOFF();
 }


 /**
   @brief            LampCtrl_TurnON
   @details          Turn ON UVC Lamps
   @param[in]        None
   @param[out]       None
   @param[in,out]    None
   @retval  type     None
 */
void LampCtrl_TurnON(void)
{
	gpio_set_level(LMPCTRL_UVC_ENABLE_PORT, 1);
	//IOEC_ControlExternalOutput(extIO_FanPort, extIO_FanPin,1);
	//IOEC_ControlExternalOutput(extIO_FanPort, extIO_FanPin,0);
}

 /**
    @brief            LampCtrl_TurnOFF
    @details          Turn OFF UVC Lamps
    @param[in]        None
    @param[out]       None
    @param[in,out]    None
    @retval  type     None
 */
void LampCtrl_TurnOFF(void)
{
	gpio_set_level(LMPCTRL_UVC_ENABLE_PORT, 0);
	//IOEC_ControlExternalOutput(extIO_FanPort, extIO_FanPin,0);
}

/**
  @brief            LampCtrl_10msTask
  @details          Periodic 10ms task to be called by Scheduler
  @param[in]        None
  @param[out]       None
  @param[in,out]    None
  @retval  type     None
*/
void LampCtrl_10msTask(void)
{
	if (timecounter == 0)
	{
		//LampCtrl_TurnON();
	}
	else if(timecounter == 1000) //10 seconds
	{
		//LampCtrl_TurnOFF();
	}
	timecounter++;
	if(timecounter >= 2000) //20 seconds
	{
		timecounter = 0;
	}
}


 //_____ L O C A L  F U N C T I O N S ________________________________________
 // local interface functions