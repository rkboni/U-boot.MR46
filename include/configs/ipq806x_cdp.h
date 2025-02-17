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

#ifndef _IPQCDP_H
#define _IPQCDP_H

#include <linux/sizes.h>

/*
 * Disabled for actual chip.
 * #define CONFIG_RUMI
 */
#if !defined(DO_DEPS_ONLY) || defined(DO_SOC_DEPS_ONLY)
/*
 * Beat the system! tools/scripts/make-asm-offsets uses
 * the following hard-coded define for both u-boot's
 * ASM offsets and platform specific ASM offsets :(
 */
#include <generated/generic-asm-offsets.h>
#if !defined(DO_SOC_DEPS_ONLY)
#include <generated/asm-offsets.h>
#endif
#endif /* !DO_DEPS_ONLY */

#define CONFIG_BOARD_EARLY_INIT_F

#define CONFIG_SYS_CACHELINE_SIZE   64
#define CONFIG_IPQ_FDT_HIGH     0xffffffff

#undef CONFIG_CMD_FLASH
#undef CONFIG_CMD_FPGA		        /* FPGA configuration support */
#undef CONFIG_CMD_IMI
#undef CONFIG_CMD_IMLS
#undef CONFIG_CMD_NFS		        /* NFS support */
#undef CONFIG_IPQ_ETH
#undef CONFIG_SYS_MAX_FLASH_SECT
#define CONFIG_NR_DRAM_BANKS            1
#define CONFIG_SKIP_LOWLEVEL_INIT

#define CONFIG_BOARD_EARLY_INIT_F

#define CONFIG_HW_WATCHDOG

#define CONFIG_ARCH_CPU_INIT

#define CONFIG_SYS_MALLOC_LEN           (CONFIG_ENV_SIZE_MAX + (256 << 10))

/*
 * Size of malloc() pool
 */

/*
 * select serial console configuration
 */
#define CONFIG_CONS_INDEX               1

/*
 * Enable crash dump support, this will dump the memory
 * regions via TFTP in case magic word found in memory
 */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_BAUDRATE                 115200
#define CONFIG_SYS_BAUDRATE_TABLE       {4800, 9600, 19200, 38400, 57600,\
								115200}

#define V_PROMPT                        "(IPQ) # "
#define CONFIG_SYS_CBSIZE               (512 * 2) /* Console I/O Buffer Size */

#define CONFIG_SYS_INIT_SP_ADDR         CONFIG_SYS_SDRAM_BASE + GENERATED_IPQ_RESERVE_SIZE - GENERATED_GBL_DATA_SIZE
#define CONFIG_SYS_MAXARGS              16
#define CONFIG_SYS_PBSIZE               (CONFIG_SYS_CBSIZE + \
						sizeof(CONFIG_SYS_PROMPT) + 16)

#define TLMM_BASE_ADDR      0x00800000
#define GPIO_CONFIG_ADDR(x) (TLMM_BASE_ADDR + 0x1000 + (x)*0x10)
#define GPIO_IN_OUT_ADDR(x) (TLMM_BASE_ADDR + 0x1004 + (x)*0x10)

#undef CONFIG_IPQ_ROOTFS_AUTH

#define CONFIG_SYS_TEXT_BASE        0x42000000
#define CONFIG_SYS_SDRAM_BASE           0x40000000
#define CONFIG_SYS_SDRAM_SIZE           0x10000000
#define CONFIG_MAX_RAM_BANK_SIZE        CONFIG_SYS_SDRAM_SIZE
#define CONFIG_SYS_LOAD_ADDR            (CONFIG_SYS_SDRAM_BASE + (64 << 20))

/*
 * I2C Configs
 */
#define CONFIG_IPQ806X_I2C             1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 10

#ifdef CONFIG_IPQ806X_I2C
#define CONFIG_SYS_I2C_SPEED		0
#endif

#ifdef CONFIG_IPQ806X_PCI
#define CONFIG_PCI
#define CONFIG_CMD_PCI
#define CONFIG_PCI_SCAN_SHOW
#endif

#ifndef __ASSEMBLY__
#include <compiler.h>
#include "../../board/qca/arm/ipq806x/ipq806x_cdp.h"
extern loff_t board_env_offset;
extern loff_t board_env_range;
extern uint32_t flash_index;
extern uint32_t flash_chip_select;
extern uint32_t flash_block_size;
extern board_ipq806x_params_t *gboard_param;
extern int rootfs_part_avail;
#ifdef CONFIG_IPQ806X_PCI
void board_pci_deinit(void);
#endif

static uint32_t inline clk_is_dummy(void)
{
	return gboard_param->clk_dummy;
}

/*
 * XXX XXX Please do not instantiate this structure. XXX XXX
 * This is just a convenience to avoid
 *      - adding #defines for every new reservation
 *      - updating the multiple associated defines like smem base,
 *        kernel start etc...
 *      - re-calculation of the defines if the order changes or
 *        some reservations are deleted
 * For new reservations just adding a member to the structure should
 * suffice.
 * Ensure that the size of this structure matches with the definition
 * of the following IPQ806x compile time definitions
 *      PHYS_OFFSET     (linux-sources/arch/arm/mach-msm/Kconfig)
 *      zreladdr        (linux-sources/arch/arm/mach-msm/Makefile.boot)
 *      CONFIG_SYS_INIT_SP_ADDR defined above should point to the bottom.
 *      MSM_SHARED_RAM_PHYS (linux-sources/arch/arm/mach-msm/board-ipq806x.c)
 *
 */
#if !defined(DO_DEPS_ONLY) || defined(DO_SOC_DEPS_ONLY)
typedef struct {
	uint8_t	nss[16 * 1024 * 1024];
	uint8_t	smem[2 * 1024 * 1024];
	uint8_t	uboot[1 * 1024 * 1024];
	uint8_t	nsstcmdump[128 * 1024];
	uint8_t sbl3[384 * 1024];
	uint8_t plcfwdump[512*1024];
	uint8_t wlanfwdump[(1 * 1024 * 1024) - GENERATED_GBL_DATA_SIZE];
	uint8_t init_stack[GENERATED_GBL_DATA_SIZE];
} __attribute__ ((__packed__)) ipq_mem_reserve_t;

/* Convenience macros for the above convenience structure :-) */
#define IPQ_MEM_RESERVE_SIZE(x)		sizeof(((ipq_mem_reserve_t *)0)->x)
#define IPQ_MEM_RESERVE_BASE(x)		\
	(CONFIG_SYS_SDRAM_BASE + \
	 ((uint32_t)&(((ipq_mem_reserve_t *)0)->x)))
#endif

#define IPQ_KERNEL_START_ADDR	\
	(CONFIG_SYS_SDRAM_BASE + GENERATED_IPQ_RESERVE_SIZE)

#define IPQ_DRAM_KERNEL_SIZE	\
	(CONFIG_SYS_SDRAM_SIZE - GENERATED_IPQ_RESERVE_SIZE)

#define IPQ_BOOT_PARAMS_ADDR		(IPQ_KERNEL_START_ADDR + 0x100)

#define IPQ_NSSTCM_DUMP_ADDR		(IPQ_MEM_RESERVE_BASE(nsstcmdump))

#define IPQ_TEMP_DUMP_ADDR		(IPQ_MEM_RESERVE_BASE(nsstcmdump))
#define IPQ_TFTP_MIN_ADDR       (CONFIG_SYS_SDRAM_BASE + (32 << 20))
#define IPQ_TFTP_MAX_ADDR       (gd->bd->bi_dram[0].start + \
								 gd->bd->bi_dram[0].size)
#define IPQ_PANIC_DUMP_SIZE		0x40000
#define IPQ_PANIC_DUMP_ADDR		0x5ffC0000

#endif /* __ASSEMBLY__ */

#define CONFIG_SYS_MEMTEST_START        CONFIG_SYS_SDRAM_BASE + 0x1300000
#define CONFIG_SYS_MEMTEST_END          CONFIG_SYS_MEMTEST_START + 0x100

#define CONFIG_CMDLINE_TAG	 1	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS 1

#define CONFIG_CMD_IMI

#define CONFIG_CMD_SOURCE   1
#define CONFIG_INITRD_TAG   1

/*
 * SPI Flash Configs

#define CONFIG_IPQ_SPI
#define CONFIG_SPI_FLASH
#define CONFIG_CMD_SF
#define CONFIG_SPI_FLASH_STMICRO
#define CONFIG_SPI_FLASH_SPANSION
#define CONFIG_SPI_FLASH_MACRONIX
#define CONFIG_SPI_FLASH_WINBOND
#define CONFIG_SYS_HZ                   1000

#define CONFIG_SF_DEFAULT_BUS 0
#define CONFIG_SF_DEFAULT_CS 0
#define CONFIG_SF_DEFAULT_MODE SPI_MODE_0
 */

/*
 * NAND Flash Configs
 */

#define CONFIG_SYS_NAND_ONFI_DETECTION

/*
 * Expose SPI driver as a pseudo NAND driver to make use
 * of U-Boot's MTD framework.
 */
#define CONFIG_SYS_MAX_NAND_DEVICE	(CONFIG_IPQ_MAX_NAND_DEVICE + \
					 CONFIG_IPQ_MAX_SPI_DEVICE)

/*
 * U-Boot Env Configs
 */
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_BOARD_LATE_INIT

#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1

#if defined(CONFIG_ENV_IS_IN_NAND) || defined(CONFIG_ENV_IS_IN_MMC)

#define CONFIG_ENV_SPI_CS               flash_chip_select
#define CONFIG_ENV_SPI_MODE             SPI_MODE_0
#define CONFIG_ENV_OFFSET               board_env_offset
#define CONFIG_ENV_SECT_SIZE            flash_block_size
#define CONFIG_ENV_SPI_BUS              flash_index
#endif
#define CONFIG_ENV_RANGE		        CONFIG_ENV_SIZE_MAX
#define CONFIG_ENV_SIZE                 CONFIG_ENV_RANGE

#define CONFIG_BOOTCOMMAND  "run meraki_bootkernel2; run meraki_bootkernel1"
#define CONFIG_EXTRA_ENV_SETTINGS    \
       "itb_config=config@1\0" \
       "loadaddr=0x42000000\0" \
       "signed_imgaddr=0x42000028\0" \
       "tzaddr=0x44000000\0" \
       "fuseaddr=0x44000000\0" \
       "imgaddr=0x44000028\0" \
       "set_bootargs=setenv bootargs loader=u-boot maxcpus=1\0" \
       "set_ubi=setenv mtdids nand0=nand0; setenv mtdparts mtdparts=nand0:0x46c0000@0x36c0000(ubi); ubi part ubi\0" \
       "part=part.safe\0" \
       "boot_part=ubi read $loadaddr $part; bootm $loadaddr#$itb_config\0" \
       "boot_signedpart=ubi read $tzaddr $part; validate $tzaddr && cp.b $tzaddr $loadaddr $mbnsize && bootm $signed_imgaddr#$itb_config\0" \
       "meraki_ubi_boot=run set_bootargs; run set_ubi; boot_meraki_qca;\0" \
       "uboot_loadaddr=0x41200000\0" \
       "chainload_uboot=if ubi check uboot.dev; then ubi read $uboot_loadaddr uboot.dev; chainload $uboot_loadaddr; fi\0" \
       "check_boot_diag_img=if ubi check boot_diagnostic; then ubi remove boot_diagnostic; ubi read $tzaddr diagnostic1; validate $tzaddr && bootm $imgaddr; fi\0" \
       "scrloadaddr=0x41000000\0" \
       "scrname=boot.scr\0" \
       "check_load_boot_scr=if ubifsls $scrname; then ubifsload $scrloadaddr $scrname; bootcustom $scrloadaddr; fi\0" \
       "check_boot_part_dev=if ubi check part.dev; then setenv part part.dev; run boot_part; fi\0" \
       "check_dev_unlock=ubi read $tzaddr dev.crt; validate $tzaddr\0" \
       "meraki_bootkernel_boot=run meraki_bootkernel2; run meraki_bootkernel1\0" \
       "meraki_bootkernel2=nand read 0x42000000 0x02c40000 0x00a80000; bootbk 0x42000000 bootkernel2 $itb_config\0" \
       "meraki_bootkernel1=nand read 0x42000000 0x021c0000 0x00a80000; bootbk 0x42000000 bootkernel1 $itb_config\0"


#define CONFIG_AUTOBOOT_STOP_STR "xyzzy"

#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEBUG
#define CONFIG_MTD_DEBUG_VERBOSE 0
#define NUM_ALT_PARTITION		8

#define CONFIG_RBTREE		/* for ubi */
#define CONFIG_LZO

#define CONFIG_CMD_BOOTZ
#define CONFIG_IMAGE_FORMAT_LEGACY
#define CONFIG_SYS_BOOTM_LEN		SZ_32M

#define CONFIG_MII
#define CONFIG_CMD_MII

#define CONFIG_SYS_RX_ETH_BUFFER	8

#ifdef CONFIG_IPQ_APPSBL_DLOAD
#define CONFIG_CMD_TFTPPUT
/* We will be uploading very big files */
#define CONFIG_NET_RETRY_COUNT 500
#endif
/* L1 cache line size is 64 bytes, L2 cache line size is 128 bytes
 * Cache flush and invalidation based on L1 cache, so the cache line
 * size is configured to 64 */
#define CONFIG_SYS_CACHELINE_SIZE  64
/*#define CONFIG_SYS_DCACHE_OFF*/

/* Enabling this flag will report any L2 errors.
 * By default we are disabling it */
/*#define CONFIG_IPQ_REPORT_L2ERR*/

#endif /* _IPQCDP_H */

