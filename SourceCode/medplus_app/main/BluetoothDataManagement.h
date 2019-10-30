/**
@file			  BluetoothDataManagement.h
@author			  JGS
@brief			  Bluetooth Data Management related structure definitions and function prototype declaration

@copyright		  PSDI

@attention		  The information contained herein is confidential property of
                  PSDI. The use, copy, transfer or disclosure
                  of such information is prohibited except by express written
                  agreement with Directed Electronics Inc.
*/

#ifndef BT_DM_H_
#define BT_DM_H_

//_____I N C L U D E ________________________________________________________
// mandatory includes needed by this file (hard dependencies)
#include "Std_Types.h"

//_____D E F I N E __________________________________________________________
// public defines, enums and macros
#define BLUFI_DEMO_TAG "BLUFI_DEMO"
#define BLUFI_INFO(fmt, ...)   ESP_LOGI(BLUFI_DEMO_TAG, fmt, ##__VA_ARGS__)
#define BLUFI_ERROR(fmt, ...)  ESP_LOGE(BLUFI_DEMO_TAG, fmt, ##__VA_ARGS__)


#define MFG_DATA_NOT_ONBOARDED 0x00
#define MFG_DATA_ONBOARDED 0x01

//_____ T Y P E D E F _______________________________________________________
// public typedefs, structs, unions


//_____ P R O T O T Y P E S _________________________________________________
// public interface (API)
void BtDm_Init(void);
void BtDm_ReInit(void);
void BtDm_DeInit(void);
void BtDm_10msTask(void);
void blufi_dh_negotiate_data_handler(uint8_t *data, int len, uint8_t **output_data, int *output_len, bool *need_free);
int blufi_aes_encrypt(uint8_t iv8, uint8_t *crypt_data, int crypt_len);
int blufi_aes_decrypt(uint8_t iv8, uint8_t *crypt_data, int crypt_len);
uint16_t blufi_crc_checksum(uint8_t iv8, uint8_t *data, int len);

int blufi_security_init(void);
void blufi_security_deinit(void);
uint8 blufi_GetConnectionStatus(void);

void BtDm_EnableBluetooth(void);
void BtDm_DisableBluetooth(void);

void Set_Blufi_Adv_Data(uint8_t onboard_status);
//_____ P R O T O T Y P E S _________________________________________________
// local interface (API)

#endif /* BT_DM_H_ */
