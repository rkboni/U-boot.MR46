/*
 *
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _IPQ40XX_CDP_H_
#define _IPQ40XX_CDP_H_

#include <asm/mach-types.h>
#include <configs/ipq40xx_cdp.h>
#include <asm/u-boot.h>
#include <phy.h>
#include "qca_common.h"

#define NO_OF_DBG_UART_GPIOS	2
#ifdef CONFIG_IPQ40XX_I2C
#define NO_OF_I2C_GPIOS		2
#endif
#define MAX_CONF_NAME		5

#define KERNEL_AUTH_CMD		0x13

#define IPQ40XX_EDMA_DEV	1
typedef struct {
	uint count;
	u8 addr[7];
} ipq40xx_edma_phy_addr_t;

/* ipq40xx edma Paramaters */
typedef struct {
	uint base;
	int unit;
	uint mac_conn_to_phy;
	phy_interface_t phy;
	ipq40xx_edma_phy_addr_t phy_addr;
	const char phy_name[MDIO_NAME_LEN];
} ipq40xx_edma_board_cfg_t;

typedef struct {
	int gpio;
	unsigned int func;
	unsigned int out;
	unsigned int pull;
	unsigned int drvstr;
	unsigned int oe;
	unsigned int gpio_vm;
	unsigned int gpio_od_en;
	unsigned int gpio_pu_res;
} gpio_func_data_t;

typedef struct {
	unsigned int uart_dm_base;
	gpio_func_data_t *dbg_uart_gpio;
} uart_cfg_t;

#ifdef CONFIG_IPQ40XX_I2C
typedef struct {
	unsigned int i2c_base;
	gpio_func_data_t *i2c_gpio;
} i2c_cfg_t;
#endif

#ifdef CONFIG_IPQ40XX_PCI
#define PCI_MAX_DEVICES	1

typedef struct {
	gpio_func_data_t	*pci_gpio;
	uint32_t		pci_gpio_count;
	uint32_t		parf;
	uint32_t		elbi;
	uint32_t		pcie20;
	uint32_t		axi_bar_start;
	uint32_t		axi_bar_size;
	uint32_t		pcie_rst;
	uint32_t		axi_conf;
	int			linkup;
} pcie_params_t;

void board_pci_init(void);
#endif /* CONFIG_IPQ40XX_PCI */

/* Board specific parameters */
typedef struct {
	unsigned int machid;
	unsigned int ddr_size;
	const char *mtdids;
	gpio_func_data_t *spi_nor_gpio;
	unsigned int spi_nor_gpio_count;
	gpio_func_data_t *nand_gpio;
	unsigned int nand_gpio_count;
	gpio_func_data_t *sw_gpio;
	unsigned int sw_gpio_count;
	gpio_func_data_t *rgmii_gpio;
	unsigned int rgmii_gpio_count;
	ipq40xx_edma_board_cfg_t edma_cfg[IPQ40XX_EDMA_DEV];
	uart_cfg_t *uart_cfg;
	uart_cfg_t *console_uart_cfg;
#ifdef CONFIG_IPQ40XX_I2C
	i2c_cfg_t *i2c_cfg;
#endif
	gpio_func_data_t *mmc_gpio;
	unsigned int mmc_gpio_count;
	unsigned int spi_nand_available;
	unsigned int nor_nand_available;
	unsigned int nor_emmc_available;
#ifdef CONFIG_IPQ40XX_PCI
	pcie_params_t pcie_cfg[PCI_MAX_DEVICES];
#endif
	const char *dtb_config_name[MAX_CONF_NAME];
} __attribute__ ((__packed__)) board_ipq40xx_params_t;

typedef enum {
	SMEM_SPINLOCK_ARRAY = 7,
	SMEM_AARM_PARTITION_TABLE = 9,
	SMEM_HW_SW_BUILD_ID = 137,
	SMEM_USABLE_RAM_PARTITION_TABLE = 402,
	SMEM_POWER_ON_STATUS_INFO = 403,
	SMEM_MACHID_INFO_LOCATION = 425,
	SMEM_IMAGE_VERSION_TABLE = 469,
	SMEM_BOOT_FLASH_TYPE = 478,
	SMEM_BOOT_FLASH_INDEX = 479,
	SMEM_BOOT_FLASH_CHIP_SELECT = 480,
	SMEM_BOOT_FLASH_BLOCK_SIZE = 481,
	SMEM_BOOT_FLASH_DENSITY = 482,
	SMEM_PARTITION_TABLE_OFFSET = 483,
	SMEM_BOOT_DUALPARTINFO = 484,
	SMEM_FIRST_VALID_TYPE = SMEM_SPINLOCK_ARRAY,
	SMEM_LAST_VALID_TYPE = SMEM_BOOT_DUALPARTINFO,
	SMEM_MAX_SIZE = SMEM_PARTITION_TABLE_OFFSET + 1,
} smem_mem_type_t;


extern board_ipq40xx_params_t *gboard_param;
unsigned int get_board_index(unsigned int machid);
void qca_configure_gpio(gpio_func_data_t *gpio, uint count);


#define MSM_SDC1_BASE      0x7824000
extern qca_mmc mmc_host;
#endif
