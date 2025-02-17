/*
 * Copyright (c) 2012-2014 The Linux Foundation. All rights reserved.
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


#ifndef  _IPQ806X_CDP_H_
#define  _IPQ806X_CDP_H_

#include <asm/mach-types.h>
#include <configs/ipq806x_cdp.h>
#include <phy.h>
#include <asm/arch-ipq806x/clock.h>
#include <asm/u-boot.h>

#define TZBSP_BUILD_VER_QUERY_CMD      0x4
#define KERNEL_AUTH_CMD                 0x7

typedef struct {
	int gpio;
	unsigned int func;
	unsigned int out;
	unsigned int pull;
	unsigned int drvstr;
	unsigned int oe;
} gpio_func_data_t;

typedef struct {
	unsigned int m_value;
	unsigned int n_value;
	unsigned int d_value;
} clk_mnd_t;

/* SPI Mode */

typedef enum {
	NOR_SPI_MODE_0,
	NOR_SPI_MODE_1,
	NOR_SPI_MODE_2,
	NOR_SPI_MODE_3,
} spi_mode;

/* SPI GSBI Bus number */

typedef enum {
	GSBI_BUS_5 = 0,
	GSBI_BUS_6,
	GSBI_BUS_7,
} spi_gsbi_bus_num;

/* SPI Chip selects */

typedef enum {
	SPI_CS_0 ,
	SPI_CS_1,
	SPI_CS_2,
	SPI_CS_3,
} spi_cs;

/* Flash Types */

typedef enum {
	ONLY_NAND,
	ONLY_NOR,
	NAND_NOR,
	NOR_MMC,
} flash_desc;

#define NO_OF_DBG_UART_GPIOS	2
#define NO_OF_I2C_GPIOS     	2

#define SPI_NOR_FLASH_VENDOR_MICRON       0x1
#define SPI_NOR_FLASH_VENDOR_SPANSION     0x2

/* SPI parameters */

typedef struct {
	spi_mode mode;
	spi_gsbi_bus_num bus_number;
	spi_cs chip_select;
	int vendor;
} spinorflash_params_t;

typedef struct {
	uint count;
	u8 addr[7];
} ipq_gmac_phy_addr_t;

typedef struct {
	uint base;
	int unit;
	uint is_macsec;
	uint mac_pwr0;
	uint mac_pwr1;
	uint mac_conn_to_phy;
	phy_interface_t phy;
	ipq_gmac_phy_addr_t phy_addr;
	const char phy_name[MDIO_NAME_LEN];
} ipq_gmac_board_cfg_t;

#define IPQ_GMAC_NMACS		4

#ifdef CONFIG_IPQ806X_PCI
#define PCI_MAX_DEVICES		3

typedef struct {
	gpio_func_data_t 		*pci_rst_gpio;
	uint32_t			parf;
	uint32_t			elbi;
	uint32_t			pcie20;
	uint32_t			axi_bar_start;
	uint32_t			axi_bar_size;
	uint32_t			pcie_rst;
	clk_offset_t			*pci_clks;
	uint32_t			axi_conf;
	int 				linkup;
} pcie_params_t;

void board_pci_init(void);
#endif /* CONFIG_IPQ806X_PCI */

#define MAX_SOC_CONFIGS			10
/* Board specific parameters */
typedef struct {
	unsigned int machid;
	unsigned int ddr_size;
	unsigned int uart_gsbi;
	unsigned int uart_gsbi_base;
	unsigned int uart_dm_base;
	unsigned int clk_dummy;
	clk_mnd_t uart_mnd_value;
	clk_mnd_t usb_core_mnd_value;
	clk_mnd_t usb_utmi_mnd_value;
	unsigned int gmac_gpio_count;
	gpio_func_data_t *gmac_gpio;
	ipq_gmac_board_cfg_t gmac_cfg[IPQ_GMAC_NMACS];
	gpio_func_data_t *switch_gpio;
	flash_desc flashdesc;
	spinorflash_params_t flash_param;
	gpio_func_data_t dbg_uart_gpio[NO_OF_DBG_UART_GPIOS];
#ifdef CONFIG_IPQ806X_I2C
	unsigned int i2c_gsbi;
	unsigned int i2c_gsbi_base;
	clk_mnd_t i2c_mnd_value;
	gpio_func_data_t i2c_gpio[NO_OF_I2C_GPIOS];
#endif
#ifdef CONFIG_IPQ806X_PCI
	pcie_params_t	pcie_cfg[PCI_MAX_DEVICES];
#endif
#ifdef CONFIG_IPQ_MMC
	gpio_func_data_t *emmc_gpio;
	unsigned int emmc_gpio_count;
#endif
	unsigned char *dtb_config_name[MAX_SOC_CONFIGS];
} __attribute__ ((__packed__)) board_ipq806x_params_t;

extern board_ipq806x_params_t *gboard_param;

typedef enum {
	SMEM_SPINLOCK_ARRAY = 7,
	SMEM_AARM_PARTITION_TABLE = 9,
	SMEM_APPS_BOOT_MODE = 106,
	SMEM_HW_SW_BUILD_ID = 137,
	SMEM_USABLE_RAM_PARTITION_TABLE = 402,
	SMEM_POWER_ON_STATUS_INFO = 403,
	SMEM_RLOCK_AREA = 404,
	SMEM_BOOT_INFO_FOR_APPS = 418,
	SMEM_BOOT_FLASH_TYPE = 421,
	SMEM_BOOT_FLASH_INDEX = 422,
	SMEM_BOOT_FLASH_CHIP_SELECT = 423,
	SMEM_BOOT_FLASH_BLOCK_SIZE = 424,
	SMEM_MACHID_INFO_LOCATION = 425,
	SMEM_BOOT_DUALPARTINFO = 427,
	SMEM_PARTITION_TABLE_OFFSET = 428,
	SMEM_BOOT_FLASH_DENSITY = 429,
	SMEM_IMAGE_VERSION_TABLE = 430,
	SMEM_FIRST_VALID_TYPE = SMEM_SPINLOCK_ARRAY,
	SMEM_LAST_VALID_TYPE = SMEM_IMAGE_VERSION_TABLE,
	SMEM_MAX_SIZE = SMEM_PARTITION_TABLE_OFFSET + 1,
} smem_mem_type_t;

static inline int gmac_cfg_is_valid(ipq_gmac_board_cfg_t *cfg)
{
	/*
	 * 'cfg' is valid if and only if
	 *	unit number is non-negative and less than IPQ_GMAC_NMACS.
	 *	'cfg' pointer lies within the array range of
	 *		board_ipq806x_params_t->gmac_cfg[]
	 */
	return ((cfg >= &gboard_param->gmac_cfg[0]) &&
		(cfg < &gboard_param->gmac_cfg[IPQ_GMAC_NMACS]) &&
		(cfg->unit >= 0) && (cfg->unit < IPQ_GMAC_NMACS));
}

unsigned int get_board_index(unsigned int machid);
void ipq_configure_gpio(gpio_func_data_t *gpio, uint count);

#ifdef CONFIG_IPQ_MMC

#define MSM_SDC1_BASE      0x12400000

typedef struct {
	uint base;
	uint clk_mode;
	struct mmc *mmc;
	int dev_num;
} ipq_mmc;

extern ipq_mmc mmc_host;
int ipq_mmc_init(bd_t *, ipq_mmc *);
void board_mmc_deinit(void);
#endif
#endif
