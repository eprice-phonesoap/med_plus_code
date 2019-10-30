/**
@file			  WiFiDataManagement.h
@author			  JGS
@brief			  WiFi Data Management related structure definitions and function prototype declaration

@copyright		  PSDI

@attention		  The information contained herein is confidential property of
                  PSDI. The use, copy, transfer or disclosure
                  of such information is prohibited except by express written
                  agreement with Directed Electronics Inc.
*/

#ifndef WIFI_DM_H_
#define WIFI_DM_H_

//_____I N C L U D E ________________________________________________________
// mandatory includes needed by this file (hard dependencies)
#include "Std_Types.h"

//_____D E F I N E __________________________________________________________
// public defines, enums and macros

//_____ T Y P E D E F _______________________________________________________
// public typedefs, structs, unions


//_____ P R O T O T Y P E S _________________________________________________
// public interface (API)
void WifiDm_Init(void);
void WifiDm_GetMac(char * data);
uint8 WifiDm_GetConnectionStatus(void);
void WifiDm_GetMacRaw(uint8_t * data);
uint8 WiFiDm_IsCredentialsPresent(void);
void WiFiDm_ClearCredentials(void);

uint8 WiFiDm_Connect(void);
void WiFiDm_Reconnect(void);
void WiFiDm_Disconnect(void);

void WiFiDm_ScanStart(uint8_t NativeScanFlag);
uint8_t WiFiDm_GetScanStatus(void);
void WiFiDm_ScanStop(void);

//_____ P R O T O T Y P E S _________________________________________________
// local interface (API)

#endif /* WIFI_DM_H_ */
