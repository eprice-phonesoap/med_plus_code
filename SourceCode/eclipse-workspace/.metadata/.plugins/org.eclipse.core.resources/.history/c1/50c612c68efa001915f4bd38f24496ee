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
#include "IOExtenderControl.h"
#include "BeepControl.h"

//_____D E F I N E __________________________________________________________
// local private defines, enums, macros

//_____ T Y P E D E F _______________________________________________________
// local private typedefs, structs, unions


//_____ P R O T O T Y P E S _________________________________________________
// local function declarations

//_____ L O C A L  D A T A __________________________________________________
// local private data (static)
static int timecounter=0;
static BEEP_CONTROL_EVENT mBEEP_CONTROL_EVENT = BEEP_IDLE;
//_____ P U B L I C   F U N C T I O N S _____________________________________
// public interface functions


void BeepControl_SetEvent(BEEP_CONTROL_EVENT event)
{
	mBEEP_CONTROL_EVENT = event;
}


/**
  @brief            BeepControl_Init
  @details          Initialize Beep control service module
  @param[in]        None
  @param[out]       None
  @param[in,out]    None
  @retval  type     None
*/
 void BeepControl_Init(void)
 {
	 BeepControl_TurnOFF();
 }


 /**
   @brief            BeepControl_TurnON
   @details
   @param[in]        None
   @param[out]       None
   @param[in,out]    None
   @retval  type     None
 */
void BeepControl_TurnON(void)
{
	IOEC_ControlExternalOutput(extIO_PiezoPort, extIO_PiezoPin,1);
}
void fanControl_TurnOn(void)
{

}

 /**
    @brief            BeepControl_TurnOFF
    @details
    @param[in]        None
    @param[out]       None
    @param[in,out]    None
    @retval  type     None
 */
void BeepControl_TurnOFF(void)
{
	IOEC_ControlExternalOutput(extIO_PiezoPort, extIO_PiezoPin,0);
}

/**
  @brief            BeepControl_10msTask
  @details          Periodic 10ms task to be called by Scheduler
  @param[in]        None
  @param[out]       None
  @param[in,out]    None
  @retval  type     None
*/
void BeepControl_10msTask(void)
{
	switch(mBEEP_CONTROL_EVENT)
	{
	case BEEP_START:
		timecounter = 0;
		mBEEP_CONTROL_EVENT = BEEP_DOWORK;
		BeepControl_TurnON();
		break;
	case BEEP_DOWORK:
		timecounter++;
		if(timecounter == 10)
		{
			BeepControl_TurnOFF();
			mBEEP_CONTROL_EVENT = BEEP_IDLE;
		}
		break;
	default:
		break;
	}
}


 //_____ L O C A L  F U N C T I O N S ________________________________________
 // local interface functions
