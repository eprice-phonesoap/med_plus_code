/**
@file			  iap_https.h
@author			  Jey
@brief			  Definition for use in iap_https.c

@copyright		  Pura Scents

@attention		  The information contained herein is confidential property of
                  Pura. The use, copy, transfer or disclosure
                  of such information is prohibited except by express written
                  agreement with Pura.
*/

#ifndef IAP_HTTPS_H_
#define IAP_HTTPS_H_

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

#include "Platform_Types.h"
#include "Std_Types.h"

//_____D E F I N E __________________________________________________________
// public defines, enums and macros

//_____ T Y P E D E F _______________________________________________________
// public typedefs, structs, unions
typedef struct iap_https_config_ {

    // Version number of the running firmware image.
    int current_software_version;

    // Name of the host that provides the firmware images, e.g. "www.classycode.io".
    const char *server_host_name;

    // TCP port for TLS communication, e.g. "443".
    const char *server_port;

    // Public key of the server's root CA certificate.
    // Needs to be in PEM format (base64-encoded DER data with begin and end marker).
    const char *server_root_ca_public_key_pem;

    // Public key of the server's peer certificate (for certificate pinning).
    // Needs to be in PEM format (base64-encoded DER data with begin and end marker).
    const char *peer_public_key_pem;

    // Path to the metadata file which contains information on the firmware image,
    // e.g. /ota/meta.txt. We perform an HTTP/1.1 GET request on this file.
    char server_metadata_path[256];

    // Path to the firmware image file.
    char server_firmware_path[256];

    // Default time between two checks, in seconds.
    // If you want to trigger the check manually, set the value to 0 and call the
    // iap_https_check_now function.
    // During development, this is typically a small value, e.g. 10 seconds.
    // In production, especially with many devices, higher values make more sense
    // to keep the network traffic low (e.g. 3600 for 1 hour).
    uint32_t polling_interval_s;

    // Automatic re-boot after upgrade.
    // If the application can't handle arbitrary re-boots, set this to 'false'
    // and manually trigger the reboot.
    int auto_reboot;

} iap_https_config_t;


//_____ P R O T O T Y P E S _________________________________________________
// public interface (API)
// Module initialisation, call once at application startup.
int iap_https_init(iap_https_config_t *config);
void iap_https_update_host_name(iap_https_config_t *config);

// Manually trigger a firmware update check.
// Queries the server for a firmware update and, if one is available, installs it.
// If automatic checks are enabled, calling this function causes the timer to be re-set.
int iap_https_check_now();

// Returns 1 if an update is currently in progress, 0 otherwise.
int iap_https_update_in_progress();

// Returns 1 if a new firmware has been installed but not yet booted, 0 otherwise.
int iap_https_new_firmware_installed();

//_____ P R O T O T Y P E S _________________________________________________
// local interface (API)

#endif /* IAP_HTTPS_H_ */
