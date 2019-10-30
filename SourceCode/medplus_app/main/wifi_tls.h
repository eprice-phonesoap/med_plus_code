/**
@file			  wifi_tls.h
@author			  Jey
@brief			  Definition for use in wifi_tls.c

@copyright		  Pura Scents

@attention		  The information contained herein is confidential property of
                  Pura. The use, copy, transfer or disclosure
                  of such information is prohibited except by express written
                  agreement with Pura.
*/

#ifndef WIFI_TLS_H_
#define WIFI_TLS_H_

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

#include "mbedtls/platform.h"
#include "mbedtls/net.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"

#include "Platform_Types.h"
#include "Std_Types.h"

//_____D E F I N E __________________________________________________________
// public defines, enums and macros
// Forward declaration of the opaque context object.
struct wifi_tls_context_;


//_____ T Y P E D E F _______________________________________________________
// public typedefs, structs, unions
typedef struct wifi_tls_init_struct_ {

    // Name of the host that provides the firmware images, e.g. "www.classycode.io".
    const char *server_host_name;

    // Port for the connection, e.g. "443".
    const char *server_port;

    // Public key of the server's root CA certificate.
    // Needs to be in PEM format (base64-encoded DER data with begin and end marker).
    const char *server_root_ca_public_key_pem;

    // Public key of the server's peer certificate for certificate pinning.
    // Needs to be in PEM format (base64-encoded DER data with begin and end marker).
    const char *peer_public_key_pem;

} wifi_tls_init_struct_t;

typedef struct wifi_tls_request_ {

    // Request buffer.
    // Example: "GET https://www.classycode.io/esp32/ota.txt HTTP/1.1\nHost: www.classycode.io\n\n"
    // Not necessarily zero-terminated.
    char *request_buffer;

    // Number of bytes in the request buffer.
    uint32_t request_len;

    // Response buffer.
    // This buffer will be filled with the data received from the server.
    // Data may be received in chunks. Every chunk is stored in the buffer and provided
    // to the client via the response_callback function (see below).
    char *response_buffer;

    // Size of the response buffer.
    // Defines the maximum number of bytes that will be read into the response buffer.
    uint32_t response_buffer_size;

    // Make custom data available to the callback.
    void *custom_data;

    // Callback function to handle the response.
    // Return 1 to continue reading, 0 to end reading.
    int (*response_callback)(struct wifi_tls_context_ *context, struct wifi_tls_request_ *request, int index, size_t len);

} wifi_tls_request_t;

//_____ P R O T O T Y P E S _________________________________________________
// public interface (API)
// Create a context for TLS communication to a server.
// The context can be re-used for multiple connections to the same server on the same port.
// The init structure and all fields can be released after calling this function.
struct wifi_tls_context_ *wifi_tls_create_context(wifi_tls_init_struct_t *params);

// Release the context.
// Performs necessary cleanup and releases all memory associated with the context.
void wifi_tls_free_context(struct wifi_tls_context_ *context);

// Connects to the server, performs the TLS handshake and certificate verification.
// Returns 0 on success.
int wifi_tls_connect(struct wifi_tls_context_ *context);

// Disconnects from the server.
void wifi_tls_disconnect(struct wifi_tls_context_ *context);

// Send a request to the server.
// Calls the response callback function defined in the request structure.
// Returns 0 on success.
int wifi_tls_send_request(struct wifi_tls_context_ *context, wifi_tls_request_t *request);


//_____ P R O T O T Y P E S _________________________________________________
// local interface (API)

#endif /* WIFI_TLS_H_ */
