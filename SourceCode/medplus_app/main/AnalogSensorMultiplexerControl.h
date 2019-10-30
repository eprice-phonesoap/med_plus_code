/**
@file			  AnalogSensorMultiplexer.h
@author			  JGS
@brief			  Analog Sensor Management related structure definitions and function prototype declaration
*/

#ifndef ASMC_H_
#define ASMC_H_

//_____I N C L U D E ________________________________________________________
// mandatory includes needed by this file (hard dependencies)
#include "Std_Types.h"

//_____D E F I N E __________________________________________________________
// public defines, enums and macros
#define UV_MUX_CH_1 0
#define UV_MUX_CH_2 1
#define UV_MUX_CH_3 2
#define UV_MUX_CH_4 3
#define UV_MUX_CH_5 4
#define UV_MUX_CH_6 5
#define UV_MUX_CH_7 6
#define UV_MUX_CH_8 7

//_____ T Y P E D E F _______________________________________________________
// public typedefs, structs, unions


//_____ P R O T O T Y P E S _________________________________________________
// public interface (API)
void ASMC_Init(void);
void ASMC_SelectMUX(uint8 channel);
//_____ P R O T O T Y P E S _________________________________________________
// local interface (API)

#endif /* ASMC_H_ */
