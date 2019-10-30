/**
@file			  WiFiDataManagement.c
@author			  JGS
@brief			  WiFi Data Management module related functions definitions

@copyright		  PSDI

@attention		  The information contained herein is confidential property of
                  PSDI. The use, copy, transfer or disclosure
                  of such information is prohibited except by express written
                  agreement with Directed Electronics Inc.
*/


//_____I N C L U D E ________________________________________________________
// Place here all includes needed by this file starting with its own header
// showing the reason of the include.
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
#include "bt.h"

#include "esp_blufi_api.h"
#include "esp_bt_defs.h"
#include "esp_gap_ble_api.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "BluetoothDataManagement.h"
#include "WiFiDataManagement.h"

#include "nvs_flash.h"
#include "nvs.h"

#include "LedControl.h"

//_____D E F I N E __________________________________________________________
// local private defines, enums, macros
#define WIFI_LIST_NUM   10


//_____ T Y P E D E F _______________________________________________________
// local private typedefs, structs, unions


//_____ P R O T O T Y P E S _________________________________________________
// local function declarations

//_____ L O C A L  D A T A __________________________________________________
// local private data (static)
wifi_config_t sta_config;
wifi_config_t ap_config;

/* FreeRTOS event group to signal when we are connected & ready to make a request */
EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;



/* store the station info for send back to phone */
bool gl_sta_connected = false;
uint8_t gl_sta_bssid[6];
uint8_t gl_sta_ssid[32];
int gl_sta_ssid_len;

uint8_t BlufiNativeScanCallReceived = FALSE;
uint8_t WiFiScanDone = FALSE;
//extern const char *TAG;


#define STORAGE_NAMESPACE "storage"

#define TAG "WiFiDM"

//_____ P U B L I C   F U N C T I O N S _____________________________________
// public interface functions

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event);
static uint8 mac_address[6];
static char mac_address_string[13];

/**
  @brief            WifiDm_Init
  @details          Initialize WiFi Data Management module
  @param[in]        None
  @param[out]       None
  @param[in,out]    None
  @retval  type     None
*/
 void WifiDm_Init(void)
 {

	 nvs_handle my_handle;
	 esp_err_t err;
	 gl_sta_connected = false;

     err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);

	 tcpip_adapter_init();
	 wifi_event_group = xEventGroupCreate();
	 ESP_ERROR_CHECK( esp_event_loop_init(wifi_event_handler, NULL) );
	 wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	 ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
	 ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
	 ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
	 ESP_ERROR_CHECK( esp_wifi_start() );

	 esp_wifi_get_mac(WIFI_IF_STA, mac_address);
	 sprintf(mac_address_string,"%02X%02X%02X%02X%02X%02X",mac_address[0], mac_address[1], mac_address[2],mac_address[3],mac_address[4],mac_address[5]);
	 ESP_LOGI(TAG,"MAC Address:%s", mac_address_string);


	 // Close
	 nvs_close(my_handle);

	 WiFiDm_Connect();
 }

 uint8 WiFiDm_Connect(void)
 {
	 nvs_handle my_handle;
	 esp_err_t err;
	 uint8 retval = 0;

	 size_t ssid_required_size = 0;
	 size_t password_required_size = 0;

	 gl_sta_connected = false;
	  // Open
	 err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);

	 wifi_config_t wifi_config = {
			.sta = {
				.ssid = "",
				.password = "",
			},
		};

	 // Read ssid blob

	 // obtain required memory space to store blob being read from NVS
	 err = nvs_get_blob(my_handle, "wifi_ssid", NULL, &ssid_required_size);



	 // obtain required memory space to store blob being read from NVS
	 err = nvs_get_blob(my_handle, "wifi_password", NULL, &password_required_size);
	 //ESP_LOGI(TAG,"password:\n");

	 if (password_required_size == 0)
	 {
		 ESP_LOGI(TAG,"password: Nothing saved yet!\n");
	 }
	 else
	 {
		 err = nvs_get_blob(my_handle, "wifi_password", wifi_config.sta.password, &password_required_size);

		 //ESP_LOGI(TAG,"password: %s\n", wifi_config.sta.password);
	 }

	 if (ssid_required_size == 0)
	 {
		 ESP_LOGI(TAG,"ssid: Nothing saved yet!\n");
	 }
	 else
	 {
		 err = nvs_get_blob(my_handle, "wifi_ssid", wifi_config.sta.ssid, &ssid_required_size);

		 ESP_LOGI(TAG,"ssid: %s", wifi_config.sta.ssid);

		 ESP_LOGI("example", "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
		 ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
		 ESP_ERROR_CHECK(esp_wifi_set_auto_connect(true));
		 ESP_ERROR_CHECK(esp_wifi_connect());

	 }


	 // Close
	 nvs_close(my_handle);

	 if(ssid_required_size == 0)
	 {
		 retval = 0;
	 }
	 else
	 {
		 retval = 1;
	 }


	 return retval;
 }

 void WiFiDm_Reconnect(void)
 {
	 esp_wifi_disconnect();
	 vTaskDelay(100 / portTICK_PERIOD_MS);
	 esp_wifi_connect();
 }

 void WiFiDm_Disconnect(void)
 {
	 esp_wifi_disconnect();
	 esp_wifi_deauth_sta(0);
	 esp_wifi_set_auto_connect(false);
 }


 uint8 WiFiDm_IsCredentialsPresent(void)
 {
	 uint8 retval = 0;
	 nvs_handle my_handle;
	 esp_err_t err;

	 size_t ssid_required_size = 0;
	 size_t password_required_size = 0;

	 err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);

	 err = nvs_get_blob(my_handle, "wifi_ssid", NULL, &ssid_required_size);
	 err = nvs_get_blob(my_handle, "wifi_password", NULL, &password_required_size);

	 nvs_close(my_handle);

	 if(password_required_size == 0 || ssid_required_size == 0 || password_required_size > 32 || ssid_required_size > 32 )
	 {
		 retval = 0;
		 if (password_required_size > 32 || ssid_required_size > 32)
		 {
			 WiFiDm_ClearCredentials();
		 }
	 }
	 else
	 {
		 retval = 1;
	 }

	 return retval;
 }

 void WiFiDm_ClearCredentials(void)
 {
	 nvs_handle my_handle;

	 //Delete WiFi SSID and Password Config
	 sta_config.sta.ssid[0] = '\0';
	 sta_config.sta.password[0] = '\0';
	 esp_wifi_set_config(WIFI_IF_STA, &sta_config);

	 //Erase WiFi SSID and Password keys from memory
	 nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);

	 nvs_erase_key(my_handle, "wifi_ssid");
	 nvs_erase_key(my_handle, "wifi_password");

	 nvs_commit(my_handle);
	 nvs_close(my_handle);
 }

 void WiFiDm_ScanStart(uint8_t NativeScanFlag)
 {
	 wifi_scan_config_t scanConf = {
			.ssid = NULL,
			.bssid = NULL,
			.channel = 0,
			.show_hidden = false,
			.scan_type = WIFI_SCAN_TYPE_ACTIVE,
			.scan_time.active.min = 100,
			.scan_time.active.max = 500,
		};

	 if(NativeScanFlag == true)
	 {
		BlufiNativeScanCallReceived = true;
		WiFiScanDone = FALSE;
		ESP_ERROR_CHECK(esp_wifi_scan_start(&scanConf, true));
	 }
	 else
	 {
		BlufiNativeScanCallReceived = false;
		WiFiScanDone = FALSE;
		ESP_ERROR_CHECK(esp_wifi_scan_start(&scanConf,FALSE));
	 }
 }

 uint8_t WiFiDm_GetScanStatus(void)
 {
	 return WiFiScanDone;
 }
 void WiFiDm_ScanStop(void)
 {
	 WiFiScanDone = FALSE;
	 esp_wifi_scan_stop();
 }

 uint8 WifiDm_GetConnectionStatus(void)
 {
 	return gl_sta_connected;
 }

void WifiDm_GetMac(char * data)
{
	memcpy(data, mac_address_string, 12);
}


void WifiDm_GetMacRaw(uint8_t * data)
{
	memcpy(data, mac_address, 6);
}

 //_____ L O C A L  F U N C T I O N S ________________________________________
 // local interface functions

 /**
   @brief            wifi_event_handler
   @details          Event Handler for WiFi Events
   @param[in]        *ctx - Pointer to Context
   	   	   	   	   	 *event - Pointer to Event
   @param[out]       None
   @param[in,out]    None
   @retval  type     ESP_OK
 */
 static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
 {
     wifi_mode_t mode;

     switch (event->event_id) {
     case SYSTEM_EVENT_STA_START:
         esp_wifi_connect();
         break;
     case SYSTEM_EVENT_STA_GOT_IP: {
         esp_blufi_extra_info_t info;
         gl_sta_connected = true;
         xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
         esp_wifi_get_mode(&mode);

         memset(&info, 0, sizeof(esp_blufi_extra_info_t));
         memcpy(info.sta_bssid, gl_sta_bssid, 6);
         info.sta_bssid_set = true;
         info.sta_ssid = gl_sta_ssid;
         info.sta_ssid_len = gl_sta_ssid_len;
         esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_SUCCESS, 0, &info);
         break;
     }
     case SYSTEM_EVENT_STA_LOST_IP:
    	 //if(SystemIsControlStateNormal() == TRUE)
    	 {
    		 esp_wifi_disconnect();
    		 esp_wifi_connect();
    	 }
    	 break;
     case SYSTEM_EVENT_STA_CONNECTED:

         memcpy(gl_sta_bssid, event->event_info.connected.bssid, 6);
         memcpy(gl_sta_ssid, event->event_info.connected.ssid, event->event_info.connected.ssid_len);
         gl_sta_ssid_len = event->event_info.connected.ssid_len;

         break;
     case SYSTEM_EVENT_STA_DISCONNECTED:
         /* This is a workaround as ESP32 WiFi libs don't currently
            auto-reassociate. */
         gl_sta_connected = false;
         memset(gl_sta_ssid, 0, 32);
         memset(gl_sta_bssid, 0, 6);
         gl_sta_ssid_len = 0;
         //if(SystemIsControlStateNormal() == TRUE)
         {
        	 esp_wifi_connect();
         }
         xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
         break;
     case SYSTEM_EVENT_AP_START:
         esp_wifi_get_mode(&mode);

         /* TODO: get config or information of softap, then set to report extra_info */
         if (gl_sta_connected) {
             esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_SUCCESS, 0, NULL);
         } else {
             esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_FAIL, 0, NULL);
         }
         break;
     case SYSTEM_EVENT_SCAN_DONE: {
    	 if(BlufiNativeScanCallReceived == true)
    	 {
			 uint16_t apCount = 0;
			 esp_wifi_scan_get_ap_num(&apCount);
			 if (apCount == 0) {
				 BLUFI_INFO("Nothing AP found");
				 break;
			 }
			 wifi_ap_record_t *ap_list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * apCount);
			 if (!ap_list) {
				 BLUFI_ERROR("malloc error, ap_list is NULL");
				 break;
			 }
			 ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&apCount, ap_list));
			 esp_blufi_ap_record_t * blufi_ap_list = (esp_blufi_ap_record_t *)malloc(apCount * sizeof(esp_blufi_ap_record_t));
			 if (!blufi_ap_list) {
				 if (ap_list) {
					 free(ap_list);
				 }
				 BLUFI_ERROR("malloc error, blufi_ap_list is NULL");
				 break;
			 }
			 for (int i = 0; i < apCount; ++i)
			 {
				 blufi_ap_list[i].rssi = ap_list[i].rssi;
				 memcpy(blufi_ap_list[i].ssid, ap_list[i].ssid, sizeof(ap_list[i].ssid));
			 }
			 esp_blufi_send_wifi_list(apCount, blufi_ap_list);
			 esp_wifi_scan_stop();
			 free(ap_list);
			 free(blufi_ap_list);
    	 }
    	 else
    	 {
    		 WiFiScanDone = TRUE;
    	 }

		 break;
	 }
     default:
         break;
     }
     return ESP_OK;
 }
