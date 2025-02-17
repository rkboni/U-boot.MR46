/*
 **************************************************************************
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
*/
#ifndef __IPQ807X_EDMA__
#define __IPQ807X_EDMA__

#define IPQ807X_NSS_DP_START_PHY_PORT   1
#define IPQ807X_NSS_DP_MAX_PHY_PORTS    6

#define IPQ807X_EDMA_BUF_SIZE		2000
#define IPQ807X_EDMA_DEVICE_NODE_NAME	"edma"
#define IPQ807X_EDMA_RX_BUFF_SIZE	(IPQ807X_EDMA_BUF_SIZE + IPQ807X_EDMA_RX_PREHDR_SIZE)
#define IPQ807X_EDMA_RX_PREHDR_SIZE	(sizeof(struct ipq807x_edma_rx_preheader))
#define IPQ807X_EDMA_TX_PREHDR_SIZE	(sizeof(struct ipq807x_edma_tx_preheader))

#define IPQ807X_EDMA_TXDESC_RING_SIZE	8
#define IPQ807X_EDMA_TXCMPL_RING_SIZE	8
#define IPQ807X_EDMA_RXDESC_RING_SIZE	16
#define IPQ807X_EDMA_RXFILL_RING_SIZE	16

#define IPQ807X_EDMA_START_GMACS	IPQ807X_NSS_DP_START_PHY_PORT
#define IPQ807X_EDMA_MAX_GMACS		IPQ807X_NSS_DP_MAX_PHY_PORTS
#define IPQ807X_EDMA_TX_BUF_SIZE	(1540 + IPQ807X_EDMA_TX_PREHDR_SIZE)

#define IPQ807X_EDMA_MAX_TXCMPL_RINGS	8	/* Max TxCmpl rings */
#define IPQ807X_EDMA_MAX_RXDESC_RINGS	16	/* Max RxDesc rings */
#define IPQ807X_EDMA_MAX_RXFILL_RINGS	8	/* Max RxFill rings */
#define IPQ807X_EDMA_MAX_TXDESC_RINGS	24	/* Max TxDesc rings */

#define IPQ807X_EDMA_GET_DESC(R, i, type) (&(((type *)((R)->desc))[i]))
#define IPQ807X_EDMA_RXFILL_DESC(R, i) 	IPQ807X_EDMA_GET_DESC(R, i, struct ipq807x_edma_rxfill_desc)
#define IPQ807X_EDMA_RXDESC_DESC(R, i) 	IPQ807X_EDMA_GET_DESC(R, i, struct ipq807x_edma_rxdesc_desc)
#define IPQ807X_EDMA_TXDESC_DESC(R, i) 	IPQ807X_EDMA_GET_DESC(R, i, struct ipq807x_edma_txdesc_desc)
#define IPQ807X_EDMA_TXCMPL_DESC(R, i) 	IPQ807X_EDMA_GET_DESC(R, i, struct ipq807x_edma_txcmpl_desc)
#define IPQ807X_EDMA_RXPH_SRC_INFO_TYPE_GET(rxph)	(((rxph)->src_info >> 8) & 0xf0)

#define IPQ807X_EDMA_DEV		1
#define IPQ807X_EDMA_TX_QUEUE		1
#define IPQ807X_EDMA_RX_QUEUE		1

//#define IPQ807X_EDMA_TX_DESC_RING_START	23
#define IPQ807X_EDMA_TX_DESC_RING_START	0
#define IPQ807X_EDMA_TX_DESC_RING_NOS	1
#define IPQ807X_EDMA_TX_DESC_RING_SIZE	\
(IPQ807X_EDMA_TX_DESC_RING_START + IPQ807X_EDMA_TX_DESC_RING_NOS)

#define IPQ807X_EDMA_TX_CMPL_RING_START	7
#define IPQ807X_EDMA_TX_CMPL_RING_NOS	1
#define IPQ807X_EDMA_TX_CMPL_RING_SIZE	\
(IPQ807X_EDMA_TX_CMPL_RING_START + IPQ807X_EDMA_TX_CMPL_RING_NOS)

#define IPQ807X_EDMA_RX_DESC_RING_START	15
#define IPQ807X_EDMA_RX_DESC_RING_NOS	1
#define IPQ807X_EDMA_RX_DESC_RING_SIZE	\
(IPQ807X_EDMA_RX_DESC_RING_START + IPQ807X_EDMA_RX_DESC_RING_NOS)

#define IPQ807X_EDMA_RX_FILL_RING_START	7
#define IPQ807X_EDMA_RX_FILL_RING_NOS	1
#define IPQ807X_EDMA_RX_FILL_RING_SIZE	\
(IPQ807X_EDMA_RX_FILL_RING_START + IPQ807X_EDMA_RX_FILL_RING_NOS)

#define IPQ807X_EDMA_TX_IMR_NORMAL_MASK	1
#define IPQ807X_EDMA_RX_IMR_NORMAL_MASK	1
#define IPQ807X_EDMA_INTR_CLEAR_TYPE	0
#define IPQ807X_EDMA_INTR_SW_IDX_W_TYPE	0
#define IPQ807X_EDMA_RSS_TYPE_NONE	0x1

#define NETDEV_TX_BUSY	1

#define PSGMIIPHY_PLL_VCO_RELATED_CTRL	0x0009878c
#define PSGMIIPHY_PLL_VCO_VAL			0x2803

#define PSGMIIPHY_VCO_CALIBRATION_CTRL	0x0009809c
#define PSGMIIPHY_VCO_VAL		0x4ADA
#define PSGMIIPHY_VCO_RST_VAL	0xADA

#define RGMII_TCSR_ESS_CFG	0x01953000
#define ESS_RGMII_CTRL		0x0C000004
/*
 * Tx descriptor
 */
struct ipq807x_edma_txdesc_desc {
	uint32_t buffer_addr;
		/* buffer address */
	uint32_t word1;
		/* more bit, TSO, preheader, pool, offset and length */
};

/*
 * TxCmpl descriptor
 */
struct ipq807x_edma_txcmpl_desc {
	uint32_t buffer_addr;	/* buffer address/opaque */
	uint32_t status;	/* status */
};

/*
 * Rx descriptor
 */
struct ipq807x_edma_rxdesc_desc {
	uint32_t buffer_addr;	/* buffer address */
	uint32_t status;	/* status */
};

/*
 * RxFill descriptor
 */
struct ipq807x_edma_rxfill_desc {
	uint32_t buffer_addr;	/* Buffer address */
	uint32_t word1;		/* opaque_ind and buffer size */
};

/*
 * Tx descriptor ring
 */
struct ipq807x_edma_txdesc_ring {
	uint32_t id;			/* TXDESC ring number */
	void *desc;			/* descriptor ring virtual address */
	dma_addr_t dma;			/* descriptor ring physical address */
	uint16_t count;			/* number of descriptors */
};

/*
 * TxCmpl ring
 */
struct ipq807x_edma_txcmpl_ring {
	uint32_t id;			/* TXCMPL ring number */
	void *desc;			/* descriptor ring virtual address */
	dma_addr_t dma;			/* descriptor ring physical address */
	uint16_t count;			/* number of descriptors in the ring */
};

/*
 * RxFill ring
 */
struct ipq807x_edma_rxfill_ring {
	uint32_t id;			/* RXFILL ring number */
	void *desc;			/* descriptor ring virtual address */
	dma_addr_t dma;			/* descriptor ring physical address */
	uint16_t count;			/* number of descriptors in the ring */
};

/*
 * RxDesc ring
 */
struct ipq807x_edma_rxdesc_ring {
	uint32_t id;			/* RXDESC ring number */
	struct ipq807x_edma_rxfill_ring *rxfill;	/* RXFILL ring used */
	void *desc;			/* descriptor ring virtual address */
	dma_addr_t dma;			/* descriptor ring physical address */
	uint16_t count;			/* number of descriptors in the ring */
};

/*
 * EDMA Tx Preheader
 */
struct ipq807x_edma_tx_preheader {
	uint32_t opaque;   /* Opaque, contains skb pointer */
	uint16_t src_info; /* Src information */
	uint16_t dst_info; /* Dest information */
	uint32_t tx_pre2;  /* SVLAN & CVLAN flag, drop prec, hash value */
	uint32_t tx_pre3;  /* STAG, CTAG */
	uint32_t tx_pre4;  /* CPU code, L3 & L4 offset, service code */
	uint32_t tx_pre5;  /* IP addr index, ACL index */
	uint32_t tx_pre6;  /* IP payload checksum, copy2cpu, timestamp, dscp */
	uint32_t tx_pre7;  /* Timestamp, QoS TAG */
};

/*
 * EDMA Rx Preheader
 */
struct ipq807x_edma_rx_preheader {
	uint32_t opaque; /* Opaque, contains skb pointer*/
	uint16_t src_info; /* Src information */
	uint16_t dst_info; /* Dest information */
	uint32_t rx_pre2; /* SVLAN & CVLAN flag, drop prec, hash value */
	uint32_t rx_pre3; /* STAG, CTAG */
	uint32_t rx_pre4; /* CPU code, L3 & L4 offset, service code */
	uint32_t rx_pre5; /* IP addr index, ACL index */
	uint32_t rx_pre6; /* IP payload checksum, copy2cpu, timestamp, dscp */
	uint32_t rx_pre7; /* Timestamp, QoS TAG */
};

enum ipq807x_edma_tx {
	EDMA_TX_OK = 0,		/* Tx success */
	EDMA_TX_DESC = 1,	/* Not enough descriptors */
	EDMA_TX_FAIL = 2,	/* Tx failure */
};


/* per core queue related information */
struct queue_per_cpu_info {
	u32 tx_mask; /* tx interrupt mask */
	u32 rx_mask; /* rx interrupt mask */
	u32 tx_status; /* tx interrupt status */
	u32 rx_status; /* rx interrupt status */
	u32 tx_start; /* tx queue start */
	u32 rx_start; /* rx queue start */
	struct ipq807x_edma_common_info *c_info; /* edma common info */
};

/* edma hw specific data */
struct ipq807x_edma_hw {
	unsigned long  __iomem *hw_addr; /* inner register address */
	u8 intr_clear_type; /* interrupt clear */
	u8 intr_sw_idx_w; /* To do chk type interrupt software index */
	u16 rx_buff_size; /* To do chk type Rx buffer size */
	u8 rss_type; /* rss protocol type */
	uint16_t rx_payload_offset; /* start of the payload offset */
	uint32_t flags; /* internal flags */
	int active; /* status */
	struct ipq807x_edma_txdesc_ring *txdesc_ring; /* Tx Descriptor Ring, SW is producer */
	struct ipq807x_edma_txcmpl_ring *txcmpl_ring; /* Tx Completion Ring, SW is consumer */
	struct ipq807x_edma_rxdesc_ring *rxdesc_ring; /* Rx Descriptor Ring, SW is consumer */
	struct ipq807x_edma_rxfill_ring *rxfill_ring; /* Rx Fill Ring, SW is producer */
	uint32_t txdesc_rings; /* Number of TxDesc rings */
	uint32_t txdesc_ring_start; /* Id of first TXDESC ring */
	uint32_t txdesc_ring_end; /* Id of the last TXDESC ring */
	uint32_t txcmpl_rings; /* Number of TxCmpl rings */
	uint32_t txcmpl_ring_start; /* Id of first TXCMPL ring */
	uint32_t txcmpl_ring_end; /* Id of last TXCMPL ring */
	uint32_t rxfill_rings; /* Number of RxFill rings */
	uint32_t rxfill_ring_start; /* Id of first RxFill ring */
	uint32_t rxfill_ring_end; /* Id of last RxFill ring */
	uint32_t rxdesc_rings; /* Number of RxDesc rings */
	uint32_t rxdesc_ring_start; /* Id of first RxDesc ring */
	uint32_t rxdesc_ring_end; /* Id of last RxDesc ring */
	uint32_t tx_intr_mask; /* tx interrupt mask */
	uint32_t rx_intr_mask; /* rx interrupt mask */
	uint32_t rxfill_intr_mask; /* Rx fill ring interrupt mask */
	uint32_t rxdesc_intr_mask; /* Rx Desc ring interrupt mask */
	uint32_t txcmpl_intr_mask; /* Tx Cmpl ring interrupt mask */
	uint32_t misc_intr_mask; /* misc interrupt interrupt mask */
};

struct ipq807x_edma_common_info {
	struct ipq807x_edma_hw hw;
};

#define MAX_PHY 6
struct ipq807x_eth_dev {
	u8 *phy_address;
	uint no_of_phys;
	uint interface;
	uint speed;
	uint duplex;
	uint sw_configured;
	uint mac_unit;
	uint mac_ps;
	int link_printed;
	u32 padding;
	struct eth_device *dev;
	struct ipq807x_edma_common_info *c_info;
	struct phy_ops *ops[MAX_PHY];
	const char phy_name[MDIO_NAME_LEN];
} __attribute__ ((aligned(8)));

static inline void* ipq807x_alloc_mem(u32 size)
{
	void *p = malloc(size);
	if (p != NULL)
		memset(p, 0, size);
	return p;
}

static inline void ipq807x_free_mem(void *ptr)
{
	if (ptr)
		free(ptr);
}

//extern struct ipq807x_edma_hw ipq807x_edma_hw;

uint32_t ipq807x_edma_reg_read(uint32_t reg_off);
void ipq807x_edma_reg_write(uint32_t reg_off, uint32_t val);


extern int get_eth_mac_address(uchar *enetaddr, uint no_of_macs);

typedef struct {
	uint count;
	u8 addr[7];
} ipq807x_edma_phy_addr_t;

/* ipq807x edma Paramaters */
typedef struct {
	uint base;
	int unit;
	uint mac_conn_to_phy;
	phy_interface_t phy;
	ipq807x_edma_phy_addr_t phy_addr;
	char phy_name[MDIO_NAME_LEN];
} ipq807x_edma_board_cfg_t;

extern void ipq807x_ppe_provision_init(void);
extern void ipq807x_speed_clock_set(int port, int speed_clock1, int speed_clock2);
extern void ipq807x_pqsgmii_speed_set(int port, int speed, int status);
extern void ipq807x_uxsgmii_speed_set(int port, int speed, int duplex, int status);
extern void ppe_port_bridge_txmac_sett(int port, int status);
extern void ipq807x_10g_r_speed_set(int port, int status);
extern int phy_status_get_from_ppe(int port_id);

#endif /* ___IPQ807X_EDMA__ */
