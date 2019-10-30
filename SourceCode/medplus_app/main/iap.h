/**
@file			  iap.h
@author			  Jey
@brief			  Definition for use in iap.c

@copyright		  Pura Scents

@attention		  The information contained herein is confidential property of
                  Pura. The use, copy, transfer or disclosure
                  of such information is prohibited except by express written
                  agreement with Pura.
*/

#ifndef IAP_H_
#define IAP_H_

//_____I N C L U D E ________________________________________________________
// mandatory includes needed by this file (hard dependencies)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "soc/uart_struct.h"

#include "esp_ota_ops.h"
#include "iap.h"

#include "Platform_Types.h"
#include "Std_Types.h"

//_____D E F I N E __________________________________________________________
// public defines, enums and macros
#define IAP_OK      0
#define IAP_FAIL    -1
#define IAP_ERR_ALREADY_INITIALIZED     0x101
#define IAP_ERR_NOT_INITIALIZED         0x102
#define IAP_ERR_SESSION_ALREADY_OPEN    0x103
#define IAP_ERR_OUT_OF_MEMORY           0x104
#define IAP_ERR_NO_SESSION              0x105
#define IAP_ERR_PARTITION_NOT_FOUND     0x106
#define IAP_ERR_WRITE_FAILED            0x107

//_____ T Y P E D E F _______________________________________________________
// public typedefs, structs, unions
typedef int32_t iap_err_t;

//_____ P R O T O T Y P E S _________________________________________________
// public interface (API)
// Call once at application startup, before calling any other function of this module.
iap_err_t iap_init();

// Call to start a programming session.
// Sets the programming pointer to the start of the next OTA flash partition.
iap_err_t iap_begin();

// Call to write a block of data to the current location in flash.
// If the write fails, you need to abort the current programming session
// with 'iap_abort' and start again from the beginning.
iap_err_t iap_write(uint8_t *bytes, uint16_t len);

// Call to close a programming session and activate the programmed partition.
iap_err_t iap_commit();

// Abort the current programming session.
iap_err_t iap_abort();

//_____ P R O T O T Y P E S _________________________________________________
// local interface (API)

#endif /* IAP_H_ */
