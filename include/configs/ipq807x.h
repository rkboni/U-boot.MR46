/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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

#ifndef DO_DEPS_ONLY
#include <generated/asm-offsets.h>
#endif

/*
 * Disabled for actual chip.
 * #define CONFIG_RUMI
 */

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_BOARD_LATE_INIT
#define CONFIG_SYS_CACHELINE_SIZE   64

#define CONFIG_NR_DRAM_BANKS            1
#define CONFIG_SKIP_LOWLEVEL_INIT

#define CONFIG_SYS_BOOTM_LEN            (64 << 20)
#define HAVE_BLOCK_DEVICE
/*
 * Size of malloc() pool
 */

/*
 * select serial console configuration
 */
#define CONFIG_CONS_INDEX               1
#define CONFIG_SYS_DEVICE_NULLDEV

/* allow to overwrite serial and ethaddr */
#define CONFIG_BAUDRATE                 115200
#define CONFIG_SYS_BAUDRATE_TABLE       {4800, 9600, 19200, 38400, 57600,\
								115200}

#define CONFIG_SYS_CBSIZE               (512 * 2) /* Console I/O Buffer Size */

/*

          svc_sp     --> --------------
          irq_sp     --> |            |
	  fiq_sp     --> |            |
	  bd         --> |            |
          gd         --> |            |
          pgt        --> |            |
          malloc     --> |            |
          text_base  --> |------------|
*/

#ifdef CONFIG_IPQ807x_I2C
#define CONFIG_DM_I2C
#endif

#define CONFIG_SYS_INIT_SP_ADDR 	(CONFIG_SYS_TEXT_BASE -\
			CONFIG_SYS_MALLOC_LEN - CONFIG_ENV_SIZE -\
			GENERATED_BD_INFO_SIZE)

#define CONFIG_SYS_MAXARGS              16
#define CONFIG_SYS_PBSIZE               (CONFIG_SYS_CBSIZE + \
						sizeof(CONFIG_SYS_PROMPT) + 16)

#define TLMM_BASE			0x01000000
#define GPIO_CONFIG_ADDR(x)		(TLMM_BASE + (x)*0x1000)
#define GPIO_IN_OUT_ADDR(x)		(TLMM_BASE + 0x4 + (x)*0x1000)

#define CONFIG_SYS_SDRAM_BASE           0x40000000
/*#define CONFIG_SYS_TEXT_BASE            0x4A900000*/
#define CONFIG_SYS_SDRAM_SIZE           0x10000000
#define CONFIG_MAX_RAM_BANK_SIZE        CONFIG_SYS_SDRAM_SIZE
#define CONFIG_SYS_LOAD_ADDR            (CONFIG_SYS_SDRAM_BASE + (64 << 20))

#define CONFIG_SYS_NAND_BASE        0

#define QCA_KERNEL_START_ADDR		CONFIG_SYS_SDRAM_BASE
#define QCA_DRAM_KERNEL_SIZE		CONFIG_SYS_SDRAM_SIZE
#define QCA_BOOT_PARAMS_ADDR		(QCA_KERNEL_START_ADDR + 0x100)

#define CONFIG_IPQ_NO_MACS		6
#define CONFIG_ENV_SECT_SIZE        	(64 * 1024)
/*
 * IPQ_TFTP_MIN_ADDR: Starting address of Linux HLOS region.
 * CONFIG_TZ_END_ADDR: Ending address of Trust Zone and starting
 * address of WLAN Area.
 * TFTP file can only be written in Linux HLOS region and WLAN AREA.
 */
#define IPQ_TFTP_MIN_ADDR		(CONFIG_SYS_SDRAM_BASE + (16 << 20))
#define IPQ_TZ_END_ADDR		(CONFIG_SYS_SDRAM_BASE + (88 << 21))

#ifndef __ASSEMBLY__
#include <compiler.h>
extern loff_t board_env_offset;
extern loff_t board_env_range;
extern loff_t board_env_size;
#endif

#define CONFIG_ENV_OFFSET		board_env_offset
#define CONFIG_ENV_RANGE	   CONFIG_ENV_SIZE_MAX
#define CONFIG_ENV_SIZE                 CONFIG_ENV_RANGE
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE_MAX + (1024 << 10))


/* Allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/*
 * MMC configs
 */
#undef CONFIG_QCA_MMC

/*
 * NAND Flash Configs
 */

/* CONFIG_QPIC_NAND: QPIC NAND in BAM mode
 * CONFIG_IPQ_NAND: QPIC NAND in FIFO/block mode.
 * BAM is enabled by default.
 */
/*#define CONFIG_SYS_NAND_SELF_INIT*/
#define CONFIG_SYS_NAND_ONFI_DETECTION

/*
 * Expose SPI driver as a pseudo NAND driver to make use
 * of U-Boot's MTD framework.
 */
#define CONFIG_SYS_MAX_NAND_DEVICE	CONFIG_IPQ_MAX_NAND_DEVICE + \
					CONFIG_IPQ_MAX_SPI_DEVICE

#define CONFIG_QPIC_NAND_NAND_INFO_IDX	0

#define QCA_SPI_NOR_DEVICE		"spi0.0"

/*
 * U-Boot Env Configs
 */
#define CONFIG_OF_LIBFDT	1

/* MTEST */
#define CONFIG_CMD_MEMTEST
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE + 0x1300000
#define CONFIG_SYS_MEMTEST_END		CONFIG_SYS_MEMTEST_START + 0x100

/* NSS firmware loaded using bootm */
#define CONFIG_BOOTARGS "console=ttyMSM0,115200n8"
#define QCA_ROOT_FS_PART_NAME "rootfs"

#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define NUM_ALT_PARTITION	16

#define CONFIG_CMD_BOOTZ

#define CONFIG_OF_BOARD_SETUP

#ifdef CONFIG_OF_BOARD_SETUP
#define DLOAD_DISABLE		0x1
#define RESERVE_ADDRESS_START	0x4AB00000 /*TZAPPS, SMEM and TZ Regions */
#define RESERVE_ADDRESS_SIZE	0x5500000

/*
 * Below Configs need to be updated after enabling reset_crashdump
 * Included now to avoid build failure
 */
#define SET_MAGIC				0x1
#define CLEAR_MAGIC				0x0
#define SCM_CMD_TZ_CONFIG_HW_FOR_RAM_DUMP_ID	0x9
#define SCM_CMD_TZ_FORCE_DLOAD_ID		0x10
#define BOOT_VERSION				0
#define TZ_VERSION				1
#define RPM_VERSION				3
#endif

#define CONFIG_FDT_FIXUP_PARTITIONS

/*
 * USB Support
 */
#undef CONFIG_USB_XHCI_IPQ

#undef CONFIG_PCI_IPQ
#define PCI_MAX_DEVICES	2
#ifdef CONFIG_PCI_IPQ
#define CONFIG_PCI
#define CONFIG_CMD_PCI
#define CONFIG_PCI_SCAN_SHOW
#endif

#define CONFIG_NET_RETRY_COUNT		5
#define CONFIG_SYS_RX_ETH_BUFFER	16
#define CONFIG_MII
#define CONFIG_CMD_MII
#define CONFIG_IPADDR	192.168.10.10
#define CONFIG_NETMASK	255.255.255.0
#define CONFIG_SERVERIP	192.168.10.1
#define CONFIG_CMD_TFTPPUT
#define CONFIG_IPQ_MDIO			1
#undef CONFIG_QCA8075_PHY

/*
 * CRASH DUMP ENABLE
 */
#ifdef CONFIG_QCA_APPSBL_DLOAD
#define CONFIG_CMD_TFTPPUT
/* We will be uploading very big files */
#undef CONFIG_NET_RETRY_COUNT
#define CONFIG_NET_RETRY_COUNT  500

#define IPQ_TEMP_DUMP_ADDR 0x44000000
#endif

/*#define CONFIG_QCA_KERNEL_CRASHDUMP_ADDRESS	*((unsigned int *)0x08600658)*/
#define CONFIG_CPU_CONTEXT_DUMP_SIZE		4096
/*#define CONFIG_TLV_DUMP_SIZE			2048*/

/* L1 cache line size is 64 bytes, L2 cache line size is 128 bytes
 * Cache flush and invalidation based on L1 cache, so the cache line
 * size is configured to 64 */
#define CONFIG_SYS_CACHELINE_SIZE  64
#define CONFIG_SYS_DCACHE_OFF

/* Enabling this flag will report any L2 errors.
 * By default we are disabling it */
/*#define CONFIG_IPQ_REPORT_L2ERR*/

/*
 * Other commands
 */

#define CONFIG_ARMV7_PSCI

#define CONFIG_SYS_MEM_TOP_HIDE 0

#define CONFIG_BOOTCOMMAND	"run meraki_boot"

#define CONFIG_REMAKE_ELF

#define CONFIG_TFTP_BLOCKSIZE 1400

#define CONFIG_EXTRA_ENV_SETTINGS    \
   "initrd_high=0xffffffff\0" \
   "fdt_high=0xffffffff\0" \
   "itb_config=config@1\0" \
   "loadaddr=0x50000000\0" \
   "imgaddr=0x50000028\0" \
   "fuseaddr=0x44000000\0" \
   "part=part.safe\0" \
   "set_bootargs=setenv bootargs loader=u-boot\0" \
   "set_ubi=setenv mtdids nand0=nand0; setenv mtdparts mtdparts=nand0:0xED80000@0x1280000(ubi); ubi part ubi\0" \
   "boot_part=ubi read $loadaddr $part; bootm $loadaddr#$itb_config\0" \
   "boot_signedpart=ubi read $loadaddr $part; validate $loadaddr && bootm $imgaddr#$itb_config\0" \
   "bootkernel1=setenv part part.safe; run boot_part\0" \
   "bootkernel2=setenv part part.old; run boot_part\0" \
   "meraki_boot=run set_bootargs; run set_ubi; boot_meraki_qca; run bootkernel1; run bootkernel2\0" \
   "uboot_loadaddr=0x4A900000\0" \
   "chainload_uboot=if ubi check uboot.dev; then ubi read $uboot_loadaddr uboot.dev; chainload $uboot_loadaddr; fi\0" \
   "check_boot_diag_img=if ubi check boot_diagnostic; then ubi remove boot_diagnostic; ubi read $loadaddr diagnostic1; validate $loadaddr && bootm $imgaddr; fi\0" \
   "scrloadaddr=0x44000000\0" \
   "scrname=boot.scr\0" \
   "check_load_boot_scr=if ubifsls $scrname; then ubifsload $scrloadaddr $scrname; bootcustom $scrloadaddr; fi\0" \
   "check_boot_part_dev=if ubi check part.dev; then setenv part part.dev; run boot_part; fi\0" \
   "check_dev_unlock=ubi read $loadaddr dev.crt; validate $loadaddr\0"

#endif /* _IPQCDP_H */
