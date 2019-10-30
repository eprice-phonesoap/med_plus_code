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

#ifndef LAMPCONTROL_H_
#define LAMPCONTROL_H_

//_____I N C L U D E ________________________________________________________
// mandatory includes needed by this file (hard dependencies)
#include "Std_Types.h"

//_____D E F I N E __________________________________________________________
// public defines, enums and macros

//_____ T Y P E D E F _______________________________________________________
// public typedefs, structs, unions


//_____ P R O T O T Y P E S _________________________________________________
// public interface (API)
void LampCtrl_Init(void);
void LampCtrl_TurnON(void);
void LampCtrl_TurnOFF(void);
void LampCtrl_10msTask(void);

//_____ P R O T O T Y P E S _________________________________________________
// local interface (API)

#endif /* LAMPCONTROL_H_ */
