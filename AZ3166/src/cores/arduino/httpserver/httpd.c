/**
 ******************************************************************************
 * @file    httpd.c
 * @author  QQ DING
 * @version V1.0.0
 * @date    1-September-2015
 * @brief   The main HTTPD server thread and its initialization.
 ******************************************************************************
 *
 *  The MIT License
 *  Copyright (c) 2014 MXCHIP Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is furnished
 *  to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 */

#include <string.h>

#include "httpd.h"
#include "httpd_wsgi.h"
#include "http-strings.h"
#include "mico.h"

typedef enum
{
    HTTPD_INACTIVE = 0,
    HTTPD_INIT_DONE,
    HTTPD_THREAD_RUNNING,
    HTTPD_THREAD_SUSPENDED,
} httpd_state_t;

httpd_state_t httpd_state;

static mico_thread_t httpd_main_thread;

#define http_server_thread_stack_size 0x2000

/* Why HTTPD_MAX_MESSAGE + 2?
 * Handlers are allowed to use HTTPD_MAX_MESSAGE bytes of this buffer.
 * Internally, the POST var processing needs a null termination byte and an
 * '&' termination byte.
 */
static bool httpd_stop_req;

#define HTTPD_CLIENT_SOCK_TIMEOUT 10
#define HTTPD_TIMEOUT_EVENT 0

/** Maximum number of backlogged http connections
 *
 *  httpd has a single listening socket from which it accepts connections.
 *  HTTPD_MAX_BACKLOG_CONN is the maximum number of connections that can be
 *  pending.  For example, suppose a webpage contains 10 images.  If a client
 *  attempts to load all 10 of those images at once, only the first
 *  HTTPD_MAX_BACKLOG_CONN attempts can succeed.  Some clients will retry when
 *  the attempts fail; others will limit the maximum number of open connections
 *  that it has.  But some may attempt to load all 10 simultaneously.  If your
 *  web pages have many images, or css files, or java script files, you may
 *  need to increase this number.
 *
 *  \note Your underlying TCP/IP stack may have other limitations
 *  besides the backlog.  For example, the treck stack limits the
 *  number of system-wide TCP sockets to TM_OPTION_TCP_SOCKETS_MAX.
 *  You will have to adjust this value if you need more than
 *  TM_OPTION_TCP_SOCKETS_MAX simultaneous TCP sockets.
 *
 */
#define HTTPD_MAX_BACKLOG_CONN 5

static int http_sockfd;

int client_sockfd;
static bool https_active;

bool httpd_is_https_active( )
{
    return https_active;
}

static int net_get_sock_error( int sock )
{
    return -kInProgressErr;
}

static int httpd_close_sockets( )
{
    int ret, status = kNoErr;

    if ( http_sockfd != -1 )
    {
        ret = close( http_sockfd );
        if ( ret != 0 )
        {
            httpd_d("failed to close http socket: %d", net_get_sock_error(http_sockfd));
            status = -kInProgressErr;
        }
        http_sockfd = -1;
    }

    if ( client_sockfd != -1 )
    {
        ret = close( client_sockfd );
        if ( ret != 0 )
        {
            httpd_d("Failed to close client socket: %d", net_get_sock_error(client_sockfd));
            status = -kInProgressErr;
        }
        client_sockfd = -1;
    }

    return status;
}

static void httpd_suspend_thread( bool warn )
{
    if ( warn )
    {
        httpd_d("Suspending thread");
    } else
    {
        httpd_d("Suspending thread");
    }
    httpd_close_sockets( );
    httpd_state = HTTPD_THREAD_SUSPENDED;
    mico_rtos_suspend_thread( NULL );
}

static int httpd_setup_new_socket( int port )
{
    int one = 1;
    int status, sockfd;
    struct sockaddr_in addr_listen;

    /* create listening TCP socket */
    sockfd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if ( sockfd < 0 )
    {
        status = net_get_sock_error( sockfd );
        httpd_d("Socket creation failed: Port: %d Status: %d", port, status);
        return status;
    }

    setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &one, sizeof(one) );

    addr_listen.sin_family = AF_INET;
    addr_listen.sin_addr.s_addr = INADDR_ANY;
    addr_listen.sin_port = htons( port );
    /* bind insocket */
    status = bind( sockfd, (struct sockaddr *) &addr_listen, sizeof(addr_listen) );
    if ( status < 0 )
    {
        status = net_get_sock_error( sockfd );
        httpd_d("Failed to bind socket on port: %d Status: %d", status, port);
        close( sockfd );
        return status;
    }

    status = listen( sockfd, HTTPD_MAX_BACKLOG_CONN );
    if ( status < 0 )
    {
        status = net_get_sock_error( sockfd );
        httpd_d("Failed to listen on port %d: %d.", port, status);
        close( sockfd );
        return status;
    }

    httpd_d("Listening on port %d.", port);
    return sockfd;
}

static int httpd_setup_main_sockets( )
{
    http_sockfd = httpd_setup_new_socket( HTTP_PORT );
    if ( http_sockfd < 0 )
    {
        /* Socket creation failed */
        return http_sockfd;
    }

    return kNoErr;
}

static int httpd_select( int max_sock, const fd_set *readfds,
                         fd_set *active_readfds,
                         int timeout_secs )
{
    int activefds_cnt;
    struct timeval timeout;

    fd_set local_readfds;

    if ( timeout_secs >= 0 )
        timeout.tv_sec = timeout_secs;
    timeout.tv_usec = 0;

    memcpy( &local_readfds, readfds, sizeof(fd_set) );
    
    activefds_cnt = select(max_sock + 1, &local_readfds, NULL, NULL, timeout_secs >= 0 ? &timeout : NULL);
    if (activefds_cnt < 0) {
        httpd_d("Select failed: %d\r\n", timeout_secs);
        httpd_suspend_thread( true );
    }

    if ( httpd_stop_req )
    {
        httpd_d("HTTPD stop request received\r\n");
        httpd_stop_req = false;
        httpd_suspend_thread( false );
    }

    if ( activefds_cnt )
    {
        /* Update users copy of fd_set only if he wants */
        if ( active_readfds )
            memcpy( active_readfds, &local_readfds, sizeof(fd_set) );
        return activefds_cnt;
    }

    httpd_d("TIMEOUT\r\n");

    return HTTPD_TIMEOUT_EVENT;
}

static int httpd_accept_client_socket( const fd_set *active_readfds )
{
    int main_sockfd = -1;
    struct sockaddr addr_from;
    socklen_t addr_from_len;

    if ( FD_ISSET( http_sockfd, active_readfds ) )
    {
        main_sockfd = http_sockfd;
        https_active = false;
    }
    
    addr_from_len = sizeof(addr_from);
    
    client_sockfd = accept( main_sockfd, &addr_from, &addr_from_len );
    if ( client_sockfd < 0 )
    {
        httpd_d("net_accept client socket failed %d.", client_sockfd);
        return -kInProgressErr;
    }
    
    /*
     * Enable TCP Keep-alive for accepted client connection
     *  -- By enabling this feature TCP sends probe packet if there is
     *  inactivity over connection for specfied interval
     *  -- If there is no response to probe packet for specified retries
     *  then connection is closed with RST packet to peer end
     *  -- Ref: http://tldp.org/HOWTO/html_single/TCP-Keepalive-HOWTO/
     *
     * We are doing this as we have single threaded web server with
     * synchronous (blocking) API usage like send, recv and they might get
     * blocked due to un-availability of peer end, causing web server to
     * be in-responsive forever.
     */
    int optval = true;
    if ( setsockopt( client_sockfd, SOL_SOCKET, 0x0008, &optval, sizeof(optval) ) == -1 )
    {
        httpd_d("Unsupported option SO_KEEPALIVE: %d", net_get_sock_error(client_sockfd));
    }
    
    /* TCP Keep-alive idle/inactivity timeout is 10 seconds */
    optval = 10;
    if ( setsockopt( client_sockfd, IPPROTO_TCP, 0x03, &optval, sizeof(optval) ) == -1 )
    {
        httpd_d("Unsupported option TCP_KEEPIDLE: %d", net_get_sock_error(client_sockfd));
    }
    
    /* TCP Keep-alive retry count is 5 */
    optval = 5;
    if ( setsockopt( client_sockfd, IPPROTO_TCP, 0x05, &optval, sizeof(optval) ) == -1 )
    {
        httpd_d("Unsupported option TCP_KEEPCNT: %d", net_get_sock_error(client_sockfd));
    }
    
    /* TCP Keep-alive retry interval (in case no response for probe
     * packet) is 1 second.
     */
    optval = 1;
    if ( setsockopt( client_sockfd, IPPROTO_TCP, 0x04, &optval, sizeof(optval) ) == -1 )
    {
        httpd_d("Unsupported option TCP_KEEPINTVL: %d", net_get_sock_error(client_sockfd));
    }

    httpd_d("connecting %d to %d.", client_sockfd, addr_from.s_port);
    
    return kNoErr;
}

static void httpd_handle_client_connection( const fd_set *active_readfds )
{
    int activefds_cnt, status;
    fd_set readfds;

    if ( httpd_stop_req )
    {
        httpd_d("HTTPD stop request received");
        httpd_stop_req = false;
        httpd_suspend_thread( false );
    }

    status = httpd_accept_client_socket( active_readfds );
    if ( status != kNoErr )
        return;

    httpd_d("Client socket accepted: %d", client_sockfd);
    FD_ZERO( &readfds );
    FD_SET( client_sockfd, &readfds );

    while ( 1 )
    {
        if ( httpd_stop_req )
        {
            httpd_d("HTTPD stop request received");
            httpd_stop_req = false;
            httpd_suspend_thread( false );
        }

        httpd_d("Waiting on client socket");
        activefds_cnt = httpd_select( client_sockfd, &readfds, NULL, HTTPD_CLIENT_SOCK_TIMEOUT );

        if ( httpd_stop_req )
        {
            httpd_d("HTTPD stop request received");
            httpd_stop_req = false;
            httpd_suspend_thread( false );
        }

        if ( activefds_cnt == HTTPD_TIMEOUT_EVENT )
        {
            /* Timeout has occurred */
            httpd_d("Client socket timeout occurred. " "Force closing socket");

            status = close( client_sockfd );
            if ( status != kNoErr )
            {
                status = net_get_sock_error( client_sockfd );
                httpd_d("Failed to close socket %d", status);
                httpd_suspend_thread( true );
            }

            client_sockfd = -1;
            break;
        }

        httpd_d("Handling %d", client_sockfd);
        /* Note:
         * Connection will be handled with call to
         * httpd_handle_message twice, first for
         * handling request (kNoErr) and second
         * time as there is no more data to receive
         * (client closed connection) and hence
         * will return with status HTTPD_DONE
         * closing socket.
         */
        /* FIXME: remove this memset if all is working well */
        /* memset(&httpd_message_in[0], 0, sizeof(httpd_message_in)); */
        status = httpd_handle_message( client_sockfd );
        if ( status == kNoErr )
        {
            /* The handlers are expected more data on the
             socket */
            continue;
        }

        /* Either there was some error or everything went well */
        httpd_d("Close socket %d.  %s: %d", client_sockfd, status == HTTPD_DONE ? "Handler done" : "Handler failed", status);

        status = close( client_sockfd );
        if ( status != kNoErr )
        {
            status = net_get_sock_error( client_sockfd );
            httpd_d("Failed to close socket %d", status);
            httpd_suspend_thread( true );
        }
        client_sockfd = -1;

        break;
    }
}

static void httpd_main( mico_thread_arg_t arg )
{
    UNUSED_PARAMETER( arg );
    int status, max_sockfd = -1;
    fd_set readfds, active_readfds;

    status = httpd_setup_main_sockets( );
    if ( status != kNoErr )
        httpd_suspend_thread( true );

    FD_ZERO( &readfds );
    FD_SET( http_sockfd, &readfds );
    max_sockfd = http_sockfd;

    while ( 1 )
    {
        httpd_select( max_sockfd, &readfds, &active_readfds, -1 );
        httpd_handle_client_connection( &active_readfds );
    }

    /*
     * Thread will never come here. The functions called from the above
     * infinite loop will cleanly shutdown this thread when situation
     * demands so.
     */
}

static inline int tcp_local_connect( int *sockfd )
{
    uint16_t port;
    int retry_cnt = 3;

    httpd_d("Doing local connect for shutting down server\n\r");

    *sockfd = -1;
    while ( retry_cnt-- )
    {
        *sockfd = socket( AF_INET, SOCK_STREAM, 0 );
        if ( *sockfd >= 0 )
            break;
        /* Wait some time to allow some sockets to get released */
        mico_thread_msleep( 1000 );
    }

    if ( *sockfd < 0 )
    {
        httpd_d("Unable to create socket to stop server");
        return -kInProgressErr;
    }

    port = HTTP_PORT;

    char *host = "127.0.0.1";
    struct sockaddr_in addr;
    memset( &addr, 0, sizeof(struct sockaddr_in) );

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr( host );
    addr.sin_port = htons( port );

    httpd_d("local connecting ...");
    if ( connect( *sockfd, (struct sockaddr *) &addr, sizeof(addr) ) != 0 )
    {
        httpd_d("Server close error. tcp connect failed %s:%d", host, port);
        close( *sockfd );
        *sockfd = 0;
        return -kInProgressErr;
    }

    /*
     * We do not wish to do anything with this connection. Its sole
     * purpose was to wake the main httpd thread out of sleep.
     */

    return kNoErr;
}

static int httpd_signal_and_wait_for_halt( )
{
    const int total_wait_time_ms = 1000 * 20; /* 20 seconds */
    const int check_interval_ms = 100; /* 100 ms */

    int num_iterations = total_wait_time_ms / check_interval_ms;

    httpd_d("Sent stop request");
    httpd_stop_req = true;

    /* Do a dummy local connect to wakeup the httpd thread */
    int sockfd;
    int rv = tcp_local_connect( &sockfd );
    if ( rv != kNoErr )
        return rv;

    while ( httpd_state != HTTPD_THREAD_SUSPENDED && num_iterations-- )
    {
        mico_thread_msleep( check_interval_ms );
    }

    close( sockfd );
    if ( httpd_state == HTTPD_THREAD_SUSPENDED )
        return kNoErr;

    httpd_d("Timed out waiting for httpd to stop. " "Force closed temporary socket");

    httpd_stop_req = false;
    return -kInProgressErr;
}

static int httpd_thread_cleanup( void )
{
    int status = kNoErr;

    switch ( httpd_state )
    {
        case HTTPD_INIT_DONE:
            /*
             * We have no threads, no sockets to close.
             */
            break;
        case HTTPD_THREAD_RUNNING:
            status = httpd_signal_and_wait_for_halt( );
            if ( status != kNoErr ) {
                httpd_d("Unable to stop thread. Force killing it.");
            }
            /* No break here on purpose */
        case HTTPD_THREAD_SUSPENDED:
            status = mico_rtos_delete_thread( &httpd_main_thread );
            if ( status != kNoErr ) {
                httpd_d("Failed to delete thread.");
            }
            status = httpd_close_sockets( );
            httpd_state = HTTPD_INIT_DONE;
            break;
        default:
            return -kInProgressErr;
    }

    return status;
}

int httpd_is_running( void )
{
    return (httpd_state == HTTPD_THREAD_RUNNING);
}

/* This pairs with httpd_stop() */
int httpd_start( void )
{
    int status;

    if ( httpd_state != HTTPD_INIT_DONE )
    {
        httpd_d("Already started");
        return kNoErr;
    }

    status = mico_rtos_create_thread( &httpd_main_thread, MICO_APPLICATION_PRIORITY, "httpd",
                                      httpd_main,
                                      http_server_thread_stack_size, 0 );

    if ( status != kNoErr )
    {
        httpd_d("Failed to create httpd thread: %d", status);
        return -kInProgressErr;
    }

    httpd_state = HTTPD_THREAD_RUNNING;
    return kNoErr;
}

/* This pairs with httpd_start() */
int httpd_stop( void )
{
    return httpd_thread_cleanup( );
}

/* This pairs with httpd_init() */
int httpd_shutdown( void )
{
    int ret;

    httpd_d("Shutting down.");

    ret = httpd_thread_cleanup( );
    if ( ret != kNoErr ) {
        httpd_d("Thread cleanup failed");
    }

    httpd_state = HTTPD_INACTIVE;

    return ret;
}

/* This pairs with httpd_shutdown() */
int httpd_init( )
{
    int status;

    if ( httpd_state != HTTPD_INACTIVE )
        return kNoErr;

    httpd_d("Initializing");

    client_sockfd = -1;
    http_sockfd = -1;

    status = httpd_wsgi_init();
    if ( status != kNoErr )
    {
        httpd_d("Failed to initialize WSGI!");
        return status;
    }

    httpd_state = HTTPD_INIT_DONE;

    return kNoErr;
}

int httpd_use_tls_certificates( const httpd_tls_certs_t *tls_certs )
{

    httpd_d("HTTPS is not enabled in server. ");
    return -kInProgressErr;
}