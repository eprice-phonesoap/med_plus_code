/**
@file			  LampControl.h
@author			  JGS
@brief			  Lamp Control Interface related structure definitions and function prototype declaration

@copyright		  PSDI

@attention		  The information contained herein is confidential property of
                  PSDI. The use, copy, transfer or disclosure
                  of such information is prohibited except by express written
                  agreement with Directed Electronics Inc.
*/

#pragma once

//_____I N C L U D E ________________________________________________________
// mandatory includes needed by this file (hard dependencies)
#include "Std_Types.h"

//_____D E F I N E __________________________________________________________
// public defines, enums and macros

//_____ T Y P E D E F _______________________________________________________
// public typedefs, structs, unions
typedef enum
{
	BEEP_IDLE,
	BEEP_START,
	BEEP_DOWORK,
}BEEP_CONTROL_EVENT;

//_____ P R O T O T Y P E S _________________________________________________
// public interface (API)
void BeepControl_Init(void);
void BeepControl_TurnON(void);
void BeepControl_TurnOFF(void);
void BeepControl_10msTask(void);
void BeepControl_SetEvent(BEEP_CONTROL_EVENT event);

//_____ P R O T O T Y P E S _________________________________________________
// local interface (API)


