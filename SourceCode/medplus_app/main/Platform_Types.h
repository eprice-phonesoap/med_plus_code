/**
@file			  Platform_Types.h
@author			  Jey
@brief			  Platform Types declarations

@copyright		  Pura Scents

@attention		  The information contained herein is confidential property of 
                  Pura. The use, copy, transfer or disclosure
                  of such information is prohibited except by express written 
                  agreement with Pura.
*/

 #ifndef PLATFORM_TYPES_H
 #define PLARFORM_TYPES_H

//_____I N C L U D E ________________________________________________________
// mandatory includes needed by this file (hard dependencies)


//_____D E F I N E __________________________________________________________
// public defines, enums and macros
#define CPU_TYPE_8 		8
#define CPU_TYPE_16 		16
#define CPU_TYPE_32 		32
#define MSB_FIRST 			0
#define LSB_FIRST 			1
#define HIGH_BYTE_FIRST	0
#define LOW_BYTE_FIRST 	1

#define CPU_TYPE 			CPU_TYPE_32
#define CPU_BIT_ORDER 		MSB_FIRST
#define CPU_BYTE_ORDER 	HIGH_BYTE_FIRST

#define TRUE				1
#define FALSE				0

#define MILLI_SECONDS_IN_10MS_LOOP(x)	  (unsigned int) (x/10)
#define SECONDS_IN_10MS_LOOP(x)	  (unsigned int) ((x*1000)/10)
#define MINUTES_IN_10MS_LOOP(x)	  (unsigned int) ((x*60*1000)/10)

//_____ T Y P E D E F _______________________________________________________
// public typedefs, structs, unions


typedef unsigned char 		boolean;

typedef signed char 		sint8;
typedef unsigned char 		uint8;
typedef signed short 		sint16;
typedef unsigned short 	uint16;
typedef signed long 		sint32;
typedef signed long long 	sint64;
typedef unsigned long 		uint32;
typedef unsigned long long uint64;

typedef unsigned long 		uint8_least;
typedef unsigned long 		uint16_least;
typedef unsigned long 		uint32_least;
typedef signed long 		sint8_least;
typedef signed long 		sint16_least;
typedef signed long 		sint32_least;

typedef float 				float32;
typedef double 			float64;

 #endif /* PLATFORM_TYPES_H */
