/**
@file			  Std_Types.h
@author			  Jey
@brief			  Standard types declarations

@copyright		  Pura Scents

@attention		  The information contained herein is confidential property of 
                  Pura. The use, copy, transfer or disclosure
                  of such information is prohibited except by express written 
                  agreement with Pura.
*/	
 
 
 #ifndef STD_TYPES_H
 #define STD_TYPES_H

 
//_____I N C L U D E ________________________________________________________
// mandatory includes needed by this file (hard dependencies)
#include "Platform_Types.h"


//_____D E F I N E __________________________________________________________
// public defines, enums and macros
#ifndef STATUSTYPEDEFINED
#define STATUSTYPEDEFINED

#define E_OK		0x00u
typedef unsigned char StatusType; /* OSEK compliance */

#endif
#define E_NOT_OK	0x01u

#define STD_HIGH	0x01u /*Physical state 5V or 3.3V */
#define STD_LOW	0x00u /*Physical state 0V */

#define STD_ACTIVE	0x01u /*Logical state active */
#define STD_IDLE	0x00u /*Logical state idle */

#define STD_ON		0x01u
#define STD_OFF	0x00u

//_____ T Y P E D E F _______________________________________________________
// public typedefs, structs, unions
 typedef uint8	Std_ReturnType;
 typedef struct
 {
	 uint16	vendorID;
	 uint16	moduleID;
	 uint8	sw_major_version;
	 uint8	sw_minor_version;
	 uint8	sw_patch_version;
 }Std_VersionInfoType;

 #endif /* STD_TYPES_H */
