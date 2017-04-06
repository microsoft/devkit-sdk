/**
 * mdns.h
 * This provides mDNS related functionality.
 *
 */


/**************************************************************************************************************
 * INCLUDES
 **************************************************************************************************************/

#define MDNS_PORT 5353
#define MDNS_ADDR 0xFB0000E0 //"224.0.0.251" 

extern void mdns_handler(struct netif *iface, uint8_t* pkt, int pkt_len);
extern void init_mdns(char *instance_name, char *hostname, char *txt_att, uint16_t port);

