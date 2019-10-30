/**
@file			  Main.h
@author			  Jey
@brief			  Definition for use in main.c

@copyright		  Pura Scents

@attention		  The information contained herein is confidential property of
                  Pura. The use, copy, transfer or disclosure
                  of such information is prohibited except by express written
                  agreement with Pura.
*/

#ifndef MAIN_H_
#define MAIN_H_

//_____I N C L U D E ________________________________________________________
// mandatory includes needed by this file (hard dependencies)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "Platform_Types.h"
#include "Std_Types.h"

#include "iap_https.h"  // Coordinating firmware updates

#include "LedControl.h"
#include "AnalogSensorManagement.h"
#include "BluetoothDataManagement.h"
#include "WiFiDataManagement.h"
#include "OTAManagement.h"

//_____D E F I N E __________________________________________________________
// public defines, enums and macros


//_____ T Y P E D E F _______________________________________________________
// public typedefs, structs, unions

//_____ P R O T O T Y P E S _________________________________________________
// public interface (API)
void Main_GetSoftwareVersion(uint8_t * sw_version);

//_____ P R O T O T Y P E S _________________________________________________
// local interface (API)

#endif /* MAIN_H_ */
