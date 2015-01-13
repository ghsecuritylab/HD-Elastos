#ifndef __NETIF_ETHERNETIF_H__
#define __NETIF_ETHERNETIF_H__

#include "lwip/netif.h"
#include <hdElastos.h>

#define NIOCTL_GADDR		0x01
#define ETHERNET_MTU		1500

struct eth_device
{
	/* inherit from rt_device */
	struct rt_device parent;

	struct eth_addr *ethaddr;
	struct netif *netif;
	struct rt_semaphore tx_ack;

	UInt16 link_changed;
	UInt16 link_status;

	/* eth device interface */
	struct pbuf* (*eth_rx)(rt_device_t dev);
	Int32 (*eth_tx)(rt_device_t dev, struct pbuf* p);
};

Int32 eth_rx_ready(struct eth_device* dev);
Int32 eth_device_ready(struct eth_device* dev);
Int32 eth_device_linkchange(struct eth_device* dev, rt_bool_t up);

Int32 eth_device_init(struct eth_device* dev, const char* name);

Int32 eth_system_device_init(void);

#endif /* __NETIF_ETHERNETIF_H__ */