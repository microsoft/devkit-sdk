#ifndef __MBED_CONFIG_DATA__
#define __MBED_CONFIG_DATA__

// Configuration parameters
#define MBED_CONF_APP_NETWORK_INTERFACE             EMW10xxINTERFACE           // set by application
#define MBED_CONF_PLATFORM_DEFAULT_SERIAL_BAUD_RATE 9600                       // set by library:platform
#define MBED_CONF_EVENTS_PRESENT                    1                          // set by library:events
#define MBED_CONF_RTOS_PRESENT                      1                          // set by library:rtos
#define MBED_CONF_PLATFORM_STDIO_FLUSH_AT_EXIT      1                          // set by library:platform
#define MBED_CONF_NSAPI_PRESENT                     1                          // set by library:nsapi
#define MBED_CONF_PLATFORM_STDIO_CONVERT_NEWLINES   0                          // set by library:platform
#define MBED_CONF_PLATFORM_STDIO_BAUD_RATE          9600                       // set by library:platform
// Macros
#define MBEDTLS_SSL_PROTO_TLS1_1                                               // defined by application
#define MBEDTLS_MD5_C                                                          // defined by application
#define MBEDTLS_SHA1_C                                                         // defined by application
#define UNITY_INCLUDE_CONFIG_H                                                 // defined by library:utest
#define MBEDTLS_USER_CONFIG_FILE                    "mbedtls_entropy_config.h" // defined by application
#define HTTP_RECEIVE_BUFFER_SIZE                    8192 
#define MBED_HEAP_STATS_ENABLED                     1
#define MBED_STACK_STATS_ENABLED                    1
#endif
