/*
 * Copyright (c) 2012-2017 The Linux Foundation. All rights reserved.
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

#include <common.h>
#include <linux/mtd/ipq_nand.h>
#include <asm/arch-ipq806x/gpio.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch-ipq806x/clock.h>
#include <asm/arch-qca-common/ebi2.h>
#include <asm/arch-qca-common/smem.h>
#include <asm/arch-qca-common/scm.h>
#include <linux/errno.h>
#include "ipq806x_board_param.h"
#include "ipq806x_cdp.h"
#include <asm/arch-ipq806x/nss/msm_ipq806x_gmac.h>
#include <asm/arch-ipq806x/timer.h>
#include <nand.h>
#include <phy.h>
#include <part.h>
#include <mmc.h>
#include <environment.h>
#include <watchdog.h>

#include <meraki_config.h>

DECLARE_GLOBAL_DATA_PTR;

#define BUILD_ID_LEN	(32)

#define MTDPARTS_BUF_LEN	256

/* Watchdog bite time set to default reset value */
#define RESET_WDT_BITE_TIME 0x31F3

/* Watchdog bark time value is ketp larger than the watchdog timeout
 * of 0x31F3, effectively disabling the watchdog bark interrupt
 */
#define RESET_WDT_BARK_TIME (5 * RESET_WDT_BITE_TIME)

#define GPIO_POE_AT_DETECT                     15
#define GPIO_BRICK_DETECT                      16
#define BIT_MASK_1                                     0x00000001
#define GPIO_CFG_BASE                          0x00801000
#define GPIO_IO_BASE                           0x00801004
#define GPIO_CFG(x)    (GPIO_CFG_BASE + 0x10 * x)
#define GPIO_IO(x) (GPIO_IO_BASE + 0x10 * x)

enum{
       POWER_POE_AT=0,
       POWER_POE_AF=1,
};
enum{
       POWER_ADAPTOR_NOT12V=0,
       POWER_ADAPTOR_12V=1,
};


#define L2ESR_IND_ADDR		(0x204)

/* L2ESR bit fields */
#define L2ESR_MPDCD		BIT(0)
#define L2ESR_MPSLV		BIT(1)
#define L2ESR_TSESB		BIT(2)
#define L2ESR_TSEDB		BIT(3)
#define L2ESR_DSESB		BIT(4)
#define L2ESR_DSEDB		BIT(5)
#define L2ESR_MSE		BIT(6)
#define L2ESR_MPLDREXNOK	BIT(8)

/*
 * If SMEM is not found, we provide a value, that will prevent the
 * environment from being written to random location in the flash.
 *
 * NAND: In the case of NAND, we do this by setting ENV_RANGE to
 * zero. If ENV_RANGE < ENV_SIZE, then environment is not written.
 *
 * SPI Flash: In the case of SPI Flash, we do this by setting the
 * flash_index to -1.
 */

#if !defined(CONFIG_ENV_IS_NOWHERE)
loff_t board_env_offset;
#endif
loff_t board_env_range;
extern int nand_env_device;
#ifdef CONFIG_IPQ_MMC
ipq_mmc mmc_host;
#endif
char *env_name_spec;
extern char *mmc_env_name_spec;
extern char *nand_env_name_spec;
#ifdef CONFIG_CMD_SAVEENV
int (*saveenv)(void);
#endif
#if !defined(CONFIG_ENV_IS_NOWHERE)
env_t *env_ptr;
#endif
extern env_t *mmc_env_ptr;
extern env_t *nand_env_ptr;
extern int mmc_env_init(void);
extern int mmc_saveenv(void);
extern void mmc_env_relocate_spec(void);

extern int nand_env_init(void);
extern int nand_saveenv(void);
extern void nand_env_relocate_spec(void);

#if defined(CONFIG_ENV_IS_NOWHERE)
static int no_saveenv(void)
{
	return 0;
}
#endif

/*
 * Don't have this as a '.bss' variable. The '.bss' and '.rel.dyn'
 * sections seem to overlap.
 *
 * $ arm-none-linux-gnueabi-objdump -h u-boot
 * . . .
 *  8 .rel.dyn      00004ba8  40630b0c  40630b0c  00038b0c  2**2
 *                  CONTENTS, ALLOC, LOAD, READONLY, DATA
 *  9 .bss          0000559c  40630b0c  40630b0c  00000000  2**3
 *                  ALLOC
 * . . .
 *
 * board_early_init_f() initializes this variable, resulting in one
 * of the relocation entries present in '.rel.dyn' section getting
 * corrupted. Hence, when relocate_code()'s 'fixrel' executes, it
 * patches a wrong address, which incorrectly modifies some global
 * variable resulting in a crash.
 *
 * Moral of the story: Global variables that are written before
 * relocate_code() gets executed cannot be in '.bss'
 */
board_ipq806x_params_t *gboard_param = (board_ipq806x_params_t *)0xbadb0ad;
extern int ipq_gmac_eth_initialize(const char *ethaddr);
uchar ipq_def_enetaddr[6] = {0x00, 0x03, 0x7F, 0xBA, 0xDB, 0xAD};

/*******************************************************
Function description: Board specific initialization.
I/P : None
O/P : integer, 0 - no error.

********************************************************/
static board_ipq806x_params_t *get_board_param(unsigned int machid)
{
	unsigned int index = 0;

	for (index = 0; index < NUM_IPQ806X_BOARDS; index++) {
		if (machid == board_params[index].machid)
			return &board_params[index];
	}
	BUG_ON(index == NUM_IPQ806X_BOARDS);
	printf("cdp: Invalid machine id 0x%x\n", machid);
	for (;;);
}

#ifdef CONFIG_IPQ_REPORT_L2ERR
static void report_l2err(u32 l2esr)
{
	if (l2esr & L2ESR_MPDCD)
		printf("L2 Master port decode error\n");

	if (l2esr & L2ESR_MPSLV)
		printf("L2 master port slave error\n");

	if (l2esr & L2ESR_TSESB)
		printf("L2 tag soft error, single-bit\n");

	if (l2esr & L2ESR_TSEDB)
		printf("L2 tag soft error, double-bit\n");

	if (l2esr & L2ESR_DSESB)
		printf("L2 data soft error, single-bit\n");

	if (l2esr & L2ESR_DSEDB)
		printf("L2 data soft error, double-bit\n");

	if (l2esr & L2ESR_MSE)
		printf("L2 modified soft error\n");

	if (l2esr & L2ESR_MPLDREXNOK)
		printf("L2 master port LDREX received Normal OK response\n");
}
#endif

#ifdef CONFIG_HW_WATCHDOG
void hw_watchdog_reset(void)
{
	writel(1, APCS_WDT0_RST);
}
#endif

unsigned int smem_get_board_machtype(void)
{
	return MACH_TYPE_IPQ806X_AP160;
}

int board_init()
{
	int ret;
	uint32_t start_blocks;
	uint32_t size_blocks;
	loff_t board_env_size;

#ifdef CONFIG_IPQ_REPORT_L2ERR
	u32 l2esr;

	/* Record any kind of L2 errors caused during
	 * the previous boot stages as we are clearing
	 * the L2 errors before jumping to linux.
	 * Refer to cleanup_before_linux() */
	l2esr = get_l2_indirect_reg(L2ESR_IND_ADDR);
	report_l2err(l2esr);
#endif
	//ipq_smem_flash_info_t *sfi = &ipq_smem_flash_info;

	/*
	 * after relocation gboard_param is reset to NULL
	 * initialize again
	 */
	gd->bd->bi_boot_params = IPQ_BOOT_PARAMS_ADDR;
	gd->bd->bi_arch_number = smem_get_board_machtype();
    printf("cdp: machid %d\n", smem_get_board_machtype());
	gboard_param = get_board_param(gd->bd->bi_arch_number);


#if 0
	ret = smem_get_boot_flash(&sfi->flash_type,
				  &sfi->flash_index,
				  &sfi->flash_chip_select,
				  &sfi->flash_block_size);
	if (ret < 0) {
		printf("cdp: get boot flash failed\n");
		return ret;
	}

	/*
	 * Should be inited, before env_relocate() is called,
	 * since env. offset is obtained from SMEM.
	 */
	if (sfi->flash_type != SMEM_BOOT_MMC_FLASH) {
		ret = smem_ptable_init();
		if (ret < 0) {
			printf("cdp: SMEM init failed\n");
			return ret;
		}
	}
#endif

#if !defined(CONFIG_ENV_IS_NOWHERE)
	if (sfi->flash_type == SMEM_BOOT_NAND_FLASH) {
		nand_env_device = CONFIG_IPQ_NAND_NAND_INFO_IDX;
	} else if (sfi->flash_type == SMEM_BOOT_SPI_FLASH) {
		nand_env_device = CONFIG_IPQ_SPI_NAND_INFO_IDX;
#ifdef CONFIG_IPQ_MMC
	} else if (sfi->flash_type == SMEM_BOOT_MMC_FLASH) {
		gboard_param->emmc_gpio = emmc1_gpio;
		gboard_param->emmc_gpio_count = ARRAY_SIZE(emmc1_gpio);
#endif
	} else {
		printf("BUG: unsupported flash type : %d\n", sfi->flash_type);
		BUG();
	}


	if (sfi->flash_type != SMEM_BOOT_MMC_FLASH) {
		ret = smem_getpart("0:APPSBLENV", &start_blocks, &size_blocks);
		if (ret < 0) {
			printf("cdp: get environment part failed\n");
			return ret;
		}

		board_env_offset = ((loff_t) sfi->flash_block_size) * start_blocks;
		board_env_size = ((loff_t) sfi->flash_block_size) * size_blocks;
	}

	if (sfi->flash_type == SMEM_BOOT_NAND_FLASH) {
		board_env_range = CONFIG_ENV_SIZE_MAX;
		BUG_ON(board_env_size < CONFIG_ENV_SIZE_MAX);
	} else if (sfi->flash_type == SMEM_BOOT_SPI_FLASH) {
		board_env_range = board_env_size;
		BUG_ON(board_env_size > CONFIG_ENV_SIZE_MAX);
#ifdef CONFIG_IPQ_MMC
	} else if (sfi->flash_type == SMEM_BOOT_MMC_FLASH) {
		board_env_range = CONFIG_ENV_SIZE_MAX;
#endif
        } else {
                printf("BUG: unsupported flash type : %d\n", sfi->flash_type);
                BUG();
        }

	if (sfi->flash_type != SMEM_BOOT_MMC_FLASH) {
		saveenv = nand_saveenv;
		env_ptr = nand_env_ptr;
		env_name_spec = nand_env_name_spec;
#ifdef CONFIG_IPQ_MMC
	} else {
		saveenv = mmc_saveenv;
		env_ptr = mmc_env_ptr;
		env_name_spec = mmc_env_name_spec;
#endif
	}
#else
		board_env_range = CONFIG_ENV_SIZE_MAX;
#ifdef CONFIG_CMD_SAVEENV
		saveenv = no_saveenv;
#endif
#endif

        meraki_config_get_product();

        if (get_meraki_product_id() == MERAKI_BOARD_YOWIE || get_meraki_product_id() == MERAKI_BOARD_CITIZENSNIPS) {
#ifdef CONFIG_IPQ_SNPS_GMAC
            (*(volatile unsigned int *) GPIO_CONFIG_ADDR(6)) = 0x2c0; // PHY enable
            (*(volatile unsigned int *) GPIO_IN_OUT_ADDR(6)) = 0x0;   // Set to low
            udelay(24000);
            (*(volatile unsigned int *) GPIO_IN_OUT_ADDR(6)) = 0x2;   // Set to high
#endif
            //GPIO2, PWM reset, output, low-active
            (*(volatile unsigned int *)GPIO_CFG(2)) = 0x203;
            (*(volatile unsigned int *)GPIO_IO(2)) = 0x2;//keep high level to prevent accident operation
            //GPIO15, POE_AT_DETECT_L, input, low active
            (*(volatile unsigned int *)GPIO_CFG(15)) = 0x3;
            //GPIO16, GPIO_BRICK_DETC_CTL, input, high active
            (*(volatile unsigned int *)GPIO_CFG(16)) = 0x1;
            //GPIO26, Factory Reset button, input, low active
            (*(volatile unsigned int *)GPIO_CFG(26)) = 0x3;
            //GPIO31, WL_LED_ORANGE, output, high active
            (*(volatile unsigned int *)GPIO_CFG(31)) = 0x341;
            (*(volatile unsigned int *)GPIO_IO(31)) = 0x2; //orange on
            //GPIO32, WL_LED_WHITE, output, high active
            (*(volatile unsigned int *)GPIO_CFG(32)) = 0x203;
            (*(volatile unsigned int *)GPIO_IO(32)) = 0x0; //turn white led off

            //GPIO68, IPQ8068 reset, output, low-active
            (*(volatile unsigned int *)GPIO_CFG(68)) = 0x203;
            (*(volatile unsigned int *)GPIO_IO(68)) = 0x2;
        } else {
#ifdef CONFIG_IPQ_SNPS_GMAC
            if (meraki_product_has_aq_phy()) {
                // Fix gmac pins & config
                gboard_param->gmac_gpio_count = ARRAY_SIZE(aq_gmac1_gpio);
                gboard_param->gmac_gpio = aq_gmac1_gpio;
                memcpy(gboard_param->gmac_cfg, aq_gmac_cfg, sizeof(aq_gmac_cfg));
            }
#endif
            //GPIO22, IPQ8068 reset, output, low-active
            (*(volatile unsigned int *)GPIO_CFG(22)) = 0x203;
            (*(volatile unsigned int *)GPIO_IO(22)) = 0x2;
        }

	return 0;
}

#if !defined(CONFIG_ENV_IS_NOWHERE)
void env_relocate_spec(void)
{
	ipq_smem_flash_info_t sfi;

	smem_get_boot_flash(&sfi.flash_type,
				  &sfi.flash_index,
				  &sfi.flash_chip_select,
				  &sfi.flash_block_size);

	if (sfi.flash_type != SMEM_BOOT_MMC_FLASH) {
		nand_env_relocate_spec();
#ifdef CONFIG_IPQ_MMC
	} else {
		mmc_env_relocate_spec();
#endif
	}

}
#endif

void enable_caches(void)
{
	icache_enable();
#ifndef CONFIG_SYS_DCACHE_OFF
	dcache_enable();
#endif
}

void clear_l2cache_err(void)
{
	unsigned int val;

	val = get_l2_indirect_reg(L2ESR_IND_ADDR);
#ifdef CONFIG_IPQ_REPORT_L2ERR
	report_l2err(val);
#endif
	set_l2_indirect_reg(L2ESR_IND_ADDR, val);
}

#if !defined(CONFIG_ENV_IS_NOWHERE)
int env_init(void)
{
	ipq_smem_flash_info_t sfi;
	int ret;

	smem_get_boot_flash(&sfi.flash_type,
				  &sfi.flash_index,
				  &sfi.flash_chip_select,
				  &sfi.flash_block_size);

	if (sfi.flash_type != SMEM_BOOT_MMC_FLASH) {
		ret = nand_env_init();
#ifdef CONFIG_IPQ_MMC
	} else {
		ret = mmc_env_init();
#endif
	}

	return ret;
}
#endif

/*******************************************************
Function description: DRAM initialization.
I/P : None
O/P : integer, 0 - no error.

********************************************************/

int dram_init(void)
{
	gd->ram_size = gboard_param->ddr_size;
	return 0;
}

/*******************************************************
Function description: initi Dram Bank size
I/P : None
O/P : integer, 0 - no error.

********************************************************/


int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = IPQ_KERNEL_START_ADDR;
	gd->bd->bi_dram[0].size = gboard_param->ddr_size - GENERATED_IPQ_RESERVE_SIZE;

	return 0;
}

/**********************************************************
Function description: Display board information on console.
I/P : None
O/P : integer, 0 - no error.

**********************************************************/

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	printf("Board: %s\n", sysinfo.board_string);
	return 0;
}
#endif /* CONFIG_DISPLAY_BOARDINFO */

void reset_cpu(ulong addr)
{
	printf("\nResetting with watch dog!\n");

	writel(0, APCS_WDT0_EN);
	writel(1, APCS_WDT0_RST);
	writel(RESET_WDT_BARK_TIME, APCS_WDT0_BARK_TIME);
	writel(RESET_WDT_BITE_TIME, APCS_WDT0_BITE_TIME);
	writel(1, APCS_WDT0_EN);
	writel(1, APCS_WDT0_CPU0_WDOG_EXPIRED_ENABLE);

	for(;;);
}

static void configure_nand_gpio(void)
{
	/* EBI2 CS, CLE, ALE, WE, OE */
	gpio_tlmm_config(34, 1, 0, GPIO_NO_PULL, GPIO_10MA, GPIO_OE_ENABLE);
	gpio_tlmm_config(35, 1, 0, GPIO_NO_PULL, GPIO_10MA, GPIO_OE_ENABLE);
	gpio_tlmm_config(36, 1, 0, GPIO_NO_PULL, GPIO_10MA, GPIO_OE_ENABLE);
	gpio_tlmm_config(37, 1, 0, GPIO_NO_PULL, GPIO_10MA, GPIO_OE_ENABLE);
	gpio_tlmm_config(38, 1, 0, GPIO_NO_PULL, GPIO_10MA, GPIO_OE_ENABLE);

	/* EBI2 BUSY */
	gpio_tlmm_config(39, 1, 0, GPIO_PULL_UP, GPIO_10MA, GPIO_OE_ENABLE);

	/* EBI2 D7 - D0 */
	gpio_tlmm_config(40, 1, 0, GPIO_KEEPER, GPIO_10MA, GPIO_OE_ENABLE);
	gpio_tlmm_config(41, 1, 0, GPIO_KEEPER, GPIO_10MA, GPIO_OE_ENABLE);
	gpio_tlmm_config(42, 1, 0, GPIO_KEEPER, GPIO_10MA, GPIO_OE_ENABLE);
	gpio_tlmm_config(43, 1, 0, GPIO_KEEPER, GPIO_10MA, GPIO_OE_ENABLE);
	gpio_tlmm_config(44, 1, 0, GPIO_KEEPER, GPIO_10MA, GPIO_OE_ENABLE);
	gpio_tlmm_config(45, 1, 0, GPIO_KEEPER, GPIO_10MA, GPIO_OE_ENABLE);
	gpio_tlmm_config(46, 1, 0, GPIO_KEEPER, GPIO_10MA, GPIO_OE_ENABLE);
	gpio_tlmm_config(47, 1, 0, GPIO_KEEPER, GPIO_10MA, GPIO_OE_ENABLE);
}

void board_nand_init(void)
{
	struct ebi2cr_regs *ebi2_regs;
	if ((gboard_param->flashdesc == ONLY_NAND) ||
	    (gboard_param->flashdesc == NAND_NOR)) {

		ebi2_regs = (struct ebi2cr_regs *) IPQ806x_EBI2CR_BASE;

		nand_clock_config();
		configure_nand_gpio();

		/* NAND Flash is connected to CS0 */
		clrsetbits_le32(&ebi2_regs->chip_select_cfg0, CS0_CFG_MASK,
				CS0_CFG_SERIAL_FLASH_DEVICE);

		ipq_nand_init(IPQ_NAND_LAYOUT_LINUX);
	}
}

#if !defined(CONFIG_ENV_IS_NOWHERE)
void ipq_get_part_details(void)
{
	int ret, i;
	uint32_t start;		/* block number */
	uint32_t size;		/* no. of blocks */

	ipq_smem_flash_info_t *smem = &ipq_smem_flash_info;

	struct { char *name; ipq_part_entry_t *part; } entries[] = {
		{ "0:HLOS", &smem->hlos },
#ifdef CONFIG_IPQ_LOAD_NSS_FW
		{ "0:NSS0", &smem->nss[0] },
		{ "0:NSS1", &smem->nss[1] },
#endif
		{ "rootfs", &smem->rootfs },
	};

	for (i = 0; i < ARRAY_SIZE(entries); i++) {
		ret = smem_getpart(entries[i].name, &start, &size);
		if (ret < 0) {
			ipq_part_entry_t *part = entries[i].part;
			printf("cdp: get part failed for %s\n", entries[i].name);
			part->offset = 0xBAD0FF5E;
			part->size = 0xBAD0FF5E;
		} else {
			ipq_set_part_entry(smem, entries[i].part, start, size);
		}
	}

	return;
}
#endif

/*
 * Get the kernel partition details from SMEM and populate the,
 * environment with sufficient information for the boot command to
 * load and execute the kernel.
 */
int board_late_init(void)
{
	unsigned int machid;
	//ipq_smem_flash_info_t *sfi = &ipq_smem_flash_info;

    /*
	if (sfi->flash_type != SMEM_BOOT_MMC_FLASH) {
		ipq_get_part_details();
	}
    */

        /* get machine type from SMEM and set in env */
	machid = gd->bd->bi_arch_number;
	if (machid != 0) {
		env_set_addr("machid", (void *)machid);
        printf("late_init: machid %d\n", machid);
		gd->bd->bi_arch_number = machid;
	}

	set_meraki_kernel_config();
	return 0;
}

/*
 * This function is called in the very beginning.
 * Retreive the machtype info from SMEM and map the board specific
 * parameters. Shared memory region at Dram address 0x40400000
 * contains the machine id/ board type data polulated by SBL.
 */
int board_early_init_f(void)
{
	gboard_param = get_board_param(smem_get_board_machtype());
	return 0;
}

/*
 * Gets the ethernet address from the ART partition table and return the value
 */
int get_eth_mac_address(struct eth_device *dev)
{
	s32 ret = 0;
	uchar enet_addr[6];
	ret = eth_getenv_enetaddr("ethaddr", enet_addr);
	if(!ret) {
		memcpy(dev->enetaddr, ipq_def_enetaddr, 6);
		//just using dev address to make mac addresses different
		dev->enetaddr[5] = (u8)dev & 0xff;
	} else {
		memcpy(dev->enetaddr, enet_addr, 6);
	}
	meraki_cryptid_ethaddr(dev->enetaddr, 1);

	return ret;
}

void ipq_configure_gpio(gpio_func_data_t *gpio, uint count)
{
	int i;

	for (i = 0; i < count; i++) {
		gpio_tlmm_config(gpio->gpio, gpio->func, gpio->out,
				gpio->pull, gpio->drvstr, gpio->oe);
		gpio++;
	}
}

#ifndef CONFIG_IPQ_SNPS_GMAC
int board_eth_init(bd_t *bis)
{
       return 0;
}
#else
int board_eth_init(bd_t *bis)
{
	int status;
	gpio_func_data_t *gpio = gboard_param->switch_gpio;

	if (gpio) {
		gpio_tlmm_config(gpio->gpio, gpio->func, gpio->out,
				gpio->pull, gpio->drvstr, gpio->oe);
	};

	ipq_gmac_common_init(gboard_param->gmac_cfg);

	ipq_configure_gpio(gboard_param->gmac_gpio,
			gboard_param->gmac_gpio_count);
	/*
	 * Register the swith driver routines before
	 * initing the GMAC
	 */
	switch (gboard_param->machid) {
	case MACH_TYPE_IPQ806X_AP160_2XX:
		ipq_register_switch(ipq_qca8511_init);
		break;
	case MACH_TYPE_IPQ806X_AP160:
	  {
		 switch(get_meraki_product_id()) {
			case MERAKI_BOARD_SASQUATCH:
			case MERAKI_BOARD_WOOKIE:
			case MERAKI_BOARD_DOCTOROCTOPUS:
				 ipq_register_switch(ipq_qca8511_init);
				 break;
			default:
				 break;
		}
	  }
		break;
	default:
		ipq_register_switch(ipq_athrs17_init);
		break;
	}

	status = ipq_gmac_init(gboard_param->gmac_cfg);
	return status;
}
#endif

#ifdef CONFIG_IPQ_MMC

int board_mmc_env_init(void)
{
	block_dev_desc_t *blk_dev;
	disk_partition_t disk_info;
	loff_t board_env_size;
	int ret;

	if (mmc_init(mmc_host.mmc)) {
		/* The HS mode command(cmd6) is getting timed out. So mmc card is
		 * not getting initialized properly. Since the env partition is not
		 * visible, the env default values are writing into the default
		 * partition (start of the mmc device). So do a reset again.
		 */
		if (mmc_init(mmc_host.mmc)) {
			printf("MMC init failed \n");
			return -1;
		}
	}
	blk_dev = mmc_get_dev(mmc_host.dev_num);
	ret = find_part_efi(blk_dev, "0:APPSBLENV", &disk_info);

	if (ret > 0) {
		board_env_offset = disk_info.start * disk_info.blksz;
		board_env_size = disk_info.size * disk_info.blksz;
		board_env_range = board_env_size;
		BUG_ON(board_env_size > CONFIG_ENV_SIZE_MAX);
	}
	return ret;
}

int board_mmc_init(bd_t *bis)
{
	gpio_func_data_t *gpio = gboard_param->emmc_gpio;

	if (gpio) {
		mmc_host.base = MSM_SDC1_BASE;
		ipq_configure_gpio(gboard_param->emmc_gpio,
				gboard_param->emmc_gpio_count);
		mmc_host.clk_mode = MMC_IDENTIFY_MODE;
		emmc_clock_config(mmc_host.clk_mode);

		ipq_mmc_init(bis, &mmc_host);
		board_mmc_env_init();
	}
	return 0;
}

void board_mmc_deinit(void)
{
	int i;
	gpio_func_data_t *gpio = gboard_param->emmc_gpio;

	if (gpio) {
		for (i = 0; i < gboard_param->emmc_gpio_count; i++) {
			gpio_tlmm_config(gpio->gpio, 0, 0, 0, 0, 0);
			gpio++;
		}
		emmc_clock_disable();
	}
}
#endif

#ifdef CONFIG_OF_BOARD_SETUP
int ipq_get_tz_version(char *buf, int size)
{
	int ret;

	ret = scm_call(SCM_SVC_INFO, TZBSP_BUILD_VER_QUERY_CMD, NULL,
			0, buf, BUILD_ID_LEN);
	if(ret) {
		printf("ipq: fdt fixup cannot get TZ build ID\n");
		return ret;
	}

	snprintf(buf, size, "%s\n", buf);
	return 0;
}

void ipq_fdt_fixup_version(void *blob)
{
	int nodeoff;
	int ret;
	char version[BUILD_ID_LEN + 1];

	nodeoff = fdt_path_offset(blob, "/");
	if (nodeoff < 0) {
		debug("ipq: fdt fixup unable to find root node\n");
		return;
	}

#if 0 // MERAKI UNUSED
	if(!ipq_smem_get_boot_version(version, sizeof(version))) {
		debug("BOOT Build Version: %s\n", version);
		ret = fdt_setprop(blob, nodeoff, "boot_version",
				version, strlen(version));
		if (ret)
			debug("%s: unable to set Boot version\n", __func__);
	}
#endif

	if(!ipq_get_tz_version(version, sizeof(version))) {
		debug("TZ Build Version: %s\n", version);
		ret = fdt_setprop(blob, nodeoff, "tz_version",
				version, strlen(version));
		if (ret)
			debug("%s: unable to set TZ version\n", __func__);
	}
}

int ipq_fdt_fixup_socinfo(void *blob)
{
	uint32_t cpu_type;
	int nodeoff, ret;

	ret = ipq_smem_get_socinfo_cpu_type(&cpu_type);
	if (ret) {
		printf("ipq: fdt fixup cannot get socinfo\n");
		return ret;
	}

	if (get_meraki_product_id() == MERAKI_BOARD_DOCTOROCTOPUS || get_meraki_product_id() == MERAKI_BOARD_CITIZENSNIPS) {
		nodeoff = fdt_node_offset_by_compatible(blob, -1, "qcom,ipq8064-ap160");
	} else {
		nodeoff = fdt_node_offset_by_compatible(blob, -1, "qcom,ipq806x");
	}

	if (nodeoff < 0) {
		printf("ipq: fdt fixup cannot find compatible node\n");
		return nodeoff;
	}
	ret = fdt_setprop(blob, nodeoff, "cpu_type",
				&cpu_type, sizeof(cpu_type));
	if (ret)
		printf("%s: cannot set cpu type %d\n", __func__, ret);
	return ret;
}

/*
 * For newer kernel that boot with device tree (3.14+), all of memory is
 * described in the /memory node, including areas that the kernel should not be
 * touching.
 *
 * By default, u-boot will walk the dram bank info and populate the /memory
 * node; here, overwrite this behavior so we describe all of memory instead.
 */
int ft_board_setup(void *blob, bd_t *bd)
{
	u64 memory_start = CONFIG_SYS_SDRAM_BASE;
	u64 memory_size = gboard_param->ddr_size;

	ipq_fdt_fixup_version(blob);

	fdt_fixup_memory_banks(blob, &memory_start, &memory_size, 1);

	int ret = ipq_fdt_fixup_socinfo(blob);
	if (0 != ret) {
		printf("ipq: fdt fixup failed for socinfo\n");
	}
	return ret;
}
#endif /* CONFIG_OF_BOARD_SETUP */

#ifdef CONFIG_IPQ806X_PCI

static inline void ipq_pci_gpio_set(uint32_t gpio, uint32_t set)
{
	uint32_t addr;
	uint32_t val;

	addr = GPIO_IN_OUT_ADDR(gpio);
	val = readl(addr);
	val |= (1 << set);
	writel(val, addr);
}

static void ipq_pci_gpio_fixup(void)
{
	unsigned int machid;
	/* get machine type from SMEM and set in env */
	machid = gd->bd->bi_arch_number;

	gpio_func_data_t *gpio_0 = gboard_param->pcie_cfg[0].pci_rst_gpio;
	gpio_func_data_t *gpio_1 = gboard_param->pcie_cfg[1].pci_rst_gpio;
	gpio_func_data_t *gpio_2 = gboard_param->pcie_cfg[2].pci_rst_gpio;

	if (machid == MACH_TYPE_IPQ806X_RUMI3) {
		gpio_0->gpio = -1;
		gpio_1->gpio = -1;
		gpio_2->gpio = -1;
	} else if (machid == MACH_TYPE_IPQ806X_DB147) {
		gpio_1->gpio = -1;
		gpio_2->gpio = PCIE_1_RST_GPIO;
	} else if ((machid == MACH_TYPE_IPQ806X_AP148) ||
				(machid == MACH_TYPE_IPQ806X_AP148_1XX )) {
		gpio_2->gpio = -1;
	}
}

static void ipq_pcie_write_mask(uint32_t addr,
				uint32_t clear_mask, uint32_t set_mask)
{
	uint32_t val;

	val = (readl(addr) & ~clear_mask) | set_mask;
	writel(val, addr);
}

static void ipq_pcie_parf_reset(uint32_t addr, int domain, int assert)

{
	if (assert)
		ipq_pcie_write_mask(addr, 0, domain);
	else
		ipq_pcie_write_mask(addr, domain, 0);
}

static void ipq_pcie_config_controller(int id)
{
	pcie_params_t 	*cfg;
	cfg = &gboard_param->pcie_cfg[id];

	/*
	 * program and enable address translation region 0 (device config
	 * address space); region type config;
	 * axi config address range to device config address range
	 */
	writel(0, cfg->pcie20 + PCIE20_PLR_IATU_VIEWPORT);

	writel(4, cfg->pcie20 + PCIE20_PLR_IATU_CTRL1);
	writel(BIT(31), cfg->pcie20 + PCIE20_PLR_IATU_CTRL2);
	writel(cfg->axi_conf , cfg->pcie20 + PCIE20_PLR_IATU_LBAR);
	writel(0, cfg->pcie20 + PCIE20_PLR_IATU_UBAR);
	writel((cfg->axi_conf + PCIE_AXI_CONF_SIZE - 1),
				cfg->pcie20 + PCIE20_PLR_IATU_LAR);
	writel(MSM_PCIE_DEV_CFG_ADDR,
				cfg->pcie20 + PCIE20_PLR_IATU_LTAR);
	writel(0, cfg->pcie20 + PCIE20_PLR_IATU_UTAR);

	/*
	 * program and enable address translation region 2 (device resource
	 * address space); region type memory;
	 * axi device bar address range to device bar address range
	 */
	writel(2, cfg->pcie20 + PCIE20_PLR_IATU_VIEWPORT);

	writel(0, cfg->pcie20 + PCIE20_PLR_IATU_CTRL1);
	writel(BIT(31), cfg->pcie20 + PCIE20_PLR_IATU_CTRL2);
	writel(cfg->axi_bar_start, cfg->pcie20 + PCIE20_PLR_IATU_LBAR);
	writel(0, cfg->pcie20 + PCIE20_PLR_IATU_UBAR);
	writel((cfg->axi_bar_start + cfg->axi_bar_size
		- PCIE_AXI_CONF_SIZE - 1), cfg->pcie20 + PCIE20_PLR_IATU_LAR);
	writel(cfg->axi_bar_start, cfg->pcie20 + PCIE20_PLR_IATU_LTAR);
	writel(0, cfg->pcie20 + PCIE20_PLR_IATU_UTAR);

	/* 1K PCIE buffer setting */
	writel(0x3, cfg->pcie20 + PCIE20_AXI_MSTR_RESP_COMP_CTRL0);
	writel(0x1, cfg->pcie20 + PCIE20_AXI_MSTR_RESP_COMP_CTRL1);

}

void board_pci_init()
{
	int i,j;
	pcie_params_t 		*cfg;
	gpio_func_data_t	*gpio_data;
	uint32_t val;

	ipq_pci_gpio_fixup();

	for (i = 0; i < PCI_MAX_DEVICES; i++) {
		cfg = &gboard_param->pcie_cfg[i];
		gpio_data = cfg->pci_rst_gpio;
		cfg->axi_conf = cfg->axi_bar_start +
					cfg->axi_bar_size - PCIE_AXI_CONF_SIZE;
		if (gpio_data->gpio != -1) {
			gpio_tlmm_config(gpio_data->gpio, gpio_data->func,
					gpio_data->out,	gpio_data->pull,
					gpio_data->drvstr, gpio_data->oe);
			ipq_pci_gpio_set(gpio_data->gpio, 1);
			udelay(3000);
		}


		/* assert PCIe PARF reset while powering the core */
		ipq_pcie_parf_reset(cfg->pcie_rst, BIT(6), 0);

		ipq_pcie_parf_reset(cfg->pcie_rst, BIT(2), 1);
		pcie_clock_config(cfg->pci_clks);
		/*
		* de-assert PCIe PARF reset;
		* wait 1us before accessing PARF registers
		*/
		ipq_pcie_parf_reset(cfg->pcie_rst, BIT(2), 0);
		udelay(1);

		/* enable PCIe clocks and resets */
		val = (readl(cfg->parf + PCIE20_PARF_PHY_CTRL) & ~BIT(0));
		writel(val, cfg->parf + PCIE20_PARF_PHY_CTRL);

		/* Set Tx Termination Offset */
		val = readl(cfg->parf + PCIE20_PARF_PHY_CTRL) |
				PCIE20_PARF_PHY_CTRL_PHY_TX0_TERM_OFFST(0);
		writel(val, cfg->parf + PCIE20_PARF_PHY_CTRL);

		/* PARF programming */
		writel(PCIE20_PARF_PCS_DEEMPH_TX_DEEMPH_GEN1(0x18) |
			PCIE20_PARF_PCS_DEEMPH_TX_DEEMPH_GEN2_3_5DB(0x18) |
			PCIE20_PARF_PCS_DEEMPH_TX_DEEMPH_GEN2_6DB(0x22),
			 cfg->parf + PCIE20_PARF_PCS_DEEMPH);

		writel(PCIE20_PARF_PCS_SWING_TX_SWING_FULL(0x78) |
			PCIE20_PARF_PCS_SWING_TX_SWING_LOW(0x78),
			cfg->parf + PCIE20_PARF_PCS_SWING);

		writel((4<<24), cfg->parf + PCIE20_PARF_CONFIG_BITS);

		writel(0x11019, cfg->pci_clks->parf_phy_refclk);
		writel(0x10019, cfg->pci_clks->parf_phy_refclk);


		/* enable access to PCIe slave port on system fabric */
		if (i == 0) {
			writel(BIT(4), PCIE_SFAB_AXI_S5_FCLK_CTL);
		}

		udelay(1);
		/* de-assert PICe PHY, Core, POR and AXI clk domain resets */
		ipq_pcie_parf_reset(cfg->pcie_rst, BIT(5), 0);
		ipq_pcie_parf_reset(cfg->pcie_rst, BIT(4), 0);
		ipq_pcie_parf_reset(cfg->pcie_rst, BIT(3), 0);
		ipq_pcie_parf_reset(cfg->pcie_rst, BIT(0), 0);

		/* enable link training */
		ipq_pcie_write_mask(cfg->elbi + PCIE20_ELBI_SYS_CTRL, 0,
								BIT(0));
		udelay(500);

		for (j = 0; j < 10; j++) {
			val = readl(cfg->pcie20 + PCIE20_CAP_LINKCTRLSTATUS);
			if (val & BIT(29)) {
				printf("PCI%d Link Intialized\n", i);
				cfg->linkup = 1;
				break;
			}
			udelay(10000);
		}
		ipq_pcie_config_controller(i);
	}

}

void board_pci_deinit()
{
	int i;
	pcie_params_t 		*cfg;
	gpio_func_data_t	*gpio_data;

	for (i = 0; i < PCI_MAX_DEVICES; i++) {
		cfg = &gboard_param->pcie_cfg[i];
		gpio_data = cfg->pci_rst_gpio;

		if (gpio_data->gpio != -1)
			gpio_tlmm_config(gpio_data->gpio, 0, GPIO_INPUT,
					GPIO_NO_PULL, GPIO_2MA, GPIO_OE_DISABLE);
		writel(0x7d, cfg->pcie_rst);
		writel(1, cfg->parf + PCIE20_PARF_PHY_CTRL);
		pcie_clock_shutdown(cfg->pci_clks);
	}

}
#endif /* CONFIG_IPQ806X_PCI */

void flash_power_led(int seconds)
{
	uint16_t gpio;
	int i;
	int delay = 100; /* ms */

	switch (get_meraki_product_id()) {
	case MERAKI_BOARD_CITIZENSNIPS:
		gpio = 31;  // gpio 31 -> orange power led
		break;
	case MERAKI_BOARD_DOCTOROCTOPUS:
		gpio = 19;  // gpio 19 -> orange power led
		break;

	default:
		printf("%s: Unknown board type!!\n", __func__);
		return;
	}

	(*(volatile unsigned int *)GPIO_CFG(gpio)) = 0x203;
	udelay(250);

	seconds *= 1000 / (delay * 2);
	for (i = 0; !seconds || i < seconds; i++) {
		// turn power led off
		(*(volatile unsigned int *)GPIO_IO(gpio)) = 0x0;
		mdelay(delay);
		// turn power led off
		(*(volatile unsigned int *)GPIO_IO(gpio)) = 0x2;
		mdelay(delay);
	}
}

/* after blowing fuses, cryptids need to blink leds for operator intervention */
void post_fuse_action(void)
{
      printf("Power-off now.\n");

      /* Hang forever (seconds == 0), flashing power LED */
      flash_power_led(0);
}

int do_bootbk (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
        ulong addr;
        char cmd[128];
        char cmdline[128];

        /* Validate arguments */
        if (argc != 4)
                return -1;

#ifdef CONFIG_IPQ806X_PCI
        board_pci_deinit();
#endif /* CONFIG_IPQ806X_PCI */

        addr = simple_strtoul(argv[1], NULL, 16);

        sprintf(cmdline, "loader=u-boot part=%s",argv[2]);
        env_set("bootargs", cmdline);
        env_set("machid", "136b");
        sprintf(cmd, "bootm 0x%x#%s", (unsigned int)addr, argv[3]);
        printf(cmd);
        return run_command(cmd, 0);
}

U_BOOT_CMD(bootbk, 4, 0, do_bootbk,
           "Boot Meraki bootkernel",
           "bootbk <address> <partition-name> <itb config>");
