// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include "azure_c_shared_utility/tlsio_mbedtls.h"

#ifdef USE_MBED_TLS

#include <stdlib.h>


#if defined (TARGET_LIKE_MBED)

#if defined (TIZENRT)
#include "tls/config.h"
#include "tls/debug.h"
#include "tls/ssl.h"
#include "tls/entropy.h"
#include "tls/ctr_drbg.h"
#include "tls/error.h"
#include "tls/certs.h"
#include "tls/entropy_poll.h"
#else
#include "mbedtls/config.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "mbedtls/entropy_poll.h"
#endif
#else
//mock header
#include "tls_mbed_os5_mock.h"
#endif

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "azure_c_shared_utility/tlsio_mbedtls.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/xlogging.h"

#define MBED_TLS_DEBUG_ENABLE

typedef enum TLSIO_STATE_ENUM_TAG
{
    TLSIO_STATE_NOT_OPEN,
    TLSIO_STATE_OPENING_UNDERLYING_IO,
    TLSIO_STATE_IN_HANDSHAKE,
    TLSIO_STATE_OPEN,
    TLSIO_STATE_CLOSING,
    TLSIO_STATE_ERROR
} TLSIO_STATE_ENUM;

typedef struct TLS_IO_INSTANCE_TAG
{
    XIO_HANDLE socket_io;
    TLSIO_STATE_ENUM tlsio_state;
    unsigned char* socket_io_read_bytes;
    size_t socket_io_read_byte_count;

    ON_BYTES_RECEIVED on_bytes_received;
    ON_IO_OPEN_COMPLETE on_io_open_complete;
    ON_IO_CLOSE_COMPLETE on_io_close_complete;
    ON_IO_ERROR on_io_error;
    ON_SEND_COMPLETE on_send_complete;

    void* on_bytes_received_context;
    void* on_io_open_complete_context;
    void* on_io_close_complete_context;
    void* on_io_error_context;
    void* on_send_complete_callback_context;
    
    char* hostname;

    bool tls_ready;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config config;
    mbedtls_x509_crt cacert;
    mbedtls_ssl_session ssn;
    mbedtls_x509_crt owncert;
    mbedtls_pk_context pKey;
} TLS_IO_INSTANCE;


static const IO_INTERFACE_DESCRIPTION tlsio_mbedtls_interface_description =
{
    tlsio_mbedtls_retrieveoptions,
    tlsio_mbedtls_create,
    tlsio_mbedtls_destroy,
    tlsio_mbedtls_open,
    tlsio_mbedtls_close,
    tlsio_mbedtls_send,
    tlsio_mbedtls_dowork,
    tlsio_mbedtls_setoption
};

#if defined (MBED_TLS_DEBUG_ENABLE)
void mbedtls_debug(void *ctx, int level, const char *file, int line, const char *str)
{
    ((void)level);
    ((void)ctx);
    printf("%s (%d): %s\r\n", file, line, str);
}
#endif

static void indicate_error(TLS_IO_INSTANCE* tls_io_instance)
{
    if ((tls_io_instance->tlsio_state == TLSIO_STATE_NOT_OPEN)
        || (tls_io_instance->tlsio_state == TLSIO_STATE_ERROR))
    {
        return;
    }
    tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
    if (tls_io_instance->on_io_error != NULL)
    {
        tls_io_instance->on_io_error(tls_io_instance->on_io_error_context);
    }
}

static void indicate_open_complete(TLS_IO_INSTANCE* tls_io_instance, IO_OPEN_RESULT open_result)
{
    if (tls_io_instance->on_io_open_complete != NULL)
    {
        tls_io_instance->on_io_open_complete(tls_io_instance->on_io_open_complete_context, open_result);
    }
}

/* Codes_SRS_TLSIO_MBED_OS5_TLS_99_003: [ The decode_ssl_received_bytes shall invoke mbedtls_ssl_read to read data. ] */
static int decode_ssl_received_bytes(TLS_IO_INSTANCE* tls_io_instance)
{
    int result = 0;
    unsigned char buffer[64];
    int rcv_bytes = 1;
    
    while (rcv_bytes > 0)
    {
        // The mbedtls_ssl_read calls the underlying xio_dowork, which is the socketio_dowork, recieve and send data.
        rcv_bytes = mbedtls_ssl_read(&tls_io_instance->ssl, buffer, sizeof(buffer));
        if (rcv_bytes > 0)
        {
            if (tls_io_instance->on_bytes_received != NULL)
            {
                tls_io_instance->on_bytes_received(tls_io_instance->on_bytes_received_context, buffer, rcv_bytes);
            }
        }
    }

    return result;
}

// The function would always return completed for handshake done.
static int on_handshake_done(mbedtls_ssl_context* ssl, void* context)
{
    (void)ssl;
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;

    if (tls_io_instance->tlsio_state == TLSIO_STATE_IN_HANDSHAKE)
    {
        //The phase 'TLSIO_STATE_IN_HANDSHAKE' means the adapter is already shaking hand with the server and waiting for hand shake complete.
        tls_io_instance->tlsio_state = TLSIO_STATE_OPEN;
        indicate_open_complete(tls_io_instance, IO_OPEN_OK);
    }

    return 0;
}

static int mbedtls_connect(void* context) 
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;
    int result = 0;

    // Adapter should keep shaking when the return value is MBEDTLS_ERR_SSL_WANT_READ & MBEDTLS_ERR_SSL_WANT_WRITE. 0 means shakehand completed
    do
    {
        result = mbedtls_ssl_handshake(&tls_io_instance->ssl);
    } while (result == MBEDTLS_ERR_SSL_WANT_READ || result == MBEDTLS_ERR_SSL_WANT_WRITE);

    if (result == 0)
    {
        on_handshake_done(&tls_io_instance->ssl, (void *)tls_io_instance);
    }

    return result;
}

static void on_underlying_io_open_complete(void* context, IO_OPEN_RESULT open_result)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;
    int result = 0;
    if (open_result != IO_OPEN_OK)
    {
        // If open_result is not in correct state, indicate open complete with error.
        tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
        indicate_open_complete(tls_io_instance, IO_OPEN_ERROR);
    }
    else
    {
        tls_io_instance->tlsio_state = TLSIO_STATE_IN_HANDSHAKE;
        result = mbedtls_connect(tls_io_instance);
        if (result != 0)
        {
            //If the connection of mbedtls failed, indicate open complete with error.
            indicate_open_complete(tls_io_instance, IO_OPEN_ERROR);
        }
    }
}

static void on_underlying_io_bytes_received(void* context, const unsigned char* buffer, size_t size)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;

    unsigned char* new_socket_io_read_bytes = (unsigned char*)realloc(tls_io_instance->socket_io_read_bytes, tls_io_instance->socket_io_read_byte_count + size);

    if (new_socket_io_read_bytes == NULL)
    {
        //Unable to allocate memory
        indicate_error(tls_io_instance);
    }
    else
    {
        tls_io_instance->socket_io_read_bytes = new_socket_io_read_bytes;
        (void)memcpy(tls_io_instance->socket_io_read_bytes + tls_io_instance->socket_io_read_byte_count, buffer, size);
        tls_io_instance->socket_io_read_byte_count += size;
    }
}

static void on_underlying_io_error(void* context)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;
    switch (tls_io_instance->tlsio_state)
    {
    default:
    case TLSIO_STATE_NOT_OPEN:
    case TLSIO_STATE_ERROR:
        break;

    case TLSIO_STATE_OPENING_UNDERLYING_IO:
    case TLSIO_STATE_IN_HANDSHAKE:
        tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
        indicate_open_complete(tls_io_instance, IO_OPEN_ERROR);
        break;

    case TLSIO_STATE_OPEN:
        indicate_error(tls_io_instance);
        break;
    }
}

static void on_underlying_io_close_complete(void* context)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;

    // The tlsio_mbed_os5_tls shall call on_io_close_complete along with its associated context.
    if (tls_io_instance->tlsio_state == TLSIO_STATE_CLOSING)
    {
        if (tls_io_instance->on_io_close_complete != NULL)
        {
            tls_io_instance->on_io_close_complete(tls_io_instance->on_io_close_complete_context);
        }

        tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;
    }
}

static int on_io_recv(void *context, unsigned char *buf, size_t sz)
{
    int result;
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;

    unsigned char* new_socket_io_read_bytes;

    while (tls_io_instance->socket_io_read_byte_count == 0)
    {
        // Keep invoking xio_dowork to use socket_io to receive data 
        xio_dowork(tls_io_instance->socket_io);
        if (tls_io_instance->tlsio_state == TLSIO_STATE_OPEN)
        {
            break;
        }
        else if (tls_io_instance->tlsio_state == TLSIO_STATE_NOT_OPEN)
        {
            return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
        }        
    }

    result = tls_io_instance->socket_io_read_byte_count;
    if (result > (int)sz)
    {
        result = sz;
    }

    if (result > 0)
    {
        (void)memcpy((void *)buf, tls_io_instance->socket_io_read_bytes, result);
        (void)memmove(tls_io_instance->socket_io_read_bytes, tls_io_instance->socket_io_read_bytes + result, tls_io_instance->socket_io_read_byte_count - result);
        tls_io_instance->socket_io_read_byte_count -= result;
        if (tls_io_instance->socket_io_read_byte_count > 0)
        {
            new_socket_io_read_bytes = (unsigned char*)realloc(tls_io_instance->socket_io_read_bytes, tls_io_instance->socket_io_read_byte_count);
            if (new_socket_io_read_bytes != NULL)
            {
                tls_io_instance->socket_io_read_bytes = new_socket_io_read_bytes;
            }
        }
        else
        {
            free(tls_io_instance->socket_io_read_bytes);
            tls_io_instance->socket_io_read_bytes = NULL;
        }
    }


    if ((result == 0) && (tls_io_instance->tlsio_state == TLSIO_STATE_OPEN))
    {
        result = MBEDTLS_ERR_SSL_WANT_READ;
    }
    return result;
}

static int on_io_send(void *context, const unsigned char *buf, size_t sz)
{
    int result;
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;

    // Invoking xio_send to use socket_io to send data 
    if (xio_send(tls_io_instance->socket_io, buf, sz, tls_io_instance->on_send_complete, tls_io_instance->on_send_complete_callback_context) != 0)
    {
        indicate_error(tls_io_instance);
        result = 0;
    }
    else
    {
        result = sz;
    }

    return result;
}

static int tlsio_entropy_poll(void *v, unsigned char *output, size_t len, size_t *olen)
{
    srand(time(NULL));
    char *c = (char*)malloc(len);
    memset(c, 0, len);
    for (uint16_t i = 0; i < len; i++) {
        c[i] = rand() % 256;
    }
    memmove(output, c, len);
    *olen = len;

    free(c);
    return(0);
}

// Initialize mbedTLS
static void mbedtls_init(TLS_IO_INSTANCE* tls_io_instance)
{
    if (tls_io_instance->tls_ready)
    {
        return;
    }
    
    const char *pers = "azure_iot_client";

    // mbedTLS initialize...
    mbedtls_x509_crt_init(&tls_io_instance->cacert);

    mbedtls_entropy_init(&tls_io_instance->entropy);
    mbedtls_entropy_add_source(&tls_io_instance->entropy, tlsio_entropy_poll, NULL, 128, 0);

    mbedtls_ctr_drbg_init(&tls_io_instance->ctr_drbg);
    mbedtls_ctr_drbg_seed(&tls_io_instance->ctr_drbg, mbedtls_entropy_func, &tls_io_instance->entropy, (const unsigned char *)pers, strlen(pers));

    mbedtls_ssl_config_init(&tls_io_instance->config);
    mbedtls_ssl_config_defaults(&tls_io_instance->config, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
    mbedtls_ssl_conf_rng(&tls_io_instance->config, mbedtls_ctr_drbg_random, &tls_io_instance->ctr_drbg);
    mbedtls_ssl_conf_authmode(&tls_io_instance->config, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_min_version(&tls_io_instance->config, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_3);          // v1.2
#if defined (MBED_TLS_DEBUG_ENABLE)
    mbedtls_ssl_conf_dbg(&tls_io_instance->config, mbedtls_debug, stdout);
    mbedtls_debug_set_threshold(1);
#endif

    mbedtls_ssl_init(&tls_io_instance->ssl);
    mbedtls_ssl_set_bio(&tls_io_instance->ssl, tls_io_instance, on_io_send, on_io_recv, NULL);
    mbedtls_ssl_set_hostname(&tls_io_instance->ssl, tls_io_instance->hostname);

    mbedtls_ssl_session_init(&tls_io_instance->ssn);

    mbedtls_ssl_set_session(&tls_io_instance->ssl, &tls_io_instance->ssn);
    mbedtls_ssl_setup(&tls_io_instance->ssl, &tls_io_instance->config);

    tls_io_instance->tls_ready = true;
}

// Un-initialize mbedTLS
static void mbedtls_uninit(TLS_IO_INSTANCE* tls_io_instance)
{
    if (tls_io_instance->tls_ready)
    {
        // mbedTLS cleanup...
        mbedtls_ssl_close_notify(&tls_io_instance->ssl);
        mbedtls_ssl_free(&tls_io_instance->ssl);
        mbedtls_ssl_config_free(&tls_io_instance->config);
        mbedtls_x509_crt_free(&tls_io_instance->cacert);
        mbedtls_ctr_drbg_free(&tls_io_instance->ctr_drbg);
        mbedtls_entropy_free(&tls_io_instance->entropy);

        tls_io_instance->tls_ready = false;
    }
}

/* Codes_SRS_TLSIO_MBED_OS5_TLS_99_003: [ The tlsio_mbedtls_retrieveoptions shall not do anything, and return NULL. ]*/
OPTIONHANDLER_HANDLE tlsio_mbedtls_retrieveoptions(CONCRETE_IO_HANDLE handle)
{
    return NULL;
}

/* Codes_SRS_TLSIO_MBED_OS5_TLS_99_004: [ The tlsio_mbedtls_create shall create a new instance of the tlsio for mbed TLS. ]*/
CONCRETE_IO_HANDLE tlsio_mbedtls_create(void* io_create_parameters)
{
    TLSIO_CONFIG* tls_io_config = (TLSIO_CONFIG*)io_create_parameters;
    TLS_IO_INSTANCE* result;

    /* Codes_SRS_TLSIO_MBED_OS5_TLS_99_005: [ The tlsio_mbedtls_create shall return NULL when io_create_parameters is NULL. ]*/
    if (tls_io_config == NULL)
    {
        LogError("NULL tls_io_config");
        return NULL;
    }
    
    /* Codes_SRS_TLSIO_MBED_OS5_TLS_99_006: [ The tlsio_mbedtls_create shall return NULL if allocating memory for TLS_IO_INSTANCE failed. ]*/
    result = calloc(1, sizeof(TLS_IO_INSTANCE));
    if (result != NULL)
    {
        const IO_INTERFACE_DESCRIPTION* underlying_io_interface;
        void* io_interface_parameters;

        if (tls_io_config->underlying_io_interface != NULL)
        {
            underlying_io_interface = tls_io_config->underlying_io_interface;
            io_interface_parameters = tls_io_config->underlying_io_parameters;
        }
        else
        {
            SOCKETIO_CONFIG socketio_config;

            socketio_config.hostname = tls_io_config->hostname;
            socketio_config.port = tls_io_config->port;
            socketio_config.accepted_socket = NULL;
            underlying_io_interface = socketio_get_interface_description();
            io_interface_parameters = &socketio_config;
        }

        if (underlying_io_interface == NULL)
        {
            free(result);
            LogError("Failed getting socket IO interface description.");
            return NULL;
        }
        
        result->socket_io = xio_create(underlying_io_interface, io_interface_parameters);
        if (result->socket_io == NULL)
        {
            free(result);
            LogError("socket xio create failed");
            return NULL;
        }
        result->hostname = strdup(tls_io_config->hostname);
        if (result->socket_io == NULL)
        {
            free(result);
            return NULL;
        }
        result->tls_ready = false;
        mbedtls_init(result);
                
        result->tlsio_state = TLSIO_STATE_NOT_OPEN;
    }
    
    return result;
}

/* Codes_SRS_TLSIO_MBED_OS5_TLS_99_007: [ The tlsio_mbedtls_destroy shall destroy a created instance of the tlsio for mbed os5 identified by the CONCRETE_IO_HANDLE. ]*/
/* Codes_SRS_TLSIO_MBED_OS5_TLS_99_008: [ The tlsio_mbedtls_destroy shall succeed for all tlsio_state. ]*/
void tlsio_mbedtls_destroy(CONCRETE_IO_HANDLE tls_io)
{
    if (tls_io == NULL)
    {
        return;
    }
    
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

    mbedtls_uninit(tls_io_instance);

    if (tls_io_instance->socket_io_read_bytes != NULL)
    {
        free(tls_io_instance->socket_io_read_bytes);
    }
    if (tls_io_instance->hostname != NULL)
    {
        free(tls_io_instance->hostname);
    }

    xio_destroy(tls_io_instance->socket_io);

    free(tls_io);
}

/* Codes_SRS_TLSIO_MBED_OS5_TLS_99_009: [ The tlsio_mbedtls_open shall start the process to open the ssl connection with the socket_io provided in the tlsio_mbedtls_create. ]*/
int tlsio_mbedtls_open(CONCRETE_IO_HANDLE tls_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    /* Codes_SRS_TLSIO_MBED_OS5_TLS_99_010: [ The tlsio_mbedtls_open shall fail if the input handle is null ]*/
    if (tls_io == NULL)
    {
        LogError("NULL tls_io");
        return __FAILURE__;
    }
    
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

    /* Codes_SRS_TLSIO_MBED_OS5_TLS_99_011: [ The tlsio_mbedtls_open shall fail if the tlsio_state is not correct ]*/
    if (tls_io_instance->tlsio_state != TLSIO_STATE_NOT_OPEN)
    {
        LogError("IO should not be open: %d\n", tls_io_instance->tlsio_state);
        return __FAILURE__;
    }
    
    tls_io_instance->on_bytes_received = on_bytes_received;
    tls_io_instance->on_bytes_received_context = on_bytes_received_context;

    tls_io_instance->on_io_open_complete = on_io_open_complete;
    tls_io_instance->on_io_open_complete_context = on_io_open_complete_context;

    tls_io_instance->on_io_error = on_io_error;
    tls_io_instance->on_io_error_context = on_io_error_context;

    tls_io_instance->tlsio_state = TLSIO_STATE_OPENING_UNDERLYING_IO;
    
    mbedtls_init(tls_io_instance);

    /* Codes_SRS_TLSIO_MBED_OS5_TLS_99_012: [ The tlsio_mbedtls_open shall fail if the underlying socketio open failed ]*/
    if (xio_open(tls_io_instance->socket_io, on_underlying_io_open_complete, tls_io_instance, on_underlying_io_bytes_received, tls_io_instance, on_underlying_io_error, tls_io_instance) != 0)
    {
        LogError("Underlying IO open failed");
        tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;
        return __FAILURE__;
    }
    
    return 0;
}

/* Codes_SRS_TLSIO_MBED_OS5_TLS_99_013: [ The tlsio_mbedtls_close shall close the underlying socket_io connection. ]*/
int tlsio_mbedtls_close(CONCRETE_IO_HANDLE tls_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context)
{
    /* Codes_SRS_TLSIO_MBED_OS5_TLS_99_014: [ The tlsio_mbedtls_close shall fail when input handle is null. ]*/
    if (tls_io == NULL)
    {
        return __FAILURE__;
    }
    
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

    /* Codes_SRS_TLSIO_MBED_OS5_TLS_99_015: [ The tlsio_mbedtls_close shall fail when tlsio state is not correct. ]*/
    if ((tls_io_instance->tlsio_state == TLSIO_STATE_NOT_OPEN) ||
        (tls_io_instance->tlsio_state == TLSIO_STATE_CLOSING))
    {
        return __FAILURE__;
    }
    
    tls_io_instance->tlsio_state = TLSIO_STATE_CLOSING;
    tls_io_instance->on_io_close_complete = on_io_close_complete;
    tls_io_instance->on_io_close_complete_context = callback_context;
    if (xio_close(tls_io_instance->socket_io, on_underlying_io_close_complete, tls_io_instance) != 0)
    {
        return __FAILURE__;
    }
    
    mbedtls_uninit(tls_io_instance);

    return 0;
}

/* Codes_SRS_TLSIO_MBED_OS5_TLS_99_016: [ The tlsio_mbedtls_send shall send all bytes in a buffer to mbed os5 tls connection. ]*/
int tlsio_mbedtls_send(CONCRETE_IO_HANDLE tls_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    /* Codes_SRS_TLSIO_MBED_OS5_TLS_99_017: [ The tlsio_mbedtls_send shall fail when the input tlsio is null. ] */
    /* Codes_SRS_TLSIO_MBED_OS5_TLS_99_018: [ The tlsio_mbedtls_send shall fail when the input buffer is null. ] */
    /* Codes_SRS_TLSIO_MBED_OS5_TLS_99_019: [ The tlsio_mbedtls_send shall fail when the input buffer size is 0. ] */
    if (tls_io == NULL || (buffer == NULL) || (size == 0))
    {
        return __FAILURE__;
    }

    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;
    /* Codes_SRS_TLSIO_MBED_OS5_TLS_99_020: [ If the tlsio state is not TLSIO_STATE_OPEN, the tlsio_mbedtls_send shall return 0. ]*/
    if (tls_io_instance->tlsio_state != TLSIO_STATE_OPEN)
    {
        return 0;
    }

    tls_io_instance->on_send_complete = on_send_complete;
    tls_io_instance->on_send_complete_callback_context = callback_context;
    /* Codes_SRS_TLSIO_MBED_OS5_TLS_99_021: [ The tlsio_mbedtls_send shall fail when mbedtls_ssl_write does not return correct size. ] */
    int res = mbedtls_ssl_write(&tls_io_instance->ssl, buffer, size);
    if (res != (int)size)
    {
        return __FAILURE__;
    }
    
    return 0;
}

/* Codes_SRS_TLSIO_MBED_OS5_TLS_99_022: [ The tlsio_mbedtls_dowork shall execute the async jobs for the tlsio. ] */
void tlsio_mbedtls_dowork(CONCRETE_IO_HANDLE tls_io)
{
    if (tls_io == NULL)
    {
        return;
    }

    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;
    if (tls_io_instance->tlsio_state == TLSIO_STATE_OPENING_UNDERLYING_IO
        || tls_io_instance->tlsio_state == TLSIO_STATE_IN_HANDSHAKE
        || tls_io_instance->tlsio_state == TLSIO_STATE_OPEN)
    {
        /* Codes_SRS_TLSIO_MBED_OS5_TLS_99_023: [ The tlsio_mbedtls_dowork shall invoke decode_ssl_received_bytes to do the concrete work. ] */
        decode_ssl_received_bytes(tls_io_instance);
    }
}

/* Codes_SRS_TLSIO_MBED_OS5_TLS_99_024: [ The tlsio_mbedtls_get_interface_description shall return the VTable IO_INTERFACE_DESCRIPTION. ]*/
const IO_INTERFACE_DESCRIPTION* tlsio_mbedtls_get_interface_description(void)
{
    return &tlsio_mbedtls_interface_description;
}

/* Codes_SRS_TLSIO_MBED_OS5_TLS_99_025: [ The tlsio_mbedtls_setoption shall set the option on mbedtls connection. ]*/
int tlsio_mbedtls_setoption(CONCRETE_IO_HANDLE tls_io, const char* optionName, const void* value)
{
    if (tls_io == NULL || optionName == NULL)
    {
        /* Codes_SRS_TLSIO_MBED_OS5_TLS_99_026: [ If the tlsio io is null or the option is not provided, the tlsio_mbedtls_setoption shall return __FAILURE__. ]*/
        return __FAILURE__;
    }
    
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

    /* Codes_SRS_TLSIO_MBED_OS5_TLS_99_027: [ The tlsio_mbedtls_setoption shall set the option on mbedtls connection when the optionName = TrustedCerts. ]*/
    /* Codes_SRS_TLSIO_MBED_OS5_TLS_99_028: [ The tlsio_mbedtls_setoption shall fail when the optionName = TrustedCerts but mbedtls_x509_crt_parse() fails. ]*/
    if (strcmp("TrustedCerts", optionName) == 0)
    {
        if (mbedtls_x509_crt_parse(&tls_io_instance->cacert, (const unsigned char *)value, (int)(strlen(value) + 1)) != 0)
        {
            return __FAILURE__;
        }
        else
        {
            mbedtls_ssl_conf_ca_chain(&tls_io_instance->config, &tls_io_instance->cacert, NULL);
        }
    }
    /* Codes_SRS_TLSIO_MBED_OS5_TLS_99_029: [ The tlsio_mbedtls_setoption shall set the option on mbedtls connection when the optionName = x509certificate. ]*/
    /* Codes_SRS_TLSIO_MBED_OS5_TLS_99_030: [ The tlsio_mbedtls_setoption shall fail when the optionName = x509certificate but mbedtls_x509_crt_parse() fails. ]*/
    else if (strcmp("x509certificate", optionName) == 0)
    {
        if (mbedtls_x509_crt_parse(&tls_io_instance->owncert, value, (int)(strlen(value) + 1)) != 0)
        {
            return __FAILURE__;
        }
        else if (tls_io_instance->pKey.pk_info != NULL)
        {
            mbedtls_ssl_conf_own_cert(&tls_io_instance->config, &tls_io_instance->owncert, &tls_io_instance->pKey);
        }
    }
    /* Codes_SRS_TLSIO_MBED_OS5_TLS_99_031: [ The tlsio_mbedtls_setoption shall set the option on mbedtls connection when the optionName = x509privatekey. ]*/
    /* Codes_SRS_TLSIO_MBED_OS5_TLS_99_032: [ The tlsio_mbedtls_setoption shall fail when the optionName = x509privatekey but mbedtls_pk_parse_key() fails. ]*/
    else if (strcmp("x509privatekey", optionName) == 0)
    {
        if (mbedtls_pk_parse_key(&tls_io_instance->pKey, value, (int)(strlen(value) + 1), NULL, 0) != 0)
        {
            return __LINE__;
        }
        else if (tls_io_instance->owncert.version > 0)
        {
            mbedtls_ssl_conf_own_cert(&tls_io_instance->config, &tls_io_instance->owncert, &tls_io_instance->pKey);
        }
    }
    else
    {
        if (tls_io_instance->socket_io == NULL)
        {
            return __FAILURE__;
        }
        else
        {
            return xio_setoption(tls_io_instance->socket_io, optionName, value);
        }
    }
    
    return 0;
}

#endif // USE_MBED_TLS
