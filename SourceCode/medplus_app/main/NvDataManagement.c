/**
@file			  NvDataManagement.c
@author			  JGS
@brief			  Non Volatile Data Management module related functions definitions

@copyright		  Pura

@attention		  The information contained herein is confidential property of
                  Pura Scents. The use, copy, transfer or disclosure
                  of such information is prohibited except by express written
                  agreement with Pura Scents.
*/


//_____I N C L U D E ________________________________________________________
// Place here all includes needed by this file starting with its own header
// showing the reason of the include.

#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "NvDataManagement.h"
#include "main.h"
#include "med_plus_config.h"
#include "esp_log.h"
//_____D E F I N E __________________________________________________________
// local private defines, enums, macros
#define STORAGE_NAMESPACE "storage"

#define TAG "NVM"
//_____ T Y P E D E F _______________________________________________________
// local private typedefs, structs, unions


//_____ P R O T O T Y P E S _________________________________________________
// local function declarations
void nvDM_updateEEPROMEvent(void);
void nvDM_saveEEPROMIfUpdated(void);
void nvDM_SaveData(void);
void nvDM_updateAndSaveEEPROM(void);

//_____ L O C A L  D A T A __________________________________________________
// local private data (static)
NvDataStruct nvData;

uint8_t OperatingEnvironment = OPERATION_DEV_ENVIRONMENT;

bool update_EEPROM_flag = false;

uint8_t timeCounter = 0;

const NvDataStruct nvData_Init = {
	//// versions
	0, // Reserved
	0, // Reserved
	0, // Reserved
	0, // Reserved
};



//_____ P U B L I C   F U N C T I O N S _____________________________________
// public interface functions


/**
  @brief            NvDM Init
  @details          Initialize Non Volatile Data Management module
  @param[in]        None
  @param[out]       None
  @param[in,out]    None
  @retval  type     None
*/
 void NvDM_Init(void)
 {
	 nvs_handle my_handle;
	 esp_err_t err;
	 size_t nv_data_required_size;

     err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);

     //NvVersionsDataStruct Read or Initialization
     err = nvs_get_blob(my_handle, "nv_data", NULL, &nv_data_required_size);
	 if ((nv_data_required_size == 0) || (nv_data_required_size!=sizeof(nvData)))
	 {
		 ESP_LOGI(TAG,"No information saved yet!\n");
		 memcpy((void *)&nvData, (void *)&nvData_Init, sizeof(NvDataStruct));
		 nvs_set_blob(my_handle, "nv_data", (void *)&nvData, sizeof(nvData));
		 nvs_commit(my_handle);

		 ESP_LOGI(TAG,"NvM structures Initialized to memory!\n");
	 }
	 else
	 {
		 err = nvs_get_blob(my_handle, "nv_data", (void *)&nvData, &nv_data_required_size);
	 }



	//Operating Environment Read
	 err = nvs_get_blob(my_handle, "nv_op_env", NULL, &nv_data_required_size);
	 if ((nv_data_required_size == 0) || (nv_data_required_size!=sizeof(OperatingEnvironment)))
	 {
		 ESP_LOGI(TAG,"Operating Env info not saved yet!\n");
		 OperatingEnvironment = OPERATION_DEV_ENVIRONMENT;
		 nvs_set_blob(my_handle, "nv_op_env", (void *)&OperatingEnvironment, sizeof(OperatingEnvironment));
		 nvs_commit(my_handle);

		 ESP_LOGI(TAG,"Operating Env info Initialized to memory!\n");
	 }
	 else
	 {
		 err = nvs_get_blob(my_handle, "nv_op_env", (void *)&OperatingEnvironment, &nv_data_required_size);

	 }

	 nvs_close(my_handle);
 }


 void NvDM_ResetDefaults(void)
 {
	 nvs_handle my_handle;

	 nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);


	 memcpy((void *)&nvData, (void *)&nvData_Init, sizeof(NvDataStruct));
	 nvs_set_blob(my_handle, "nv_data", (void *)&nvData, sizeof(nvData));
	 nvs_commit(my_handle);

	 nvs_close(my_handle);
 }
 /**
   @brief            NvDM_10msTask
   @details          Periodic 10ms task to be called by Scheduler
   @param[in]        None
   @param[out]       None
   @param[in,out]    None
   @retval  type     None
 */
 void NvDM_10msTask(void)
 {
	 timeCounter++;
	 if(timeCounter >= 10)
	 {
		 nvDM_saveEEPROMIfUpdated();
		 timeCounter = 0;
	 }
 }

 void NvDM_SetOperatingEnvironment(uint8_t operatingEnv)
 {
	 nvs_handle my_handle;

	 OperatingEnvironment = operatingEnv;

	 nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);

	 nvs_set_blob(my_handle, "nv_op_env", (void *)&OperatingEnvironment, sizeof(OperatingEnvironment));
	 nvs_commit(my_handle);

	 nvs_close(my_handle);
 }

 uint8_t NvDM_GetOperatingEnvironment(void)
 {
	 return OperatingEnvironment;
 }

 //_____ L O C A L  F U N C T I O N S ________________________________________
 // local interface functions


void nvDM_updateEEPROMEvent(void)
{
	update_EEPROM_flag = TRUE;
}

void nvDM_saveEEPROMIfUpdated(void)
{
	if (update_EEPROM_flag)
	{
		ESP_LOGI(TAG,"EEPROM Data Written");
	  // The object data is first compared to the data written in the EEPROM to avoid writing values that haven't changed.
	  nvDM_SaveData();
	  update_EEPROM_flag = FALSE;

	}
}

void nvDM_SaveData(void)
{
	 nvs_handle my_handle;

	 nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);

	 nvs_set_blob(my_handle, "nv_data", (void *)&nvData, sizeof(nvData));
	 nvs_commit(my_handle);

	 nvs_close(my_handle);
}

void nvDM_updateAndSaveEEPROM(void)
{
	nvDM_updateEEPROMEvent();
	nvDM_saveEEPROMIfUpdated();
}
