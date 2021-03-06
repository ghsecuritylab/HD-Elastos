/*
 * @TAG(OTHER_GPL)
 */

/*
 * (C) Copyright 2009 Ilya Yanok, Emcraft Systems Ltd <yanok@emcraft.com>
 * (C) Copyright 2008 Armadeus Systems, nc
 * (C) Copyright 2008 Eric Jarrige <eric.jarrige@armadeus.org>
 * (C) Copyright 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
 * (C) Copyright 2007 Pengutronix, Juergen Beisert <j.beisert@pengutronix.de>
 *
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * This file is based on mpc4200fec.h
 * (C) Copyright Motorola, Inc., 2000
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */


#ifndef __FEC_MXC_H
#define __FEC_MXC_H

#include "../enet.h"

#define PKTSIZE			1518
#define PKTSIZE_ALIGN		1536

struct eth_device {
	char name[16];
	unsigned char enetaddr[6];
	int iobase;
	int state;
	struct fec_priv *priv;
        int (*write_hwaddr)(struct eth_device *dev);
};

int fec_init(unsigned phy_mask, struct enet* enet);

#define FEC_RCNTRL_MAX_FL_SHIFT		16
#define FEC_RCNTRL_LOOP			0x00000001
#define FEC_RCNTRL_DRT			0x00000002
#define FEC_RCNTRL_MII_MODE		0x00000004
#define FEC_RCNTRL_PROM			0x00000008
#define FEC_RCNTRL_BC_REJ		0x00000010
#define FEC_RCNTRL_FCE			0x00000020
#define FEC_RCNTRL_RGMII		0x00000040
#define FEC_RCNTRL_RMII			0x00000100
#define FEC_RCNTRL_RMII_10T		0x00000200

#define FEC_TCNTRL_GTS			0x00000001
#define FEC_TCNTRL_HBC			0x00000002
#define FEC_TCNTRL_FDEN			0x00000004
#define FEC_TCNTRL_TFC_PAUSE		0x00000008
#define FEC_TCNTRL_RFC_PAUSE		0x00000010

#define FEC_ECNTRL_RESET		0x00000001	/* reset the FEC */
#define FEC_ECNTRL_ETHER_EN		0x00000002	/* enable the FEC */
#define FEC_ECNTRL_SPEED		0x00000020
#define FEC_ECNTRL_DBSWAP		0x00000100

#define FEC_X_WMRK_STRFWD		0x00000100


/**
 * @brief i.MX27-FEC private structure
 */
struct fec_priv {
#ifdef CONFIG_PHYLIB
	int phy_mask;
	struct phy_device *phydev;
#else
	int phy_id;
	int (*mii_postcall)(int);
#endif
};
#endif	/* __FEC_MXC_H */
