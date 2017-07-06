// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef __TLS_SOCKET_H__
#define __TLS_SOCKET_H__

#include "mbed.h"

#include "mbedtls/platform.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"

#if DEBUG_LEVEL > 0
#include "mbedtls/debug.h"
#endif

class TLSSocket
{
public:
    TLSSocket(const char *ssl_ca_pem, NetworkInterface* net_iface);
    virtual ~TLSSocket();

    nsapi_error_t connect(const char *host, uint16_t port);
    nsapi_error_t close();
    nsapi_size_or_error_t send(const void *data, nsapi_size_t size);
    nsapi_size_or_error_t recv(void *data, nsapi_size_t size);

private:
    mbedtls_entropy_context _entropy;
    mbedtls_ctr_drbg_context _ctr_drbg;
    mbedtls_x509_crt _cacert;
    mbedtls_ssl_context _ssl;
    mbedtls_ssl_config _ssl_conf;
    
    const char *_ssl_ca_pem;
    TCPSocket *_tcp_socket;
    bool check_mbedtls_ssl_write(int ret);
};


#endif  // __TLS_SOCKET_H__
