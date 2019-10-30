/**
@file			  UVCleaningApp.h
@author			  JGS
@brief			  Uv Cleaning Application related structure definitions and function prototype declaration
*/

#ifndef OLED_MANAGER_H_
#define OLED_MANAGER_H_

//_____I N C L U D E ________________________________________________________
// mandatory includes needed by this file (hard dependencies)
#include "Std_Types.h"

//_____D E F I N E __________________________________________________________
// public defines, enums and macros
#define OLM_EVENT_NONE 0
#define OLM_EVENT_DISP_OPENING 1
#define OLM_EVENT_DISP_CLOSING 2
#define OLM_EVENT_DISP_BLOCK 3
#define OLM_EVENT_DISP_READY 4
#define OLM_EVENT_DISP_WORK 5
#define OLM_EVENT_DISP_WORKCOMP 6
#define OLM_EVENT_DISP_DJC 7
#define OLM_EVENT_DISP_CFG 8


//_____ T Y P E D E F _______________________________________________________
// public typedefs, structs, unions

//_____ P R O T O T Y P E S _________________________________________________
// public interface (API)

void OLM_Init(void);
void OLM_10ms_Tasks(void);
void OLM_SetEvent(uint8 Event);
void OLD_SetCount(uint8 count);

//_____ P R O T O T Y P E S _________________________________________________
// local interface (API)

#endif /* OLED_MANAGER_H_ */
