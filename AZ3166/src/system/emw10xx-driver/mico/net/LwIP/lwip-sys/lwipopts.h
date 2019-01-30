/* MiCO Team
 * Copyright (c) 2017 MXCHIP Information Tech. Co.,Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__


typedef struct
{
    struct pbuf *  queue_next;
} wlan_buffer_header_t;

/**
 * The space in bytes required for headers in front of the Ethernet header.
 */
#define MICO_LINK_OVERHEAD_BELOW_ETHERNET_FRAME ( sizeof(wlan_buffer_header_t) + 12 + 4 + 2 )

/**
 * The maximum space in bytes required for headers in front of the Ethernet header.
 * This definition allows MICO to use a pre-built bus-generic network stack library regardless of choice of bus.
 * Note: adjust accordingly if a new bus is added.
 */
#define MICO_LINK_OVERHEAD_BELOW_ETHERNET_FRAME_MAX ( 8 + 12 + 4 + 2 )

/**
 * The space in bytes required after the end of an Ethernet packet
 */
#define MICO_LINK_TAIL_AFTER_ETHERNET_FRAME     ( 0 )

/**
 * The size of an Ethernet header
 */
#define MICO_ETHERNET_SIZE         (14)

/**
 * The size in bytes of the Link layer header i.e. the MICO specific headers and the Ethernet header
 */
#define MICO_PHYSICAL_HEADER       (MICO_LINK_OVERHEAD_BELOW_ETHERNET_FRAME_MAX + MICO_ETHERNET_SIZE)

/**
 * The size in bytes of the data after Link layer packet
 */
#define MICO_PHYSICAL_TRAILER      (MICO_LINK_TAIL_AFTER_ETHERNET_FRAME)

/**
 * The maximum size in bytes of the data part of an Ethernet frame
 */
#define MICO_PAYLOAD_MTU           (1500)


#define MICO_MONITOR_EXTRA_LENGTH (58)
/**
 * The maximum size in bytes of a packet used within MICO
 */
#define MICO_LINK_MTU              (MICO_PAYLOAD_MTU + MICO_MONITOR_EXTRA_LENGTH + MICO_PHYSICAL_HEADER + MICO_PHYSICAL_TRAILER)



/////////////////////////////////////////////

#define LWIP_COMPAT_SOCKETS         0

/**
 * MEM_ALIGNMENT: should be set to the alignment of the CPU
 *    4 byte alignment -> #define MEM_ALIGNMENT 4
 *    2 byte alignment -> #define MEM_ALIGNMENT 2
 */
#define MEM_ALIGNMENT                  4

/**
 * Use Malloc from LibC - saves code space
 */
#define MEM_LIBC_MALLOC                (1)

/**
 * MEMP_NUM_NETBUF: the number of struct netbufs.
 * (only needed if you use the sequential API, like api_lib.c)
 */
#define MEMP_NUM_NETBUF                (PBUF_POOL_SIZE)


/**
 * MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP segments.
 * (requires the LWIP_TCP option)
 */
//#define MEMP_NUM_TCP_SEG               (TCP_SND_QUEUELEN+1)

/** STF 8
 * PBUF_POOL_SIZE: the number of buffers in the pbuf pool.
 *
 * This is just a default designed to be overriden by the FreeRTOS.mk makefile
 * To perform this override, define the makefile variable LWIP_NUM_PACKET_BUFFERS_IN_POOL
 */
#ifndef PBUF_POOL_TX_SIZE
#define PBUF_POOL_TX_SIZE                 (2)
#endif


#define IP_REASS_MAX_PBUFS              0
#define IP_REASSEMBLY                   0
#define MEMP_NUM_REASSDATA              0
#define IP_FRAG                         0

/**
 * PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. The default is
 * designed to accomodate single full size TCP frame in one pbuf, including
 * TCP_MSS, IP header, and link header.
 */
#define PBUF_POOL_BUFSIZE              (LWIP_MEM_ALIGN_SIZE(MICO_LINK_MTU) + LWIP_MEM_ALIGN_SIZE(sizeof(struct pbuf)) + 1)


/**
 * TCP_MSS: TCP Maximum segment size. (default is 536, a conservative default,
 * you might want to increase this.)
 * For the receive side, this MSS is advertised to the remote side
 * when opening a connection. For the transmit size, this MSS sets
 * an upper limit on the MSS advertised by the remote host.
 */
#if 0
#define TCP_MSS                        (MICO_PAYLOAD_MTU-20-20)  /* TODO: Cannot use full Ethernet MTU since LwIP concatenates segments which are too long. */
#else /* if 0 */
#define TCP_MSS                        (1152)
#endif /* if 0 */


/**
 * TCP_SND_BUF: TCP sender buffer space (bytes).
 * must be at least as much as (2 * TCP_MSS) for things to work smoothly
 */
#ifdef TX_PACKET_POOL_SIZE
#define TCP_SND_BUF                    ((TX_PACKET_POOL_SIZE/2) * TCP_MSS)
#else
#define TCP_SND_BUF                    (6 * TCP_MSS)
#endif



/**
 * ETH_PAD_SIZE: the header space required preceeding the of each pbuf in the pbuf pool. The default is
 * designed to accomodate single full size TCP frame in one pbuf, including
 * TCP_MSS, IP header, and link header.
 *
 * This is zero since the role has been taken over by SUB_ETHERNET_HEADER_SPACE as ETH_PAD_SIZE was not always obeyed
 */
#define ETH_PAD_SIZE                   (0)

#define SUB_ETHERNET_HEADER_SPACE      (MICO_LINK_OVERHEAD_BELOW_ETHERNET_FRAME)


/**
 * PBUF_LINK_HLEN: the number of bytes that should be allocated for a
 * link level header. The default is 14, the standard value for
 * Ethernet.
 */
#define PBUF_LINK_HLEN                 (MICO_PHYSICAL_HEADER)


/**
 * LWIP_NETIF_TX_SINGLE_PBUF: if this is set to 1, lwIP tries to put all data
 * to be sent into one single pbuf. This is for compatibility with DMA-enabled
 * MACs that do not support scatter-gather.
 * Beware that this might involve CPU-memcpy before transmitting that would not
 * be needed without this flag! Use this only if you need to!
 *
 * @todo: TCP and IP-frag do not work with this, yet:
 */
/* TODO: remove this option once buffer chaining has been implemented */
#define LWIP_NETIF_TX_SINGLE_PBUF      (1)


/** Define LWIP_COMPAT_MUTEX if the port has no mutexes and binary semaphores
 *  should be used instead
 */
#define LWIP_COMPAT_MUTEX              (1)


/**
 * SYS_LIGHTWEIGHT_PROT==1: if you want inter-task protection for certain
 * critical regions during buffer allocation, deallocation and memory
 * allocation and deallocation.
 */
#define SYS_LIGHTWEIGHT_PROT           (1)

/**
 * TCPIP_THREAD_STACKSIZE: The stack size used by the main tcpip thread.
 * The stack size value itself is platform-dependent, but is passed to
 * sys_thread_new() when the thread is created.
 */
#ifdef MXCHIP_LIBRARY
#define TCPIP_THREAD_STACKSIZE         900 //((900) / sizeof( portSTACK_TYPE ))
#else
#define TCPIP_THREAD_STACKSIZE         2048//((900) / sizeof( portSTACK_TYPE ))
#endif

/**
 * TCPIP_THREAD_PRIO: The priority assigned to the main tcpip thread.
 * The priority value itself is platform-dependent, but is passed to
 * sys_thread_new() when the thread is created.
 */
#define TCPIP_THREAD_PRIO               (5) //(configMAX_PRIORITIES - 6)

#define TCP_LISTEN_BACKLOG     (1)


/**
 * LWIP_DHCP==1: Enable DHCP module.
 */
#define LWIP_DHCP                      (1)

/**
 * LWIP_PROVIDE_ERRNO: System does not have errno defines - force LwIP to create them
 */
//#define LWIP_PROVIDE_ERRNO             (1)
//#define ERRNO                          (1)

/**
 * MEMP_NUM_SYS_TIMEOUT: the number of simulateously active timeouts.
 * (requires NO_SYS==0)
  * Must be larger than or equal to LWIP_TCP + IP_REASSEMBLY + LWIP_ARP + (2*LWIP_DHCP) + LWIP_AUTOIP + LWIP_IGMP + LWIP_DNS + PPP_SUPPORT
 */
#define MEMP_NUM_SYS_TIMEOUT           (10)

#ifdef WOLFSSL_MAX_STRENGTH
#define DHCP_DOES_ARP_CHECK            (1)

#else

/* ARP before DHCP causes multi-second delay  - turn it off */
#define DHCP_DOES_ARP_CHECK            (0)
#endif

#define LWIP_HAVE_LOOPIF 1
/**
 * LWIP_NETIF_LOOPBACK==1: Support sending packets with a destination IP
 * address equal to the netif IP address, looping them back up the stack.
 */
#define LWIP_NETIF_LOOPBACK            (1)

/* yhb changed */
/**
 * MEMP_NUM_NETCONN: the number of struct netconns.
 * (only needed if you use the sequential API, like api_lib.c)
 */
#ifdef SMALL_MEMP
#define MEMP_NUM_NETCONN               (8)
#define MEMP_NUM_TCP_PCB 6
#define MEMP_NUM_TCP_PCB_LISTEN 2
#define TCP_MAX_ACCEPT_CONN 5
#define MEMP_NUM_UDP_PCB 6
#define LWIP_RAW 1 // mftest ping gateway
#define LWIP_MDNS 1
#define MEMP_NUM_TCP_SEG               (TCP_SND_QUEUELEN)
#define TCP_QUEUE_OOSEQ 0
#define LWIP_NETIF_HOSTNAME 1
#define LWIP_SO_SNDTIMEO               (1)
#define TCP_DEFAULT_LISTEN_BACKLOG     (5)
#define TCP_MAXRTX                     (5)
#define PBUF_POOL_RX_SIZE              (5)
#else
#define MEMP_NUM_NETCONN               (16)
#define MEMP_NUM_TCP_PCB 12
#define MEMP_NUM_TCP_PCB_LISTEN 4
#define TCP_MAX_ACCEPT_CONN 5
#define MEMP_NUM_UDP_PCB 8
#define LWIP_RAW 1 // mftest ping gateway
#define LWIP_MDNS 1
#define MEMP_NUM_TCP_SEG               (TCP_SND_QUEUELEN*2)
#define TCP_QUEUE_OOSEQ 0
#define LWIP_NETIF_HOSTNAME 1
#define LWIP_SO_SNDTIMEO               (1)
#define TCP_DEFAULT_LISTEN_BACKLOG     (5)
#define TCP_MAXRTX                     (12)
#define PBUF_POOL_RX_SIZE              (7)
#endif
#define TCP_WND                        ((PBUF_POOL_RX_SIZE/2) * TCP_MSS)
#ifdef TCP_RX_PBUF_COPY
#undef TCP_QUEUE_OOSEQ
#define TCP_QUEUE_OOSEQ 1
#define MEMP_NUM_TCPIP_MSG_INPKT        28
#endif

#ifdef MXCHIP_3166
#undef MEMP_NUM_UDP_PCB
#define MEMP_NUM_UDP_PCB 28
#endif

/**
 * LWIP_SO_RCVTIMEO==1: Enable SO_RCVTIMEO processing.
 */
#define LWIP_SO_RCVTIMEO               (1)


/**
 * LWIP_IGMP==1: Turn on IGMP module.
 */
#define LWIP_IGMP                      (1)


/**
 * SO_REUSE==1: Enable SO_REUSEADDR option.
 * Required by IGMP for reuse of multicast address and port by other sockets
 */
#define SO_REUSE                       (1)

/**
 * When using IGMP, LWIP_RAND() needs to be defined to a random-function returning an u32_t random value
 */
#define LWIP_RAND()                    (42)


#define LWIP_TCP_KEEPALIVE             (1)

/**
 * LWIP_DNS==1: Turn on DNS module. UDP must be available for DNS
 * transport.
 */
#define LWIP_DNS                        (1)


#ifdef LWIP_SO_RCVBUF
#if ( LWIP_SO_RCVBUF == 1 )
#include <limits.h>  /* Needed because RECV_BUFSIZE_DEFAULT is defined as INT_MAX */
#endif /* if ( LWIP_SO_RCVBUF == 1 ) */
#endif /* ifdef LWIP_SO_RCVBUF */

/**
 * LWIP_STATS : Turn off statistics gathering
 */
#ifdef MICO_LWIP_DEBUG
#define LWIP_STATS                     (1)
#else
#define LWIP_STATS                     (0)
#endif /* ifdef MICO_LWIP_DEBUG */

/**
 * Debug printing
 * By default enable debug printing for debug build, but set level to off
 * This allows user to change any desired debug level to on.
 */

#ifdef MICO_LWIP_DEBUG
#define LWIP_DEBUG
#define MEMP_OVERFLOW_CHECK            ( 2 )
#define MEMP_SANITY_CHECK              ( 1 )

#define MEM_DEBUG                      (LWIP_DBG_OFF)
#define MEMP_DEBUG                     (LWIP_DBG_OFF)
#define PBUF_DEBUG                     (LWIP_DBG_OFF)
#define API_LIB_DEBUG                  (LWIP_DBG_OFF)
#define API_MSG_DEBUG                  (LWIP_DBG_OFF)
#define TCPIP_DEBUG                    (LWIP_DBG_OFF)
#define NETIF_DEBUG                    (LWIP_DBG_OFF)
#define SOCKETS_DEBUG                  (LWIP_DBG_OFF)
#define DEMO_DEBUG                     (LWIP_DBG_OFF)
#define IP_DEBUG                       (LWIP_DBG_OFF)
#define IP_REASS_DEBUG                 (LWIP_DBG_OFF)
#define RAW_DEBUG                      (LWIP_DBG_OFF)
#define ICMP_DEBUG                     (LWIP_DBG_OFF)
#define UDP_DEBUG                      (LWIP_DBG_OFF)
#define TCP_DEBUG                      (LWIP_DBG_OFF)
#define TCP_INPUT_DEBUG                (LWIP_DBG_OFF)
#define TCP_OUTPUT_DEBUG               (LWIP_DBG_OFF)
#define TCP_RTO_DEBUG                  (LWIP_DBG_OFF)
#define TCP_CWND_DEBUG                 (LWIP_DBG_OFF)
#define TCP_WND_DEBUG                  (LWIP_DBG_OFF)
#define TCP_FR_DEBUG                   (LWIP_DBG_OFF)
#define TCP_QLEN_DEBUG                 (LWIP_DBG_OFF)
#define TCP_RST_DEBUG                  (LWIP_DBG_OFF)
#define PPP_DEBUG                      (LWIP_DBG_OFF)
#define ETHARP_DEBUG                   (LWIP_DBG_OFF)
#define IGMP_DEBUG                     (LWIP_DBG_OFF)
#define INET_DEBUG                     (LWIP_DBG_OFF)
#define SYS_DEBUG                      (LWIP_DBG_OFF)
#define TIMERS_DEBUG                   (LWIP_DBG_OFF)
#define SLIP_DEBUG                     (LWIP_DBG_OFF)
#define DHCP_DEBUG                     (LWIP_DBG_OFF)
#define AUTOIP_DEBUG                   (LWIP_DBG_OFF)
#define SNMP_MSG_DEBUG                 (LWIP_DBG_OFF)
#define SNMP_MIB_DEBUG                 (LWIP_DBG_OFF)
#define DNS_DEBUG                      (LWIP_DBG_OFF)

#define LWIP_DBG_TYPES_ON              (LWIP_DBG_OFF)   /* (LWIP_DBG_ON|LWIP_DBG_TRACE|LWIP_DBG_STATE|LWIP_DBG_FRESH|LWIP_DBG_HALT) */
#endif

#endif /* __LWIPOPTS_H__ */
