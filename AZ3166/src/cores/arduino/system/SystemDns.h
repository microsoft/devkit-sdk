// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#ifndef __SYSTEM_DNS_H__
#define __SYSTEM_DNS_H__

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

/*
* @brief    Remove all entries from the local host-list for a specific hostname and/or IP addess.
*
* @param    hostname   hostname for which entries shall be removed from the local host-list.
*           addr       address for which entries shall be removed from the local host-list.
*
* @return   the number of removed entries.
*/
int SystemDnsLocalRemoveHost(const char *hostname, const char *addr);

/*
* @brief    Add a hostname/IP address pair to the local host-list. Duplicates are not checked.
*
* @param    hostname   hostname of the new entry.
*           addr       IP address of the new entry.
*
* @return   Return 0 on success, otherwise return -1.
*/
int SystemDnsLocalAddHost(const char *hostname, const char *addr);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __SYSTEM_DNS_H__
