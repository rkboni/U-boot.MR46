/*
 * Copyright (c) 2015 The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _QCA8511_H
#define _QCA8511_H

enum QSGMII_1_CTRL {
	QSGMII_1_CTRL0 = 0,
	QSGMII_1_CTRL1,
	QSGMII_1_CTRL2,
};

enum QSGMII_SPEED_MODE {
	FORCE_10 = 0,
	FORCE_100,
	FORCE_1000,
};

enum QSGMII_CH_MODE_CTRL {
	QSGMII_PHY = 1,
	QSGMII_MAC = 2,
};

enum QSGMII_2_CTRL {
	QSGMII_2_CTRL0 = 0,
	QSGMII_2_CTRL1 = 1,
	QSGMII_2_CTRL2 = 2,
};

enum QSGMII_3_CTRL {
	QSGMII_3_CTRL0 = 0,
	QSGMII_3_CTRL1 = 1,
	QSGMII_3_CTRL2 = 2,
};

enum QCA8511_PORT {
	STATUS_PORT0 = 0,
	STATUS_PORT1,
	STATUS_PORT2,
	STATUS_PORT3,
	STATUS_PORT4,
	STATUS_PORT5,
	STATUS_PORT6,
	STATUS_PORT7,
	STATUS_PORT8,
	STATUS_PORT9,
	STATUS_PORT10,
	STATUS_PORT11,
	STATUS_PORT12,
	STATUS_PORT13,
	STATUS_PORT14,
	STATUS_PORT15,
	STATUS_PORT16,
	STATUS_PORT17,
	STATUS_PORT18,
	STATUS_PORT19,
	STATUS_PORT20,
	STATUS_PORT21,
	STATUS_PORT22,
	STATUS_PORT23,
	STATUS_PORT24,
	STATUS_PORT25,
	STATUS_PORT26,
	STATUS_PORT27,
	STATUS_PORT28,
	STATUS_PORT29,
	STATUS_PORT_MAX = 29,
};

#define QCA8511_QSGMII_CTRL1_BASE	0x13000
#define QCA8511_QSGMII_CTRL2_BASE	0x1300c
#define QCA8511_QSGMII_CTRL3_BASE	0x13010
#define QCA8511_QSGMII_CTRL4_BASE	0x13014
#define QCA8511_PORT_BASE		0x100

#define QCA8511_QSGMII_1_CTRL(x)	(QCA8511_QSGMII_CTRL1_BASE + (x * 0x50))
#define QSGMII_1_CH0_DUPLEX_MODE	(1 << 0)
#define QSGMII_1_CH0_LINK		(1 << 1)
#define QSGMII_1_CH0_SPEED_MODE(x)	(x << 2)
#define QSGMII_1_CH0_MR_AN_EN		(1 << 10)
#define QSGMII_1_BERT_EN		(1 << 14)
#define QSGMII_1_RSVD16			(1 << 16)
#define QSGMII_1_CH0_FORCED_MODE	(1 << 21)
#define QSGMII_1_CH_MODE_CTRL(x)	(x << 22)
#define QSGMII_1_CH0_PAUSE_SG_TX_EN	(1 << 24)
#define QSGMII_1_CH0_ASYM_PAUSE		(1 << 25)
#define QSGMII_1_CH0_PAUSE		(1 << 26)
#define QSGMII_1_RSVD30			(1 << 30)
#define QSGMII_1_RSVD31			(1 << 31)

#define QCA8511_QSGMII_2_CTRL(x)	(QCA8511_QSGMII_CTRL2_BASE + (x * 0x50))
#define QSGMII_2_CH1_DUPLEX_MODE	(1 << 0)
#define QSGMII_2_CH1_LINK		(1 << 1)
#define QSGMII_2_CH1_SPEED_MODE(x)	(x << 2)
#define QSGMII_2_CH1_MR_AN_EN		(1 << 10)
#define QSGMII_2_RSVD16			(1 << 16)
#define QSGMII_2_CH1_FORCED_MODE	(1 << 21)
#define QSGMII_2_CH_RSVD(x)		(x << 22)
#define QSGMII_2_CH1_PAUSE_SG_TX_EN	(1 << 24)
#define QSGMII_2_CH1_ASYM_PAUSE		(1 << 25)
#define QSGMII_2_CH1_PAUSE		(1 << 26)
#define QSGMII_2_RSVD30			(1 << 30)
#define QSGMII_2_RSVD31			(1 << 31)

#define QCA8511_QSGMII_3_CTRL(x)	(QCA8511_QSGMII_CTRL3_BASE + (x * 0x50))
#define QSGMII_3_CH2_DUPLEX_MODE	(1 << 0)
#define QSGMII_3_CH2_LINK		(1 << 1)
#define QSGMII_3_CH2_SPEED_MODE(x)	(x << 2)
#define QSGMII_3_CH2_MR_AN_EN		(1 << 10)
#define QSGMII_3_RSVD16			(1 << 16)
#define QSGMII_3_CH2_FORCED_MODE	(1 << 21)
#define QSGMII_3_CH_RSVD(x)		(x << 22)
#define QSGMII_3_CH2_PAUSE_SG_TX_EN	(1 << 24)
#define QSGMII_3_CH2_ASYM_PAUSE		(1 << 25)
#define QSGMII_3_CH2_PAUSE		(1 << 26)
#define QSGMII_3_RSVD30			(1 << 30)
#define QSGMII_3_RSVD31			(1 << 31)

#define QCA8511_QSGMII_4_CTRL(x)	(QCA8511_QSGMII_CTRL4_BASE + (x * 0x50))
#define QSGMII_4_CH3_DUPLEX_MODE	(1 << 0)
#define QSGMII_4_CH3_LINK		(1 << 1)
#define QSGMII_4_CH3_SPEED_MODE(x)	(x << 2)
#define QSGMII_4_CH3_MR_AN_EN		(1 << 10)
#define QSGMII_4_RSVD16			(1 << 16)
#define QSGMII_4_CH3_FORCED_MODE	(1 << 21)
#define QSGMII_4_CH_RSVD(x)		(x << 22)
#define QSGMII_4_CH3_PAUSE_SG_TX_EN	(1 << 24)
#define QSGMII_4_CH3_ASYM_PAUSE		(1 << 25)
#define QSGMII_4_CH3_PAUSE		(1 << 26)
#define QSGMII_4_RSVD30			(1 << 30)
#define QSGMII_4_RSVD31			(1 << 31)

#define QCA8511_PORT_STATUS_CFG(x)	(QCA8511_PORT_BASE + (x * 0x100))
#define QCA8511_PORT_CFG_SPEED(x)	(x << 0)
#define QCA8511_PORT_CFG_TX_MAC_EN	(1 << 2)
#define QCA8511_PORT_CFG_RX_MAC_EN	(1 << 3)
#define QCA8511_PORT_CFG_DUPLEX_MODE	(1 << 6)

#define QCA8511_GLOBAL_CTRL1		0x8
#define GLOBAL_CTL1_MAC25XG_4P3G_EN	(1 << 10)
#define GLOBAL_CTL1_MAC25XG_3P125G_EN	(1 << 11)
#define GLOBAL_CTL1_MAC26SG_1P25G_EN	(1 << 15)
#define GLOBAL_CTL1_MAC27SG_3P125G_EN	(1 << 16)
#define GLOBAL_CTL1_RSVD17		(1 << 17)
#define GLOBAL_CTL1_MAC28SG_3P125G_EN	(1 << 18)
#define GLOBAL_CTL1_MAC29SG_3P125G_EN	(1 << 20)
#define GLOBAL_CTL1_RSVD22		(1 << 22)
#define GLOBAL_CTL1_SPI1_EN		(1 << 23)
#define GLOBAL_CTL1_TO_EXT_INT_EN	(1 << 25)
#define GLOBAL_CTL1_LED_CLK_EN_CFG	(1 << 27)
#define GLOBAL_CTL1_RSVD28		(1 << 28)
#define GLOBAL_CTL1_RSVD29		(1 << 29)
#define GLOBAL_CTL1_TWO_WIRE_LED_EN	(1 << 30)

#define QCA8511_XAUI_SGMII_SERDES13_CTRL0	0x1341c
#define SGMII_CTRL0_RSVD(x)			(x << 1)
#define SGMII_CTRL0_XAUI_EN_PLL(x)		(x << 3)
#define SGMII_CTRL0_XAUI_DEEMP_CH0(x)		(x << 5)
#define SGMII_CTRL0_RSVD7(x)			(x << 7)
#define SGMII_CTRL0_XAUI_DEEMP_CH2(x)		(x << 9)
#define SGMII_CTRL0_RSVD11(x)			(x << 11)
#define SGMII_CTRL0_XAUI_TH_LOS_CH1(x)		(x << 13)
#define SGMII_CTRL0_XAUI_TH_LOS_CH3(x)		(x << 15)
#define SGMII_CTRL0_XAUI_DEEMP_CH3(x)		(x << 17)
#define SGMII_CTRL0_RSVD19(x)			(x << 19)
#define SGMII_CTRL0_XAUI_EN_SD(x)		(x << 21)
#define SGMII_CTRL0_XAUI_HALFTX(x)		(x << 25)
#define SGMII_CTRL0_RSVD29(x)			(x << 29)

#define QCA8511_XAUI_SGMII_SERDES13_CTRL1	0x13420
#define SGMII_CTRL1_RSVD4(x)			(x << 4)
#define SGMII_CTRL1_XAUI_EN_RX(x)		(x << 6)
#define SGMII_CTRL1_XAUI_EN_TX(x)		(x << 10)
#define SGMII_CTRL1_XAUI_TH_LOS_CH2(x) 		(x << 14)
#define SGMII_CTRL1_XAUI_DEEMP_CH1(x)		(x << 16)
#define SGMII_CTRL1_XAUI_REG(x)			(x << 19)
#define SGMII_CTRL1_RSVD23(x)			(x << 23)
#define SGMII_CTRL1_RSVD25(x)			(x << 25)

#endif
