/**
@file			  http_client.h
@author			  Jey
@brief			  Definition for use in http_client.c

@copyright		  Pura Scents

@attention		  The information contained herein is confidential property of
                  Pura. The use, copy, transfer or disclosure
                  of such information is prohibited except by express written
                  agreement with Pura.
*/

#ifndef HTTP_CLIENT_H_
#define HTTP_CLIENT_H_

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

#include "wifi_tls.h"

#include "Platform_Types.h"
#include "Std_Types.h"

//_____D E F I N E __________________________________________________________
// public defines, enums and macros
#define HTTP_SUCCESS 0
#define HTTP_ERR_INVALID_ARGS           0x101
#define HTTP_ERR_OUT_OF_MEMORY          0x102
#define HTTP_ERR_NOT_IMPLEMENTED        0x103
#define HTTP_ERR_BUFFER_TOO_SMALL       0x104
#define HTTP_ERR_SEND_FAILED            0x105
#define HTTP_ERR_INVALID_STATUS_LINE    0x106
#define HTTP_ERR_VERSION_NOT_SUPPORTED  0x107
#define HTTP_ERR_NON_200_STATUS_CODE    0x108 // additional info = status code

//_____ T Y P E D E F _______________________________________________________
// public typedefs, structs, unions
struct wifi_tls_context_;


typedef int32_t http_err_t;

// HTTP methods to use in the requests.
// TODO Right now, this is only a partial implementation.
typedef enum {
    HTTP_GET = 0,
    // HTTP_POST, ...
} http_request_verb_t;

// Callback behaviour of a single request.
// If you can provide a response buffer that you know is big enough,
// you can let this module collect all data in the buffer before it
// invokes your callback. Otherwise, for large downloads which don't
// fit in the buffer, use HTTP_STREAM_BODY which causes the callback
// to be invoked multiple times.
typedef enum {
    HTTP_WAIT_FOR_COMPLETE_BODY,
    HTTP_STREAM_BODY,
} http_response_mode_t;

// Callback return values.
// Specify HTTP_CONTINUE_RECEIVING if you're interested to receive
// more data. The size of the content provided by the web server
// in the Content-Length header overrides this value, i.e. if there's
// no more content to be received, you can use HTTP_CONTINUE_RECEIVING
// but won't get any more callbacks for the corresponding request.
typedef enum {
    HTTP_CONTINUE_RECEIVING = 0,
    HTTP_STOP_RECEIVING
} http_continue_receiving_t;


struct http_request_;

typedef http_continue_receiving_t (*http_request_headers_callback_t)(struct http_request_ *request, int statusCode, int contentLength);
typedef http_continue_receiving_t (*http_request_body_callback_t)(struct http_request_ *request, size_t bytesReceived);
typedef void (*http_request_error_callback_t)(struct http_request_ *request, http_err_t error, int additionalInfo);

typedef struct http_request_ {

    // GET, POST, ...
    http_request_verb_t verb;

    // www.classycode.io
    const char *host;

    // /esp32/ota.txt
    const char *path;

    // Buffer to store the response.
    char response_buffer[4096];

    // Size of the response buffer.
    // Needs to be large enough to hold all HTTP headers!
    size_t response_buffer_len;

    // Invoked if something goes wrong.
    http_request_error_callback_t error_callback;

    // (Optional) callback handler invoked after all headers have been received.
    // Lets the application handle re-direction, authentication requests etc.
    http_request_headers_callback_t headers_callback;

    // Define if the body callback should be invoked once after the entire message body
    // has been received (response_buffer needs to be large enough to hold the entire body),
    // or if it should be invoked periodically after parts of the message body have been
    // stored in response_buffer.
    http_response_mode_t response_mode;

    // Callback handler to process the message body.
    // Invoked once after receiving the whole message body (HTTP_WAIT_FOR_COMPLETE_BODY)
    // or periodically after receiving more body data (HTTP_STREAM_BODY). In the latter case,
    // a callback with length 0 indicates the end of the body.
    http_request_body_callback_t body_callback;

} http_request_t;


//_____ P R O T O T Y P E S _________________________________________________
// public interface (API)
// Send the specified HTTP request on the (connected and verified) tlsContext.
// The httpRequest object needs to be kept in memory until the request has been completed.
http_err_t https_send_request(struct wifi_tls_context_ *tlsContext, http_request_t *httpRequest);


// Search the buffer for the specified key and try to parse an integer value right after the key.
// Returns 0 on success.
int http_parse_key_value_int(const char *buffer, const char *key, int *value);

// Search the buffer for the specified key. If it exists, copy the string after the key up to
// but without newline into the str buffer which has a size of strLen.
// Returns 0 on success.
int http_parse_key_value_string(const char *buffer, const char *key, char *str, int strLen);

//_____ P R O T O T Y P E S _________________________________________________
// local interface (API)

#endif /* HTTP_CLIENT_H_ */
