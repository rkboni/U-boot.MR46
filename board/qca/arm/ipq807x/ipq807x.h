/*
 * Copyright (c) 2016-2017, The Linux Foundation. All rights reserved.
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

#ifndef _IPQ807X_H_
#define _IPQ807X_H_

#include <configs/ipq807x.h>
#include <asm/u-boot.h>
#include <asm/arch-qca-common/qca_common.h>

/*
 * GCC-SDCC Registers
 */
#define GCC_SDCC1_MISC		0x1842020
#define GCC_SDCC1_APPS_CBCR	0x1842018
#define GCC_SDCC1_APPS_CFG_RCGR	0x1842008
#define GCC_SDCC1_APPS_CMD_RCGR	0x1842004
#define GCC_SDCC1_APPS_M	0x184200C
#define GCC_SDCC1_APPS_N	0x1842010
#define GCC_SDCC1_APPS_D	0x1842014
#define GCC_BLSP1_UART1_APPS_CBCR       0x0180203c
#define GCC_SDCC1_BCR		0x01842000

#define GCC_BLSP1_UART2_APPS_CFG_RCGR	0x01803038
#define GCC_BLSP1_UART2_APPS_M		0x0180303C
#define GCC_BLSP1_UART2_APPS_N		0x01803040
#define GCC_BLSP1_UART2_APPS_D		0x01803044
#define GCC_BLSP1_UART2_APPS_CMD_RCGR	0x01803034
#define GCC_BLSP1_UART2_APPS_CBCR	0x0180302C

#define GCC_UART_CFG_RCGR_MODE_MASK	0x3000
#define GCC_UART_CFG_RCGR_SRCSEL_MASK	0x0700
#define GCC_UART_CFG_RCGR_SRCDIV_MASK	0x001F

#define GCC_UART_CFG_RCGR_MODE_SHIFT	12
#define GCC_UART_CFG_RCGR_SRCSEL_SHIFT	8
#define GCC_UART_CFG_RCGR_SRCDIV_SHIFT	0

#define UART2_RCGR_SRC_SEL	0x1
#define UART2_RCGR_SRC_DIV	0x0
#define UART2_RCGR_MODE		0x2
#define UART2_CMD_RCGR_UPDATE	0x1
#define UART2_CBCR_CLK_ENABLE	0x1

#define NOT_2D(two_d)		(~two_d)
#define NOT_N_MINUS_M(n,m)	(~(n - m))
#define CLOCK_UPDATE_TIMEOUT_US	1000


#define GCC_SYS_NOC_PCIE0_AXI_CLK	0x01826048
#define GCC_PCIE0_PHY_BCR	0x01875038
#define GCC_PCIE0PHY_PHY_BCR	0x0187503C
#define GCC_PCIE0_AXI_M_CBCR	0x01875008
#define GCC_PCIE0_AXI_S_CBCR	0x0187500C
#define GCC_PCIE0_AHB_CBCR	0x01875010
#define GCC_PCIE0_AUX_CBCR	0x01875014
#define GCC_PCIE0_PIPE_CBCR	0x01875018
#define GCC_PCIE0_AUX_CMD_RCGR	0x01875020
#define GCC_PCIE0_AXI_CMD_RCGR	0x01875050
#define GCC_PCIE0_AXI_CFG_RCGR	0x01875058

#define GCC_SYS_NOC_PCIE1_AXI_CLK	0x0182604C
#define GCC_PCIE1_PHY_BCR	0x01876038
#define GCC_PCIE1PHY_PHY_BCR	0x0187603C
#define GCC_PCIE1_AXI_M_CBCR	0x01876008
#define GCC_PCIE1_AXI_S_CBCR	0x0187600C
#define GCC_PCIE1_AHB_CBCR	0x01876010
#define GCC_PCIE1_AUX_CBCR	0x01876014
#define GCC_PCIE1_PIPE_CBCR	0x01876018
#define GCC_PCIE1_AUX_CMD_RCGR	0x01876020
#define GCC_PCIE1_AXI_CMD_RCGR	0x01876050
#define GCC_PCIE1_AXI_CFG_RCGR	0x01876058

#define GCC_USB0_GDSCR			0x183E078
#define GCC_SYS_NOC_USB0_AXI_CBCR	0x1826040
#define GCC_SNOC_BUS_TIMEOUT2_AHB_CBCR	0x184700C
#define GCC_USB0_MASTER_CFG_RCGR	0x183E010
#define GCC_USB0_MASTER_CMD_RCGR	0x183E00C
#define GCC_USB0_MASTER_CBCR		0x183E000
#define GCC_USB0_SLEEP_CBCR		0x183E004
#define GCC_USB0_MOCK_UTMI_CFG_RCGR	0x0183E024
#define GCC_USB0_MOCK_UTMI_M		0x0183E028
#define GCC_USB0_MOCK_UTMI_N		0x0183E02C
#define GCC_USB0_MOCK_UTMI_D		0x0183E030
#define GCC_USB0_MOCK_UTMI_CMD_RCGR	0x183E020
#define GCC_USB0_MOCK_UTMI_CBCR		0x183E008
#define GCC_USB0_PHY_CFG_AHB_CBCR	0x183E080
#define GCC_USB0_AUX_CBCR		0x183E044
#define GCC_USB_SS_REF_CLK_EN		0x0183E07C
#define GCC_USB_0_BOOT_CLOCK_CTL	0x1840000
#define GCC_USB0_BCR			0x183E070
#define GCC_QUSB2_0_PHY_BCR		0x184103C
#define GCC_USB0_PHY_BCR		0x183E034
#define GCC_USB3PHY_0_PHY_BCR		0x183E03C
#define USB30_1_GENERAL_CFG		0x8AF8808
#define USB30_1_GUCTL			0x8A0C12C
#define USB30_PHY_1_QUSB2PHY_BASE	0x79000
#define GCC_USB0_PIPE_CBCR		0x183E040

#define GCC_USB1_GDSCR			0x183F078
#define GCC_SYS_NOC_USB1_AXI_CBCR	0x1826044
#define GCC_SNOC_BUS_TIMEOUT3_AHB_CBCR	0x01847014
#define GCC_USB1_MASTER_CFG_RCGR	0x183F010
#define GCC_USB1_MASTER_CMD_RCGR	0x183F00C
#define GCC_USB1_MASTER_CBCR		0x183F000
#define GCC_USB1_SLEEP_CBCR		0x183F004
#define GCC_USB1_MOCK_UTMI_CFG_RCGR	0x183F024
#define GCC_USB1_MOCK_UTMI_M		0x183F028
#define GCC_USB1_MOCK_UTMI_N		0x0183F02C
#define GCC_USB1_MOCK_UTMI_D		0x0183F030
#define GCC_USB1_MOCK_UTMI_CMD_RCGR	0x183F020
#define GCC_USB1_MOCK_UTMI_CBCR		0x183F008
#define GCC_USB1_PHY_CFG_AHB_CBCR	0x183F080
#define GCC_USB1_AUX_CBCR		0x183F044
#define GCC_USB_1_BOOT_CLOCK_CTL	0x1841000
#define GCC_USB1_BCR			0x183F070
#define GCC_QUSB2_1_PHY_BCR		0x1841040
#define GCC_USB1_PHY_BCR		0x183F034
#define GCC_USB3PHY_1_PHY_BCR		0x183F03C
#define USB30_2_GENERAL_CFG		0x8CF8808
#define USB30_2_GUCTL			0x8C0C12C
#define USB30_PHY_2_QUSB2PHY_BASE	0x59000
#define GCC_USB1_PIPE_CBCR		0x183F040

#define KERNEL_AUTH_CMD                0x13

#define USB0_SSPHY_BASE 0x78000
#define USB1_SSPHY_BASE 0x58000

#define USB3_PHY_POWER_DOWN_CONTROL	0x804
#define QSERDES_COM_SYSCLK_EN_SEL	0xac
#define QSERDES_COM_BIAS_EN_CLKBUFLR_EN	0x34
#define QSERDES_COM_CLK_SELECT		0x174
#define QSERDES_COM_BG_TRIM		0x70
#define QSERDES_COM_SVS_MODE_CLK_SEL	0x19c
#define QSERDES_COM_HSCLK_SEL		0x178
#define QSERDES_COM_CMN_CONFIG		0x194
#define QSERDES_COM_PLL_IVCO		0x048
#define QSERDES_COM_SYS_CLK_CTRL	0x3c
#define QSERDES_COM_DEC_START_MODE0	0xd0
#define QSERDES_COM_DIV_FRAC_START1_MODE0	0xdc
#define QSERDES_COM_DIV_FRAC_START2_MODE0	0xe0
#define QSERDES_COM_DIV_FRAC_START3_MODE0	0xe4
#define QSERDES_COM_CP_CTRL_MODE0		0x78
#define QSERDES_COM_PLL_RCTRL_MODE0		0x84
#define QSERDES_COM_PLL_CCTRL_MODE0		0x90
#define QSERDES_COM_INTEGLOOP_GAIN0_MODE0	0x108
#define QSERDES_COM_LOCK_CMP1_MODE0		0x4c
#define QSERDES_COM_LOCK_CMP2_MODE0		0x50
#define QSERDES_COM_LOCK_CMP3_MODE0		0x54
#define QSERDES_COM_CORE_CLK_EN			0x18c
#define QSERDES_COM_LOCK_CMP_CFG		0xcc
#define QSERDES_COM_VCO_TUNE_MAP		0x128
#define QSERDES_COM_BG_TIMER			0x0c
#define QSERDES_COM_SSC_EN_CENTER		0x10
#define QSERDES_COM_SSC_PER1			0x1c
#define QSERDES_COM_SSC_PER2			0x20
#define QSERDES_COM_SSC_ADJ_PER1		0x14
#define QSERDES_COM_SSC_ADJ_PER2		0x18
#define QSERDES_COM_SSC_STEP_SIZE1		0x24
#define QSERDES_COM_SSC_STEP_SIZE2		0x28
#define QSERDES_RX_UCDR_SO_GAIN			0x410
#define QSERDES_RX_RX_EQU_ADAPTOR_CNTRL2	0x4d8
#define QSERDES_RX_RX_EQU_ADAPTOR_CNTRL3	0x4dc
#define QSERDES_RX_RX_EQU_ADAPTOR_CNTRL4	0x4e0
#define QSERDES_RX_RX_EQ_OFFSET_ADAPTOR_CNTRL	0x508
#define QSERDES_RX_RX_OFFSET_ADAPTOR_CNTRL2	0x50c
#define QSERDES_RX_SIGDET_CNTRL			0x514
#define QSERDES_RX_SIGDET_DEGLITCH_CNTRL	0x51c
#define QSERDES_RX_SIGDET_ENABLES		0x510
#define QSERDES_TX_HIGHZ_TRANSCEIVEREN_BIAS_D	0x268
#define QSERDES_TX_RCV_DETECT_LVL_2		0x2ac
#define QSERDES_TX_LANE_MODE			0x294
#define PCS_TXDEEMPH_M6DB_V0			0x824
#define PCS_TXDEEMPH_M3P5DB_V0			0x828
#define PCS_FLL_CNTRL2				0x8c8
#define PCS_FLL_CNTRL1				0x8c4
#define PCS_FLL_CNT_VAL_L			0x8cc
#define PCS_FLL_CNT_VAL_H_TOL			0x8d0
#define PCS_FLL_MAN_CODE			0x8d4
#define PCS_LOCK_DETECT_CONFIG1			0x880
#define PCS_LOCK_DETECT_CONFIG2			0x884
#define PCS_LOCK_DETECT_CONFIG3			0x888
#define PCS_POWER_STATE_CONFIG2			0x864
#define PCS_RXEQTRAINING_WAIT_TIME		0x8b8
#define PCS_RXEQTRAINING_RUN_TIME		0x8bc
#define PCS_LFPS_TX_ECSTART_EQTLOCK		0x8b0
#define PCS_PWRUP_RESET_DLY_TIME_AUXCLK		0x8a0
#define PCS_TSYNC_RSYNC_TIME			0x88c
#define PCS_RCVR_DTCT_DLY_P1U2_L		0x870
#define PCS_RCVR_DTCT_DLY_P1U2_H		0x874
#define PCS_RCVR_DTCT_DLY_U3_L			0x878
#define PCS_RCVR_DTCT_DLY_U3_H			0x87c
#define PCS_RX_SIGDET_LVL			0x9d8
#define USB3_PHY_START_CONTROL			0x808
#define USB3_PHY_SW_RESET			0x800
#define QSERDES_RX_UCDR_FASTLOCK_FO_GAIN	0x440
#define QSERDES_RX_RX_EQU_ADAPTOR_CNTRL3	0x4dc
#define QSERDES_RX_SIGDET_ENABLES		0x510
#define USB3_PCS_TXDEEMPH_M6DB_V0		0x824
#define USB3_PCS_TXDEEMPH_M3P5DB_V0		0x828

typedef enum {
	SMEM_SPINLOCK_ARRAY = 7,
	SMEM_AARM_PARTITION_TABLE = 9,
	SMEM_HW_SW_BUILD_ID = 137,
	SMEM_USABLE_RAM_PARTITION_TABLE = 402,
	SMEM_POWER_ON_STATUS_INFO = 403,
	SMEM_MACHID_INFO_LOCATION = 425,
	SMEM_IMAGE_VERSION_TABLE = 469,
	SMEM_BOOT_FLASH_TYPE = 498,
	SMEM_BOOT_FLASH_INDEX = 499,
	SMEM_BOOT_FLASH_CHIP_SELECT = 500,
	SMEM_BOOT_FLASH_BLOCK_SIZE = 501,
	SMEM_BOOT_FLASH_DENSITY = 502,
	SMEM_BOOT_DUALPARTINFO = 503,
	SMEM_PARTITION_TABLE_OFFSET = 504,
	SMEM_SPI_FLASH_ADDR_LEN = 505,
	SMEM_FIRST_VALID_TYPE = SMEM_SPINLOCK_ARRAY,
	SMEM_LAST_VALID_TYPE = SMEM_SPI_FLASH_ADDR_LEN,
	SMEM_MAX_SIZE = SMEM_SPI_FLASH_ADDR_LEN + 1,
} smem_mem_type_t;

/* Reserved-memory node names*/
extern const char *rsvd_node;
extern const char *del_node[];
extern const add_node_t add_node[];

void reset_crashdump(void);
#ifdef CONFIG_PCI_IPQ
void board_pci_init(int id);
__weak void board_pcie_clock_init(int id) {}
#endif
void ipq_fdt_fixup_socinfo(void *blob);
int ipq_board_usb_init(void);

__weak int ipq_get_tz_version(char *version_name, int buf_size)
{
	return 1;
}

#endif /* _IPQ807X_H_ */
