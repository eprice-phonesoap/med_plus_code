/**
@file			  OTAManagement.h
@author			  JGS
@brief			  OTA module related structure definitions and function prototype declaration

@copyright		  Pura

@attention		  The information contained herein is confidential property of
                  Pura. The use, copy, transfer or disclosure
                  of such information is prohibited except by express written
                  agreement with Pura.
*/

#ifndef OTA_H_
#define OTA_H_

//_____I N C L U D E ________________________________________________________
// mandatory includes needed by this file (hard dependencies)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "soc/uart_struct.h"

#include "Platform_Types.h"
#include "Std_Types.h"

#include "iap_https.h"  // Coordinating firmware updates
#include "main.h"

#include "med_plus_config.h"

//_____D E F I N E __________________________________________________________
// public defines, enums and macros



// Provide server name, path to metadata file and polling interval for OTA updates.
#define OTA_SERVER_METADATA_PATH  "/phonesoap/firmware/config.txt"


#define OTA_POLLING_INTERVAL_S    5
#define OTA_AUTO_REBOOT           1

// Provide the Root CA certificate for chain validation.
#define OTA_SERVER_ROOT_CA_PEM \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDSjCCAjKgAwIBAgIQRK+wgNajJ7qJMDmGLvhAazANBgkqhkiG9w0BAQUFADA/\n" \
"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n" \
"DkRTVCBSb290IENBIFgzMB4XDTAwMDkzMDIxMTIxOVoXDTIxMDkzMDE0MDExNVow\n" \
"PzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMRcwFQYDVQQD\n" \
"Ew5EU1QgUm9vdCBDQSBYMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\n" \
"AN+v6ZdQCINXtMxiZfaQguzH0yxrMMpb7NnDfcdAwRgUi+DoM3ZJKuM/IUmTrE4O\n" \
"rz5Iy2Xu/NMhD2XSKtkyj4zl93ewEnu1lcCJo6m67XMuegwGMoOifooUMM0RoOEq\n" \
"OLl5CjH9UL2AZd+3UWODyOKIYepLYYHsUmu5ouJLGiifSKOeDNoJjj4XLh7dIN9b\n" \
"xiqKqy69cK3FCxolkHRyxXtqqzTWMIn/5WgTe1QLyNau7Fqckh49ZLOMxt+/yUFw\n" \
"7BZy1SbsOFU5Q9D8/RhcQPGX69Wam40dutolucbY38EVAjqr2m7xPi71XAicPNaD\n" \
"aeQQmxkqtilX4+U9m5/wAl0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNV\n" \
"HQ8BAf8EBAMCAQYwHQYDVR0OBBYEFMSnsaR7LHH62+FLkHX/xBVghYkQMA0GCSqG\n" \
"SIb3DQEBBQUAA4IBAQCjGiybFwBcqR7uKGY3Or+Dxz9LwwmglSBd49lZRNI+DT69\n" \
"ikugdB/OEIKcdBodfpga3csTS7MgROSR6cz8faXbauX+5v3gTt23ADq1cEmv8uXr\n" \
"AvHRAosZy5Q6XkjEGB5YGV8eAlrwDPGxrancWYaLbumR9YbK+rlmM6pZW87ipxZz\n" \
"R8srzJmwN0jP41ZL9c8PDHIyh8bwRLtTcm1D9SZImlJnt1ir/md2cXjbDaJWFBM5\n" \
"JDGFoqgCWjBH4d1QB7wCCZAA62RjYJsWvIjJEubSfZGL+T0yjWW06XyxV3bqxbYo\n" \
"Ob8VZRzI9neWagqNdwvYkQsEjgfbKbYK7p2CNTUQ\n" \
"-----END CERTIFICATE-----\n"

#define OTA_PEER_PEM \
"-----BEGIN CERTIFICATE-----\n" \
"MIIGGDCCBQCgAwIBAgISA4LvLNA2szbJuESn1WpmXZcKMA0GCSqGSIb3DQEBCwUA\n" \
"MEoxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MSMwIQYDVQQD\n" \
"ExpMZXQncyBFbmNyeXB0IEF1dGhvcml0eSBYMzAeFw0xODEwMTkwMDAzMjVaFw0x\n" \
"OTAxMTcwMDAzMjVaMCExHzAdBgNVBAMTFmRldi5hcGkucHVyYXNjZW50cy5jb20w\n" \
"ggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC3hC+SyEDl7g6B9ggq3NOB\n" \
"fCadJpBiehbQE4Nu0O0pXz02rnn/EJE6VqJlzFXa5Qc2T6DM6vHeK71jKq8mwjSi\n" \
"6jgh5FWuEpZCeb0CzV/XCEZAKO2voWWrTSUgE2tBwZI2AWmZrxebod+XHvb8u7uF\n" \
"+/iiAVG/ysZatTUNRenAmCqvfKWnFxE3sIxaJhFFg71Rt3E0wpnsp2g+VQUp9x5h\n" \
"v1dfKO12hpZ45O0P3KSzAsb2Q4foyZrigexPPEeM7KdN6s0p92idh1R0kJO7ejmm\n" \
"OymE9I9RCkqyJ3NmZgVK/gpm/gUfJwYI7+Yvs627OFapXX3cTayfluSTjDUdSbXd\n" \
"AgMBAAGjggMfMIIDGzAOBgNVHQ8BAf8EBAMCBaAwHQYDVR0lBBYwFAYIKwYBBQUH\n" \
"AwEGCCsGAQUFBwMCMAwGA1UdEwEB/wQCMAAwHQYDVR0OBBYEFHSC9A/Pl/5yhjW1\n" \
"XMZLLypfwyPSMB8GA1UdIwQYMBaAFKhKamMEfd265tE5t6ZFZe/zqOyhMG8GCCsG\n" \
"AQUFBwEBBGMwYTAuBggrBgEFBQcwAYYiaHR0cDovL29jc3AuaW50LXgzLmxldHNl\n" \
"bmNyeXB0Lm9yZzAvBggrBgEFBQcwAoYjaHR0cDovL2NlcnQuaW50LXgzLmxldHNl\n" \
"bmNyeXB0Lm9yZy8wIQYDVR0RBBowGIIWZGV2LmFwaS5wdXJhc2NlbnRzLmNvbTCB\n" \
"/gYDVR0gBIH2MIHzMAgGBmeBDAECATCB5gYLKwYBBAGC3xMBAQEwgdYwJgYIKwYB\n" \
"BQUHAgEWGmh0dHA6Ly9jcHMubGV0c2VuY3J5cHQub3JnMIGrBggrBgEFBQcCAjCB\n" \
"ngyBm1RoaXMgQ2VydGlmaWNhdGUgbWF5IG9ubHkgYmUgcmVsaWVkIHVwb24gYnkg\n" \
"UmVseWluZyBQYXJ0aWVzIGFuZCBvbmx5IGluIGFjY29yZGFuY2Ugd2l0aCB0aGUg\n" \
"Q2VydGlmaWNhdGUgUG9saWN5IGZvdW5kIGF0IGh0dHBzOi8vbGV0c2VuY3J5cHQu\n" \
"b3JnL3JlcG9zaXRvcnkvMIIBBQYKKwYBBAHWeQIEAgSB9gSB8wDxAHcA4mlLribo\n" \
"6UAJ6IYbtjuD1D7n/nSI+6SPKJMBnd3x2/4AAAFmidk16AAABAMASDBGAiEAzfUD\n" \
"l6gn35EZRmTpL9ani0qE8L8BcCvMfP9fVMk8FtICIQCHUFRlLWcmivL5t/olXN91\n" \
"2lWIsLL0f3xzN25Bz2geKAB2ACk8UZZUyDlluqpQ/FgH1Ldvv1h6KXLcpMMM9OVF\n" \
"R/R4AAABZonZNfkAAAQDAEcwRQIhAMgOjNqd+NDc3e7bqnXMYayrSypFG9mVc7IM\n" \
"sN5H1Zx5AiAMb1AL8MuypmiaAebbs6ScesNuKM7WEzJRrUDbHrM4aTANBgkqhkiG\n" \
"9w0BAQsFAAOCAQEAAt/M0n0YYBcA5Uw6qlFvM1NOF19KanirSF8pIeBam35Js0h1\n" \
"Zp901dZ7FKG5pYGbxa3AGRVgPTnt81S5vHXV+EaimGEjuc0wM6VDFWqB2cOt2NyZ\n" \
"OEbLx4Myk3CQUWGNYKDhqWM3hdFQa55Mthy4eH8oxdCV/qT0PXDMrkAU6uqslsVu\n" \
"0+Zwz1t/1FfCddv7Wct0idGPwGAsy99bRsmjXYmkg9Tt+5i5yFCdJF4SLJvGutgH\n" \
"RCVWNVK8oNYk7Jj0knSzzt+4z3v6iAW45eZstBDtQuh3tzMJeUp6syhBFWT0v5Xr\n" \
"tEltw7iI7y+si4aX31khRhbz7olI3r49QccXfw==\n" \
"-----END CERTIFICATE-----\n"

//_____ T Y P E D E F _______________________________________________________
// public typedefs, structs, unions


//_____ P R O T O T Y P E S _________________________________________________
// public interface (API)
void OTA_Init(void);
void OTA_UpdateConfig(void);
void OTA_10msTask(void);

void OTA_CheckForUpdate(void);
//_____ P R O T O T Y P E S _________________________________________________
// local interface (API)

#endif /* OTA_H_ */
