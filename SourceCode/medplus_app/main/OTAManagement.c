/**
@file			  OTAManagement.c
@author			  JGS
@brief			  Over The Air Update module related functions definitions

@copyright		  Pura

@attention		  The information contained herein is confidential property of
                  Pura. The use, copy, transfer or disclosure
                  of such information is prohibited except by express written
                  agreement with Pura.
*/


//_____I N C L U D E ________________________________________________________
// Place here all includes needed by this file starting with its own header
// showing the reason of the include.
#include "OTAManagement.h"

#include "med_plus_config.h"
#include "NvDataManagement.h"

//_____D E F I N E __________________________________________________________
// local private defines, enums, macros

#define TAG "OTA Module"

//_____ T Y P E D E F _______________________________________________________
// local private typedefs, structs, unions


//_____ P R O T O T Y P E S _________________________________________________
// local function declarations

//_____ L O C A L  D A T A __________________________________________________
// local private data (static)
static const char *server_root_ca_public_key_pem = OTA_SERVER_ROOT_CA_PEM;
static const char *peer_public_key_pem = OTA_PEER_PEM;

// Static because the scope of this object is the usage of the iap_https module.
static iap_https_config_t ota_config;

//_____ P U B L I C   F U N C T I O N S _____________________________________
// public interface functions
void OTA_Init(void)
{
	uint8_t operatingEnv = OPERATION_DEV_ENVIRONMENT;

	ESP_LOGI(TAG, "Initialising OTA firmware updating.");

	operatingEnv = NvDM_GetOperatingEnvironment();

	if(operatingEnv == OPERATION_STAGE_ENVIRONMENT)
	{
		ota_config.server_host_name = STAGE_OTA_SERVER_HOST_NAME;
	}
	else if(operatingEnv == OPERATION_PROD_ENVIRONMENT)
	{
		ota_config.server_host_name = PROD_OTA_SERVER_HOST_NAME;
	}
	else //if(operatingEnv == OPERATION_DEV_ENVIRONMENT)
	{
		ota_config.server_host_name = DEV_OTA_SERVER_HOST_NAME;
	}

	ota_config.current_software_version = OTA_SOFTWARE_VERSION;

	if(operatingEnv != CURRENT_OPERATING_ENVIRONMENT)
	{
		ota_config.current_software_version = 0;
	}

	ESP_LOGI(TAG, "---------- OTA Software version: %2d -----------", ota_config.current_software_version);
	ESP_LOGI(TAG, "---------- OTA Host Name: %s -----------", ota_config.server_host_name);

	ota_config.server_port = "443";
	strncpy(ota_config.server_metadata_path, OTA_SERVER_METADATA_PATH, sizeof(ota_config.server_metadata_path) / sizeof(char));
	bzero(ota_config.server_firmware_path, sizeof(ota_config.server_firmware_path) / sizeof(char));
	ota_config.server_root_ca_public_key_pem = server_root_ca_public_key_pem;
	ota_config.peer_public_key_pem = peer_public_key_pem;
	ota_config.polling_interval_s = OTA_POLLING_INTERVAL_S;
	ota_config.auto_reboot = OTA_AUTO_REBOOT;

	iap_https_init(&ota_config);

	// Immediately check if there's a new firmware image available.
	iap_https_check_now();
}

void OTA_UpdateConfig(void)
{
	uint8_t operatingEnv = OPERATION_DEV_ENVIRONMENT;

	ESP_LOGI(TAG, "OTA Configuration updating.");

	operatingEnv = NvDM_GetOperatingEnvironment();

	if(operatingEnv == OPERATION_STAGE_ENVIRONMENT)
	{
		ota_config.server_host_name = STAGE_OTA_SERVER_HOST_NAME;
	}
	else if(operatingEnv == OPERATION_PROD_ENVIRONMENT)
	{
		ota_config.server_host_name = PROD_OTA_SERVER_HOST_NAME;
	}
	else //if(operatingEnv == PURA_OPERATION_DEV_ENVIRONMENT)
	{
		ota_config.server_host_name = DEV_OTA_SERVER_HOST_NAME;
	}

	ota_config.current_software_version = OTA_SOFTWARE_VERSION;
	if(operatingEnv != CURRENT_OPERATING_ENVIRONMENT)
	{
		ota_config.current_software_version = 0;
	}

	ESP_LOGI(TAG, "---------- OTA Software version: %2d -----------", ota_config.current_software_version);
	ESP_LOGI(TAG, "---------- OTA Host Name: %s -----------", ota_config.server_host_name);

	iap_https_update_host_name(&ota_config);
}
void OTA_CheckForUpdate(void)
{
	//iap_https_init(&ota_config);

	//Check if there's a new firmware image available.
	iap_https_check_now();
}
void OTA_10msTask(void)
{
	if (iap_https_update_in_progress())
	{

	}
}

 //_____ L O C A L  F U N C T I O N S ________________________________________
 // local interface functions
