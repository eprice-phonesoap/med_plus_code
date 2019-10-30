/**
@file			  med_plus_config.h
@author			  Jey
@brief			  Definition for use in main.c
*/

#ifndef CONFIG_H_
#define CONFIG_H_

//_____I N C L U D E ________________________________________________________
// mandatory includes needed by this file (hard dependencies)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Platform_Types.h"
#include "Std_Types.h"

//_____D E F I N E __________________________________________________________
// public defines, enums and macros

//Select anyone environment from below 2 options suitable.
#define OPERATION_DEV_ENVIRONMENT 0
#define OPERATION_STAGE_ENVIRONMENT 1
#define OPERATION_PROD_ENVIRONMENT 2

#define DEV_OTA_SERVER_HOST_NAME      "devices.vectortac.com"

#define STAGE_OTA_SERVER_HOST_NAME     "devices.vectortac.com"

#define PROD_OTA_SERVER_HOST_NAME      "devices.vectortac.com"

#define COMPILE_FOR_DEV


#ifdef COMPILE_FOR_DEV
	#define CURRENT_OPERATING_ENVIRONMENT OPERATION_DEV_ENVIRONMENT

	#define DEV_OTA_SOFTWARE_VERSION         1		// Used by the OTA module to check if the current version is different from the version
											// on the server, i.e. if an upgrade or downgrade should be performed.
	#define DEV_SW_VERSION_MAJOR 0
	#define DEV_SW_VERSION_MINOR 1

	#define OTA_SOFTWARE_VERSION DEV_OTA_SOFTWARE_VERSION

	#define SW_VERSION_MAJOR DEV_SW_VERSION_MAJOR
	#define SW_VERSION_MINOR DEV_SW_VERSION_MINOR

	#define HEARTBEAT_RATE 20 //20 seconds
#endif

#ifdef COMPILE_FOR_STAGE
	#define CURRENT_OPERATING_ENVIRONMENT OPERATION_STAGE_ENVIRONMENT

	#define STAGE_OTA_SOFTWARE_VERSION      1 	// Used by the OTA module to check if the current version is different from the version
											// on the server, i.e. if an upgrade or downgrade should be performed.
	#define STAGE_SW_VERSION_MAJOR 0
	#define STAGE_SW_VERSION_MINOR 1

	#define OTA_SOFTWARE_VERSION STAGE_OTA_SOFTWARE_VERSION

	#define SW_VERSION_MAJOR STAGE_SW_VERSION_MAJOR
	#define SW_VERSION_MINOR STAGE_SW_VERSION_MINOR

	#define HEARTBEAT_RATE 300 //5 Mins
#endif

#ifdef COMPILE_FOR_PROD
	#define CURRENT_OPERATING_ENVIRONMENT OPERATION_PROD_ENVIRONMENT

	#define PROD_OTA_SOFTWARE_VERSION          1 	// Used by the OTA module to check if the current version is different from the version
											// on the server, i.e. if an upgrade or downgrade should be performed.
	#define PROD_SW_VERSION_MAJOR 0
	#define PROD_SW_VERSION_MINOR 1

	#define OTA_SOFTWARE_VERSION PROD_OTA_SOFTWARE_VERSION

	#define SW_VERSION_MAJOR PROD_SW_VERSION_MAJOR
	#define SW_VERSION_MINOR PROD_SW_VERSION_MINOR

	#define HEARTBEAT_RATE 300 //5 Mins
#endif
#define EEPROM_VERSION 1

//_____ T Y P E D E F _______________________________________________________
// public typedefs, structs, unions

//_____ P R O T O T Y P E S _________________________________________________
// public interface (API)

//_____ P R O T O T Y P E S _________________________________________________
// local interface (API)

#endif /* MED_PLUS_V2_CONFIG_H_ */
