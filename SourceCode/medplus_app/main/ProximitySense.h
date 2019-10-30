/**
@file			  UVCleaningApp.h
@author			  JGS
@brief			  Uv Cleaning Application related structure definitions and function prototype declaration

*/

#ifndef PROX_SENSE_H_
#define PROX_SENSE_H_

//_____I N C L U D E ________________________________________________________
// mandatory includes needed by this file (hard dependencies)
#include "Std_Types.h"


//_____D E F I N E __________________________________________________________
// public defines, enums and macros
#define BTN_STATE_OFF 0
#define BTN_STATE_SHORT_PRESS 1   //500msec to 1sec second press
#define BTN_STATE_LONG_PRESS 2    //2 sec to 4 sec
#define BTN_FIVE_SECOND_PRESS 3

typedef uint8 BtnState_Type;

//_____ T Y P E D E F _______________________________________________________
// public typedefs, structs, unions

//_____ P R O T O T Y P E S _________________________________________________
// public interface (API)
void ProximitySense_Init(void);
uint8 ProximitySense_ReadLevel(void);
void ProximitySense_1msTasks(void);

//_____ P R O T O T Y P E S _________________________________________________
// local interface (API)

#endif /* PROX_SENSE_H_ */
