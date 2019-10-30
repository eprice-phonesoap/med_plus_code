/**
@file			  BluetoothDataManagement.c
@author			  JGS
@brief			  Bluetooth Data Management module related functions definitions

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
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_bt.h"

#include "esp_blufi_api.h"
#include "esp_bt_defs.h"
#include "esp_gap_ble_api.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "BluetoothDataManagement.h"
#include "LedControl.h"

#include "NvDataManagement.h"

#include "mbedtls/aes.h"
#include "mbedtls/dhm.h"
#include "mbedtls/md5.h"
#include "rom/crc.h"
#include "WiFiDataManagement.h"
#include "AnalogSensorManagement.h"

#include "main.h"

#include "OTAManagement.h"
#include "esp_log.h"

//_____D E F I N E __________________________________________________________
// local private defines, enums, macros

static const char* TAG = "BTDM";

#define BLUFI_DEVICE_NAME            "Med+"

#define STORAGE_NAMESPACE "storage"

/*
   The SEC_TYPE_xxx is for self-defined packet data type in the procedure of "BLUFI negotiate key"
   If user use other negotiation procedure to exchange(or generate) key, should redefine the type by yourself.
 */
#define SEC_TYPE_DH_PARAM_LEN   0x00
#define SEC_TYPE_DH_PARAM_DATA  0x01
#define SEC_TYPE_DH_P           0x02
#define SEC_TYPE_DH_G           0x03
#define SEC_TYPE_DH_PUBLIC      0x04


//_____ T Y P E D E F _______________________________________________________
// local private typedefs, structs, unions

struct blufi_security {
#define DH_SELF_PUB_KEY_LEN     128
#define DH_SELF_PUB_KEY_BIT_LEN (DH_SELF_PUB_KEY_LEN * 8)
    uint8_t  self_public_key[DH_SELF_PUB_KEY_LEN];
#define SHARE_KEY_LEN           128
#define SHARE_KEY_BIT_LEN       (SHARE_KEY_LEN * 8)
    uint8_t  share_key[SHARE_KEY_LEN];
    size_t   share_len;
#define PSK_LEN                 16
    uint8_t  psk[PSK_LEN];
    uint8_t  *dh_param;
    int      dh_param_len;
    uint8_t  iv[16];
    mbedtls_dhm_context dhm;
    mbedtls_aes_context aes;
};
static struct blufi_security *blufi_sec;


//_____ P R O T O T Y P E S _________________________________________________
// local function declarations
static void blufi_event_callback(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param);
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static void custom_data_handler(esp_blufi_cb_param_t *param);

//_____ L O C A L  D A T A __________________________________________________
// local private data (static)

char blufi_device_name[4+5+1]= {'M','e','d','P','-','0','0','0','0',0};
extern wifi_config_t sta_config;
extern wifi_config_t ap_config;

static uint8_t blufi_service_uuid128[32] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    //first uuid, 16bit, [12],[13] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00,
};

//static uint8_t test_manufacturer[TEST_MANUFACTURER_DATA_LEN] =  {0x12, 0x23, 0x45, 0x56};
static esp_ble_adv_data_t blufi_adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x100,
    .max_interval = 0x100,
    .appearance = 0x00,
    .manufacturer_len = 0,
    .p_manufacturer_data =  NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = 16,
    .p_service_uuid = blufi_service_uuid128,
    .flag = 0x6,
};

static esp_ble_adv_params_t blufi_adv_params = {
    .adv_int_min        = 0x100,
    .adv_int_max        = 0x100,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    //.peer_addr            =
    //.peer_addr_type       =
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};


static esp_blufi_callbacks_t blufi_callbacks = {
    .event_cb = blufi_event_callback,
    .negotiate_data_handler = blufi_dh_negotiate_data_handler,
    .encrypt_func = blufi_aes_encrypt,
    .decrypt_func = blufi_aes_decrypt,
    .checksum_func = blufi_crc_checksum,
};

extern bool gl_sta_connected;
extern uint8_t gl_sta_bssid[6];
extern uint8_t gl_sta_ssid[32];
extern int gl_sta_ssid_len;
char mac_String[13];
char mqtt_status_topic[100]={0};


//_____ P U B L I C   F U N C T I O N S _____________________________________
// public interface functions
esp_err_t save_password(uint8 * password, uint8 length);
esp_err_t save_ssid(uint8 * ssid, uint8 length);
uint8 ble_connect = false;
/**
  @brief            BtDm_Init
  @details          Initialize Bluetooth Data Management module
  @param[in]        None
  @param[out]       None
  @param[in,out]    None
  @retval  type     None
*/

 void BtDm_Init(void)
 {
	 esp_err_t ret;


	 esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

	 ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));


	 ret = esp_bt_controller_init(&bt_cfg);
	if (ret) {
		BLUFI_ERROR("%s initialize bt controller failed\n", __func__);
	}

	ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
	if (ret) {
		BLUFI_ERROR("%s enable bt controller failed\n", __func__);
		return;
	}
	esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);
	ret = esp_bluedroid_init();
	if (ret) {
		BLUFI_ERROR("%s init bluedroid failed\n", __func__);
		return;
	}

	ret = esp_bluedroid_enable();
	if (ret) {
		BLUFI_ERROR("%s init bluedroid failed\n", __func__);
		return;
	}
	BLUFI_INFO("BD ADDR: "ESP_BD_ADDR_STR"\n", ESP_BD_ADDR_HEX(esp_bt_dev_get_address()));

	BLUFI_INFO("BLUFI VERSION %04x\n", esp_blufi_get_version());


	blufi_security_init();

	esp_ble_gap_register_callback(gap_event_handler);

	//esp_ble_gap_set_scan_params(&ble_scan_params);


	esp_blufi_register_callbacks(&blufi_callbacks);
	esp_blufi_profile_init();

	ble_connect = false;

	WifiDm_GetMac(mac_String);
	blufi_device_name[5] = mac_String[8];
	blufi_device_name[6] = mac_String[9];
	blufi_device_name[7] = mac_String[10];
	blufi_device_name[8] = mac_String[11];
	esp_ble_gap_set_device_name(blufi_device_name);
	ESP_ERROR_CHECK(tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, blufi_device_name));
	ESP_LOGI(TAG,"BT Name:%s",blufi_device_name);
 }

void BtDm_EnableBluetooth(void)
{
	esp_err_t ret;

	ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
	if (ret) {
		BLUFI_ERROR("%s enable bt controller failed\n", __func__);
		return;
	}
	ret = esp_bluedroid_init();
	if (ret) {
		BLUFI_ERROR("%s init bluedroid failed\n", __func__);
		return;
	}

	ret = esp_bluedroid_enable();
	if (ret) {
		BLUFI_ERROR("%s init bluedroid failed\n", __func__);
		return;
	}

	BLUFI_INFO("BD ADDR: "ESP_BD_ADDR_STR"\n", ESP_BD_ADDR_HEX(esp_bt_dev_get_address()));

	BLUFI_INFO("BLUFI VERSION %04x\n", esp_blufi_get_version());


	blufi_security_init();

	esp_ble_gap_register_callback(gap_event_handler);

	//esp_ble_gap_set_scan_params(&ble_scan_params);

	ble_connect = false;

	WifiDm_GetMac(mac_String);
	sprintf(mqtt_status_topic, "Pura/v2/%s/status",mac_String);
	BLUFI_INFO("MQTT Status Message Topic: %s", mqtt_status_topic);
	blufi_device_name[5] = mac_String[8];
	blufi_device_name[6] = mac_String[9];
	blufi_device_name[7] = mac_String[10];
	blufi_device_name[8] = mac_String[11];
	esp_ble_gap_set_device_name(blufi_device_name);
	ESP_LOGI(TAG,"BT Name:%s",blufi_device_name);
}
 //_____ L O C A L  F U N C T I O N S ________________________________________
 // local interface functions


 uint8 blufi_GetConnectionStatus(void)
 {
	 return ble_connect;
 }

 /**
   @brief            blufi_event_callback
   @details          Callback for Blufi Events
   @param[in]        event - Event
   	   	   	   	   	 *param - Event Paramaeters
   @param[out]       None
   @param[in,out]    None
   @retval  type     None
 */
nvs_handle my_handle;
esp_err_t err;

/* connect infor*/
static uint8_t server_if;
static uint16_t conn_id;


static void blufi_event_callback(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param)
{
    /* actually, should post to blufi_task handle the procedure,
     * now, as a demo, we do simplely */
    switch (event) {

    case ESP_BLUFI_EVENT_INIT_FINISH:
        BLUFI_INFO("BLUFI init finish\n");
        Set_Blufi_Adv_Data(MFG_DATA_NOT_ONBOARDED);
        break;
    case ESP_BLUFI_EVENT_DEINIT_FINISH:
        BLUFI_INFO("BLUFI De-init finish\n");
        break;
    case ESP_BLUFI_EVENT_BLE_CONNECT:
        BLUFI_INFO("BLUFI ble connect\n");
        server_if = param->connect.server_if;
        conn_id = param->connect.conn_id;
        esp_ble_gap_stop_advertising();

        blufi_security_init();
        ble_connect = true;
        //mqtt_publish_events(MQTT_BT_CONNECTED, NULL, 0);
        break;
    case ESP_BLUFI_EVENT_BLE_DISCONNECT:
        BLUFI_INFO("BLUFI ble disconnect\n");
        blufi_security_deinit();
        esp_ble_gap_start_advertising(&blufi_adv_params);
        ble_connect = false;
        //mqtt_publish_events(MQTT_BT_DISCONNECTED, NULL, 0);
        break;
    case ESP_BLUFI_EVENT_SET_WIFI_OPMODE:
        BLUFI_INFO("BLUFI Set WIFI opmode %d\n", param->wifi_mode.op_mode);
        ESP_ERROR_CHECK( esp_wifi_set_mode(param->wifi_mode.op_mode) );
        break;
    case ESP_BLUFI_EVENT_REQ_CONNECT_TO_AP:
        BLUFI_INFO("BLUFI requset wifi connect to AP\n");
        /* there is no wifi callback when the device has already connected to this wifi
           so disconnect wifi before connection.
        */
        esp_wifi_disconnect();
        esp_wifi_connect();
        break;
    case ESP_BLUFI_EVENT_REQ_DISCONNECT_FROM_AP:
        BLUFI_INFO("BLUFI requset wifi disconnect from AP\n");
        esp_wifi_disconnect();
       break;
   case ESP_BLUFI_EVENT_REPORT_ERROR:
       BLUFI_ERROR("BLUFI report error, error code %d\n", param->report_error.state);
       esp_blufi_send_error_info(param->report_error.state);
        break;
    case ESP_BLUFI_EVENT_GET_WIFI_STATUS: {
        wifi_mode_t mode;
        esp_blufi_extra_info_t info;

       esp_wifi_get_mode(&mode);

       if (gl_sta_connected ) {
           memset(&info, 0, sizeof(esp_blufi_extra_info_t));
           memcpy(info.sta_bssid, gl_sta_bssid, 6);
           info.sta_bssid_set = true;
           info.sta_ssid = gl_sta_ssid;
           info.sta_ssid_len = gl_sta_ssid_len;
           esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_SUCCESS, 0, &info);
       } else {
           esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_FAIL, 0, NULL);
       }
       BLUFI_INFO("BLUFI get wifi status from AP\n");

       break;
   }
   case ESP_BLUFI_EVENT_RECV_SLAVE_DISCONNECT_BLE:
       BLUFI_INFO("blufi close a gatt connection");
       esp_blufi_close(server_if, conn_id);
       break;


    case ESP_BLUFI_EVENT_DEAUTHENTICATE_STA:
        /* TODO */
        break;
	case ESP_BLUFI_EVENT_RECV_STA_BSSID:
        memcpy(sta_config.sta.bssid, param->sta_bssid.bssid, 6);
        sta_config.sta.bssid_set = 1;
        esp_wifi_set_config(WIFI_IF_STA, &sta_config);
        BLUFI_INFO("Recv STA BSSID %s\n", sta_config.sta.ssid);
        break;
	case ESP_BLUFI_EVENT_RECV_STA_SSID:
        strncpy((char *)sta_config.sta.ssid, (char *)param->sta_ssid.ssid, param->sta_ssid.ssid_len);
        if(param->sta_ssid.ssid_len <= 32) //Limit SSID Length to 32 chars
        {
        	sta_config.sta.ssid[param->sta_ssid.ssid_len] = '\0';
        	esp_wifi_set_config(WIFI_IF_STA, &sta_config);
        	BLUFI_INFO("Recv STA SSID %s\n", sta_config.sta.ssid);
        	nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
        	nvs_set_blob(my_handle, "wifi_ssid", (uint8 *)param->sta_ssid.ssid, param->sta_ssid.ssid_len);
        	nvs_commit(my_handle);
        	nvs_close(my_handle);
        }
        else
        {
        	BLUFI_INFO("Recv STA SSID with unsupported length %d\n", param->sta_ssid.ssid_len);
        }
        break;
	case ESP_BLUFI_EVENT_RECV_STA_PASSWD:
        strncpy((char *)sta_config.sta.password, (char *)param->sta_passwd.passwd, param->sta_passwd.passwd_len);
        if(param->sta_passwd.passwd_len <= 32)
        {
			sta_config.sta.password[param->sta_passwd.passwd_len] = '\0';
			esp_wifi_set_config(WIFI_IF_STA, &sta_config);
			BLUFI_INFO("Recv STA PASSWORD %s\n. WiFi will be disconnected", sta_config.sta.password);
			nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
			nvs_set_blob(my_handle, "wifi_password", (uint8 *)param->sta_passwd.passwd, param->sta_passwd.passwd_len);
			nvs_commit(my_handle);
			nvs_close(my_handle);
			esp_wifi_disconnect();
        }
        else
        {
        	BLUFI_INFO("Recv STA PASSWORD with unsupported length %d\n", param->sta_passwd.passwd_len);
        }
        break;
	case ESP_BLUFI_EVENT_RECV_SOFTAP_SSID:
        strncpy((char *)ap_config.ap.ssid, (char *)param->softap_ssid.ssid, param->softap_ssid.ssid_len);
       ap_config.ap.ssid[param->softap_ssid.ssid_len] = '\0';
        ap_config.ap.ssid_len = param->softap_ssid.ssid_len;
        esp_wifi_set_config(WIFI_IF_AP, &ap_config);
        BLUFI_INFO("Recv SOFTAP SSID %s, ssid len %d\n", ap_config.ap.ssid, ap_config.ap.ssid_len);
        break;
	case ESP_BLUFI_EVENT_RECV_SOFTAP_PASSWD:
        strncpy((char *)ap_config.ap.password, (char *)param->softap_passwd.passwd, param->softap_passwd.passwd_len);
       ap_config.ap.password[param->softap_passwd.passwd_len] = '\0';
        esp_wifi_set_config(WIFI_IF_AP, &ap_config);
       BLUFI_INFO("Recv SOFTAP PASSWORD %s len = %d\n", ap_config.ap.password, param->softap_passwd.passwd_len);
        break;
	case ESP_BLUFI_EVENT_RECV_SOFTAP_MAX_CONN_NUM:
        if (param->softap_max_conn_num.max_conn_num > 4) {
            return;
        }
        ap_config.ap.max_connection = param->softap_max_conn_num.max_conn_num;
        esp_wifi_set_config(WIFI_IF_AP, &ap_config);
        BLUFI_INFO("Recv SOFTAP MAX CONN NUM %d\n", ap_config.ap.max_connection);
        break;
	case ESP_BLUFI_EVENT_RECV_SOFTAP_AUTH_MODE:
        if (param->softap_auth_mode.auth_mode >= WIFI_AUTH_MAX) {
            return;
        }
        ap_config.ap.authmode = param->softap_auth_mode.auth_mode;
        esp_wifi_set_config(WIFI_IF_AP, &ap_config);
        BLUFI_INFO("Recv SOFTAP AUTH MODE %d\n", ap_config.ap.authmode);
        break;
	case ESP_BLUFI_EVENT_RECV_SOFTAP_CHANNEL:
        if (param->softap_channel.channel > 13) {
            return;
        }
        ap_config.ap.channel = param->softap_channel.channel;
        esp_wifi_set_config(WIFI_IF_AP, &ap_config);
        BLUFI_INFO("Recv SOFTAP CHANNEL %d\n", ap_config.ap.channel);
        break;
   	case ESP_BLUFI_EVENT_GET_WIFI_LIST:{

       WiFiDm_ScanStart(true);
       break;
  	 }
	case ESP_BLUFI_EVENT_RECV_CUSTOM_DATA:
   	 BLUFI_INFO("Custom Data Event received\n");
        custom_data_handler(param);
        break;

	case ESP_BLUFI_EVENT_RECV_USERNAME:
        /* Not handle currently */
        break;
	case ESP_BLUFI_EVENT_RECV_CA_CERT:
        /* Not handle currently */
        break;
	case ESP_BLUFI_EVENT_RECV_CLIENT_CERT:
        /* Not handle currently */
        break;
	case ESP_BLUFI_EVENT_RECV_SERVER_CERT:
        /* Not handle currently */
        break;
	case ESP_BLUFI_EVENT_RECV_CLIENT_PRIV_KEY:
        /* Not handle currently */
        break;;
	case ESP_BLUFI_EVENT_RECV_SERVER_PRIV_KEY:
        /* Not handle currently */
        break;
    default:
        break;
    }
}

 /**
   @brief            gap_event_handler
   @details          Callback for Bluetooth GAP Events
   @param[in]        event - Event
   	   	   	   	   	 *param - Event Paramaeters
   @param[out]       None
   @param[in,out]    None
   @retval  type     None
 */
 static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
 {
	 uint32_t duration = 0;
	 //esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;

     switch (event) {
     case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
		 //the unit of the duration is second, 0 means scan permanently

		 esp_ble_gap_start_scanning(duration);
		 break;
	  case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
		 //scan start complete event to indicate scan start successfully or failed
		 if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
			 ESP_LOGE(TAG, "Scan start failed");
		 }
		 break;
	 case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
		 //adv start complete event to indicate adv start successfully or failed
		 if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
			 ESP_LOGE(TAG, "Adv start failed");
		 }
		 break;
	 case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
		 if (param->scan_stop_cmpl.status != ESP_BT_STATUS_SUCCESS){
			 ESP_LOGE(TAG, "Scan stop failed");
		 }
		 else {
			 ESP_LOGI(TAG, "Stop scan successfully");
		 }
		 break;

	 case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
		 if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS){
			 ESP_LOGE(TAG, "Adv stop failed");
		 }
		 else {
			 ESP_LOGI(TAG, "Stop adv successfully");
		 }
		 break;
     case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
         esp_ble_gap_start_advertising(&blufi_adv_params);
         break;
     default:
         break;
     }
 }



 static int myrand( void *rng_state, unsigned char *output, size_t len )
 {
     size_t i;

     for( i = 0; i < len; ++i )
         output[i] = esp_random();

     return( 0 );
 }

 void blufi_dh_negotiate_data_handler(uint8_t *data, int len, uint8_t **output_data, int *output_len, bool *need_free)
 {
     int ret;
     uint8_t type = data[0];

     if (blufi_sec == NULL) {
         BLUFI_ERROR("BLUFI Security is not initialized");
         return;
     }

     switch (type) {
     case SEC_TYPE_DH_PARAM_LEN:
         blufi_sec->dh_param_len = ((data[1]<<8)|data[2]);
         if (blufi_sec->dh_param) {
             free(blufi_sec->dh_param);
         }
         blufi_sec->dh_param = (uint8_t *)malloc(blufi_sec->dh_param_len);
         if (blufi_sec->dh_param == NULL) {
             return;
         }
         break;
     case SEC_TYPE_DH_PARAM_DATA:

         memcpy(blufi_sec->dh_param, &data[1], blufi_sec->dh_param_len);

         ret = mbedtls_dhm_read_params(&blufi_sec->dhm, &blufi_sec->dh_param, &blufi_sec->dh_param[blufi_sec->dh_param_len]);
         if (ret) {
             BLUFI_ERROR("%s read param failed %d\n", __func__, ret);
             return;
         }

         ret = mbedtls_dhm_make_public(&blufi_sec->dhm, (int) mbedtls_mpi_size( &blufi_sec->dhm.P ), blufi_sec->self_public_key, blufi_sec->dhm.len, myrand, NULL);
         if (ret) {
             BLUFI_ERROR("%s make public failed %d\n", __func__, ret);
             return;
         }

         mbedtls_dhm_calc_secret( &blufi_sec->dhm,
                 blufi_sec->share_key,
                 SHARE_KEY_BIT_LEN,
                 &blufi_sec->share_len,
                 NULL, NULL);

         mbedtls_md5(blufi_sec->share_key, blufi_sec->share_len, blufi_sec->psk);

         mbedtls_aes_setkey_enc(&blufi_sec->aes, blufi_sec->psk, 128);
         mbedtls_aes_setkey_dec(&blufi_sec->aes, blufi_sec->psk, 128);

         /* alloc output data */
         *output_data = &blufi_sec->self_public_key[0];
         *output_len = blufi_sec->dhm.len;
         *need_free = false;
         break;
     case SEC_TYPE_DH_P:
         break;
     case SEC_TYPE_DH_G:
         break;
     case SEC_TYPE_DH_PUBLIC:
         break;
     }
 }

 int blufi_aes_encrypt(uint8_t iv8, uint8_t *crypt_data, int crypt_len)
 {
     int ret;
     size_t iv_offset = 0;
     uint8_t iv0[16];

     memcpy(iv0, blufi_sec->iv, sizeof(blufi_sec->iv));
     iv0[0] = iv8;   /* set iv8 as the iv0[0] */

     ret = mbedtls_aes_crypt_cfb128(&blufi_sec->aes, MBEDTLS_AES_ENCRYPT, crypt_len, &iv_offset, iv0, crypt_data, crypt_data);
     if (ret) {
         return -1;
     }

     return crypt_len;
 }

 int blufi_aes_decrypt(uint8_t iv8, uint8_t *crypt_data, int crypt_len)
 {
     int ret;
     size_t iv_offset = 0;
     uint8_t iv0[16];

     memcpy(iv0, blufi_sec->iv, sizeof(blufi_sec->iv));
     iv0[0] = iv8;   /* set iv8 as the iv0[0] */

     ret = mbedtls_aes_crypt_cfb128(&blufi_sec->aes, MBEDTLS_AES_DECRYPT, crypt_len, &iv_offset, iv0, crypt_data, crypt_data);
     if (ret) {
         return -1;
     }

     return crypt_len;
 }

 uint16_t blufi_crc_checksum(uint8_t iv8, uint8_t *data, int len)
 {
     /* This iv8 ignore, not used */
     return crc16_be(0, data, len);
 }

 esp_err_t blufi_security_init(void)
 {
     blufi_sec = (struct blufi_security *)malloc(sizeof(struct blufi_security));
     if (blufi_sec == NULL) {
         return ESP_FAIL;
     }

     memset(blufi_sec, 0x0, sizeof(struct blufi_security));

     mbedtls_dhm_init(&blufi_sec->dhm);
     mbedtls_aes_init(&blufi_sec->aes);

     memset(blufi_sec->iv, 0x0, 16);
     return 0;
 }

 void blufi_security_deinit(void)
 {
     mbedtls_dhm_free(&blufi_sec->dhm);
     mbedtls_aes_free(&blufi_sec->aes);

     memset(blufi_sec, 0x0, sizeof(struct blufi_security));

     free(blufi_sec);
     blufi_sec =  NULL;
 }



 /* Save new run time value in NVS
    by first reading a table of previously saved values
    and then adding the new value at the end of the table.
    Return an error if anything goes wrong
    during this process.
  */
 esp_err_t save_ssid(uint8 * ssid, uint8 length)
 {
     nvs_handle my_handle;
     esp_err_t err;

     // Open
     err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
     if (err != ESP_OK) return err;

     err = nvs_set_blob(my_handle, "wifi_ssid", ssid, length);
     free(ssid);

     if (err != ESP_OK) return err;

     // Commit
     err = nvs_commit(my_handle);
     if (err != ESP_OK) return err;

     // Close
     nvs_close(my_handle);
     return ESP_OK;
 }


 /* Save new run time value in NVS
    by first reading a table of previously saved values
    and then adding the new value at the end of the table.
    Return an error if anything goes wrong
    during this process.
  */
 esp_err_t save_password(uint8 * password, uint8 length)
 {
     nvs_handle my_handle;
     esp_err_t err;

     // Open
     err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
     if (err != ESP_OK) return err;

     err = nvs_set_blob(my_handle, "wifi_password", password, length);
     free(password);

     if (err != ESP_OK) return err;

     // Commit
     err = nvs_commit(my_handle);
     if (err != ESP_OK) return err;

     // Close
     nvs_close(my_handle);
     return ESP_OK;
 }

#define BLUFI_SF_DEVICE_DETAILS 0x00
#define BLUFI_SF_DEVICE_STATUS  0x02
#define BLUFI_SF_WIFI_SCAN	0x05
#define BLUFI_SF_SET_OPERATING_ENVIRONMENT 0x10

uint8_t ScanStopTimer = 0;
uint8_t ScanStopFlag = FALSE;
uint8_t WifiScanTransmitFlag = FALSE;
uint8_t WifiTransmitCounter = 0;
uint16_t DetectedAPs = 0;
static wifi_ap_record_t ap_records[64];
//static uint8_t firebase_UID[28] = {0};
uint8_t claim_status;

#define MQTT_SERVICE_NOT_CONNECTED 0x00
#define MQTT_SERVICE_CONNECTED_ONBOARDING_NOT_VERIFIED 0x01
#define MQTT_SERVICE_CONNECTED_ONBOARDING_VERIFIED 0x02

void custom_data_handler(esp_blufi_cb_param_t *param)
 {
	 uint8_t sub_frame_type = param->custom_data.data[0];
	 uint8_t immediate_reply[20];

	 immediate_reply[0] = sub_frame_type;

	 switch(sub_frame_type)
	 {
	 	 case BLUFI_SF_DEVICE_DETAILS:
	 		 WifiDm_GetMacRaw(&immediate_reply[1]);
	 		 Main_GetSoftwareVersion(&immediate_reply[9]);
	 		 immediate_reply[13] = NvDM_GetOperatingEnvironment();
	 		 esp_blufi_send_custom_data(immediate_reply, 14);
	 		 BLUFI_INFO("Get Device Details custom message received\n");
	 		 break;
	 	 case BLUFI_SF_DEVICE_STATUS:
	 		BLUFI_INFO("Get Device status custom message received\n");
	 		immediate_reply[1] = 0x7F;
	 		immediate_reply[2] = 0x55;
	 		esp_blufi_send_custom_data(immediate_reply, 10);
	 		break;
	 	case BLUFI_SF_WIFI_SCAN:
	 		BLUFI_INFO("WIFI Scan custom message received\n");

			WifiScanTransmitFlag = FALSE;
			DetectedAPs = 0;
			WiFiDm_ScanStart(false);

			immediate_reply[1] = 0x00;
			esp_blufi_send_custom_data(immediate_reply, 2);
			 BLUFI_INFO("WIFI Scan Started\n");
	 		 break;
	 	 case BLUFI_SF_SET_OPERATING_ENVIRONMENT:

			BLUFI_INFO("Set Operating Environment Command Received: %d \n", (uint8_t)param->custom_data.data[1]);
			NvDM_SetOperatingEnvironment((uint8_t)param->custom_data.data[1]);
			OTA_UpdateConfig();
			//SystemSetRestartEvent();

			break;
	 	 default:
	 		 break;
	 }


 }


uint8_t mqtt_data[10];

void BtDm_10msTask(void)
{
	static uint8_t counter = 0;
	static uint8_t secCounter = 0;
	//static uint8_t MinCounter = 115;

	uint8_t transmit_data[30] = {0};
	uint8_t iLoop=0;

	counter++;
	if(counter >= 5)
	{

		counter = 0;
		secCounter++;
		if(secCounter >=20)
		{
			secCounter=0;

			//ESP_LOGI(TAG,"Free Heap Size : %d",esp_get_free_heap_size());
			if( WiFiDm_GetScanStatus() == TRUE)
			{
				WiFiDm_ScanStop();
				ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&DetectedAPs));
				 if (DetectedAPs == 0) {
					 BLUFI_INFO("Nothing AP found");

				 }
				 else
				 {
					 if(DetectedAPs > 64)
					 {
						DetectedAPs = 64;
						BLUFI_INFO("Concat Received APs to 64");
					 }
					 ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&DetectedAPs,ap_records ));
					WifiTransmitCounter = 0;
					WifiScanTransmitFlag = TRUE;
				 }
			}
		}

		if(WifiScanTransmitFlag == TRUE)
		{
			if(WifiTransmitCounter > DetectedAPs)
			{
				WifiScanTransmitFlag = FALSE;
				transmit_data[0] = 0x05;
				transmit_data[1] = 0x02;
				esp_blufi_send_custom_data(transmit_data, 2);
				BLUFI_INFO("Transmitting Complete\n");
			}
			else
			{
				transmit_data[0] = 0x05;
				transmit_data[1] = 0x01;
				if(ap_records[WifiTransmitCounter].authmode != WIFI_AUTH_OPEN)
				{
					transmit_data[2] = (1 << 7);
				}
				transmit_data[2] = transmit_data[2] | (ap_records[WifiTransmitCounter].rssi & 0x7F);
				for(iLoop=0;iLoop<=19;iLoop++)
				{
					transmit_data[3+iLoop] = ap_records[WifiTransmitCounter].ssid[0+iLoop];
					if(transmit_data[3+iLoop] == '\n')
					{
						break;
					}
				}

				esp_blufi_send_custom_data(transmit_data, 23);
				//vTaskDelay(100 / portTICK_PERIOD_MS);
				BLUFI_INFO("Transmitting AP Info: %s, %d, %d\n",ap_records[WifiTransmitCounter].ssid,ap_records[WifiTransmitCounter].authmode, ap_records[WifiTransmitCounter].rssi);
			}
			WifiTransmitCounter++;
		}
	}

}

static uint8_t onboard_status_temp[10] = {0};

void Set_Blufi_Adv_Data(uint8_t onboard_status)
{
	onboard_status_temp[0] = onboard_status;
	blufi_adv_data.manufacturer_len = 2;
	blufi_adv_data.p_manufacturer_data = (uint8_t *)onboard_status_temp;
	esp_ble_gap_config_adv_data(&blufi_adv_data);
}
