/**
@file			  NvDataManagement.h
@author			  JGS
@brief			  Non Volatile Data Management related structure definitions and function prototype declaration

@copyright		  Pura

@attention		  The information contained herein is confidential property of
                  Pura Scents. The use, copy, transfer or disclosure
                  of such information is prohibited except by express written
                  agreement with Pura Scents.
*/

#ifndef NVDM_H_
#define NVDM_H_

//_____I N C L U D E ________________________________________________________
// mandatory includes needed by this file (hard dependencies)
#include "Std_Types.h"


//_____D E F I N E __________________________________________________________
// public defines, enums and macros

//_____ T Y P E D E F _______________________________________________________
// public typedefs, structs, unions
typedef struct {
	// versions
	uint16_t Reserved0;
	uint16_t Reserved1;
	uint8_t Reserved2;
	uint8_t Reserved3;
}NvDataStruct; //Structure used to store On-boarding, Light and Heater Information. Don't change the structure for any reason.

//_____ P R O T O T Y P E S _________________________________________________
// public interface (API)
void NvDM_Init(void);
void NvDM_10msTask(void);
void NvDM_ResetDefaults(void);

void NvDM_SetOperatingEnvironment(uint8_t operatingEnv);
uint8_t NvDM_GetOperatingEnvironment(void);

//_____ P R O T O T Y P E S _________________________________________________
// local interface (API)

#endif /* NVDM_H_ */
