// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#include "mbed.h"
#include "SystemDns.h"
#include "lwip/err.h"
#include "lwip/ip_addr.h"
#include "lwip/dns.h"

int SystemDnsLocalRemoveHost(const char *hostname, const char *addr)
{
    ip_addr_t *address = new ip_addr_t;
    ip4_addr_set_u32(address, ipaddr_addr(addr));
    return dns_local_removehost(hostname, address);
}

int SystemDnsLocalAddHost(const char *hostname, const char *addr)
{
    ip_addr_t *address = new ip_addr_t;
    ip4_addr_set_u32(address, ipaddr_addr(addr));
    return dns_local_addhost(hostname, address);
}