/**
@file			  LedControl.h
@author			  JGS
@brief			  LED Control Interface related structure definitions and function prototype declaration

@copyright		  PSDI

@attention		  The information contained herein is confidential property of
                  PSDI. The use, copy, transfer or disclosure
                  of such information is prohibited except by express written
                  agreement with Directed Electronics Inc.
*/

#ifndef LEDCONTROL_H_
#define LEDCONTROL_H_

//_____I N C L U D E ________________________________________________________
// mandatory includes needed by this file (hard dependencies)
#include "Std_Types.h"

//_____D E F I N E __________________________________________________________
// public defines, enums and macros

//_____ T Y P E D E F _______________________________________________________
// public typedefs, structs, unions
typedef enum
{
	LEDCTRL_EVENT_NONE=0, //No LED activity
	LEDCTRL_EVENT_SELF_TEST,
	LEDCTRL_EVENT_ALL_OFF , //Turn OFF All LEDs
	LEDCTRL_EVENT_WIFI_CREDENTIALS_OK, //Blue blink 2Hz
	LEDCTRL_EVENT_WIFI_NOT_OK, //Red blink 2Hz
	LEDCTRL_EVENT_WIFI_CONNECTED, //Green blink 3 times in 1 second
	LEDCTRL_EVENT_ENTERING_SETUP, //Blink green/blue alternatively 2Hz
	LEDCTRL_EVENT_CLEANING_IN_PROGRESS,
	LEDCTRL_EVENT_CLEANING_NEARING_COMPLETION,
	LEDCTRL_EVENT_CLEANING_COMPLETE,
	LEDCTRL_EVENT_CLEANING_ABORTED,
	LEDCTRL_EVENT_DOOR_MOVING,
	LEDCTRL_EVENT_DOOR_OPEN,
}LEDCTRL_EVENTS;

//_____ P R O T O T Y P E S _________________________________________________
// public interface (API)
void LedCtrl_Init(void);
void LedCtrl_SetMode(LEDCTRL_EVENTS event);
void LedCtrl_10msTask(void);

//_____ P R O T O T Y P E S _________________________________________________
// local interface (API)

#endif /* LEDCONTROL_H_ */
