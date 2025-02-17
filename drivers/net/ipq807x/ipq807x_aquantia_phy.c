/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <common.h>
#include <net.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <malloc.h>
#include <phy.h>
#include <miiphy.h>
#include "ipq_phy.h"
#include "ipq807x_aquantia_phy.h"
#include <crc.h>
#include <mmc.h>
#include <nand.h>
#include <asm/global_data.h>
#include <asm/global_data.h>
#include <asm/arch-qca-common/smem.h>

DECLARE_GLOBAL_DATA_PTR;
typedef struct {
	unsigned int image_type;
	unsigned int header_vsn_num;
	unsigned int image_src;
	unsigned char *image_dest_ptr;
	unsigned int image_size;
	unsigned int code_size;
	unsigned char *signature_ptr;
	unsigned int signature_size;
	unsigned char *cert_chain_ptr;
	unsigned int cert_chain_size;
} mbn_header_t;

mbn_header_t *fwimg_header;
static int debug = 0;

#ifdef CONFIG_QCA_MMC
extern qca_mmc mmc_host;
static qca_mmc *host = &mmc_host;
#endif

extern int ipq_mdio_write(int mii_id,
		int regnum, u16 value);
extern int ipq_mdio_read(int mii_id,
		int regnum, ushort *data);

extern int ipq_sw_mdio_init(const char *);
extern void eth_clock_enable(void);
static int program_ethphy_fw(unsigned int phy_addr,
			 uint32_t load_addr,uint32_t file_size );

u16 aq_phy_reg_write(u32 dev_id, u32 phy_id,
		u32 reg_id, u16 reg_val)
{
	ipq_mdio_write(phy_id, reg_id, reg_val);
	return 0;
}

u16 aq_phy_reg_read(u32 dev_id, u32 phy_id, u32 reg_id)
{
	return ipq_mdio_read(phy_id, reg_id, NULL);
}

u8 aq_phy_get_link_status(u32 dev_id, u32 phy_id)
{
	u16 phy_data;
	uint32_t reg;
	
	reg = AQ_PHY_AUTO_STATUS_REG | AQUANTIA_MII_ADDR_C45; 
	phy_data = aq_phy_reg_read(dev_id, phy_id, reg);
	phy_data = aq_phy_reg_read(dev_id, phy_id, reg);
	
	if (((phy_data >> 2) & 0x1) & PORT_LINK_UP)
		return 0;

	return 1;
}

u32 aq_phy_get_duplex(u32 dev_id, u32 phy_id, fal_port_duplex_t *duplex)
{
	u16 phy_data;
	uint32_t reg;

	reg = AQ_PHY_LINK_STATUS_REG | AQUANTIA_MII_ADDR_C45; 
	phy_data = aq_phy_reg_read(dev_id, phy_id, reg);

	/*
	 * Read duplex
	 */
	phy_data = phy_data & 0x1;
	if (phy_data & 0x1)
		*duplex = FAL_FULL_DUPLEX;
	else
		*duplex = FAL_HALF_DUPLEX;

	return 0;
}

u32 aq_phy_get_speed(u32 dev_id, u32 phy_id, fal_port_speed_t *speed)
{
	u16 phy_data;
	uint32_t reg;

	reg = AQ_PHY_LINK_STATUS_REG | AQUANTIA_MII_ADDR_C45; 
	phy_data = aq_phy_reg_read(dev_id, phy_id, reg);

	switch ((phy_data >> 1) & 0x7) {
	case SPEED_10G:
		*speed = FAL_SPEED_10000;
		break;
	case SPEED_5G:
		*speed = FAL_SPEED_5000;
		break;
	case SPEED_2_5G:
		*speed = FAL_SPEED_2500;
		break;
	case SPEED_1000MBS:
		*speed = FAL_SPEED_1000;
		break;
	case SPEED_100MBS:
		*speed = FAL_SPEED_100;
		break;
	case SPEED_10MBS:
		*speed = FAL_SPEED_10;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

void aquantia_phy_restart_autoneg(u32 phy_id)
{
	u16 phy_data;

	phy_data = aq_phy_reg_read(0x0, phy_id, AQUANTIA_REG_ADDRESS(AQUANTIA_MMD_PHY_XS_REGISTERS,
			AQUANTIA_PHY_XS_USX_TRANSMIT));
	if (!(phy_data & AQUANTIA_PHY_USX_AUTONEG_ENABLE))
		aq_phy_reg_write(0x0, phy_id,AQUANTIA_REG_ADDRESS(
			AQUANTIA_MMD_PHY_XS_REGISTERS,
			AQUANTIA_PHY_XS_USX_TRANSMIT),
			 phy_data | AQUANTIA_PHY_USX_AUTONEG_ENABLE);

	phy_data = aq_phy_reg_read(0x0, phy_id, AQUANTIA_REG_ADDRESS(AQUANTIA_MMD_AUTONEG,
			AQUANTIA_AUTONEG_STANDARD_CONTROL1));

	phy_data |= AQUANTIA_CTRL_AUTONEGOTIATION_ENABLE;
	aq_phy_reg_write(0x0, phy_id, AQUANTIA_REG_ADDRESS(AQUANTIA_MMD_AUTONEG,
		AQUANTIA_AUTONEG_STANDARD_CONTROL1),
		phy_data | AQUANTIA_CTRL_RESTART_AUTONEGOTIATION);
}

int ipq_qca_aquantia_phy_init(struct phy_ops **ops, u32 phy_id)
{
	u16 phy_data;
	struct phy_ops *aq_phy_ops;
	aq_phy_ops = (struct phy_ops *)malloc(sizeof(struct phy_ops));
	if (!aq_phy_ops)
		return -ENOMEM;
	aq_phy_ops->phy_get_link_status = aq_phy_get_link_status;
	aq_phy_ops->phy_get_speed = aq_phy_get_speed;
	aq_phy_ops->phy_get_duplex = aq_phy_get_duplex;
	*ops = aq_phy_ops;

	phy_data = aq_phy_reg_read(0x0, phy_id, AQUANTIA_REG_ADDRESS(1, QCA_PHY_ID1));
	printf ("PHY ID1: 0x%x\n", phy_data);
	phy_data = aq_phy_reg_read(0x0, phy_id, AQUANTIA_REG_ADDRESS(1, QCA_PHY_ID2));
	printf ("PHY ID2: 0x%x\n", phy_data);
	phy_data = aq_phy_reg_read(0x0, phy_id, AQUANTIA_REG_ADDRESS(AQUANTIA_MMD_PHY_XS_REGISTERS,
			AQUANTIA_PHY_XS_USX_TRANSMIT));
	phy_data |= AQUANTIA_PHY_USX_AUTONEG_ENABLE;
	aq_phy_reg_write(0x0, phy_id, AQUANTIA_REG_ADDRESS(AQUANTIA_MMD_PHY_XS_REGISTERS,
			AQUANTIA_PHY_XS_USX_TRANSMIT), phy_data);
	phy_data = aq_phy_reg_read(0x0, phy_id, AQUANTIA_REG_ADDRESS(AQUANTIA_MMD_AUTONEG,
			AQUANTIA_AUTONEG_TRANSMIT_VENDOR_INTR_MASK));
	phy_data |= AQUANTIA_INTR_LINK_STATUS_CHANGE;
	aq_phy_reg_write(0x0, phy_id, AQUANTIA_REG_ADDRESS(AQUANTIA_MMD_AUTONEG,
			AQUANTIA_AUTONEG_TRANSMIT_VENDOR_INTR_MASK), phy_data);
	phy_data = aq_phy_reg_read(0x0, phy_id, AQUANTIA_REG_ADDRESS(AQUANTIA_MMD_GLOABLE_REGISTERS,
			AQUANTIA_GLOBAL_INTR_STANDARD_MASK));
	phy_data |= AQUANTIA_ALL_VENDOR_ALARMS_INTERRUPT_MASK;
	aq_phy_reg_write(0x0, phy_id, AQUANTIA_REG_ADDRESS(AQUANTIA_MMD_GLOABLE_REGISTERS,
			AQUANTIA_GLOBAL_INTR_STANDARD_MASK), phy_data);
	phy_data = aq_phy_reg_read(0x0, phy_id, AQUANTIA_REG_ADDRESS(AQUANTIA_MMD_GLOABLE_REGISTERS,
			AQUANTIA_GLOBAL_INTR_VENDOR_MASK));
	phy_data |= AQUANTIA_AUTO_AND_ALARMS_INTR_MASK;
	aq_phy_reg_write(0x0, phy_id, AQUANTIA_REG_ADDRESS(AQUANTIA_MMD_GLOABLE_REGISTERS,
			AQUANTIA_GLOBAL_INTR_VENDOR_MASK), phy_data);
	return 0;
}

static int do_load_fw(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int phy_addr=AQU_PHY_ADDR;
	int node, aquantia_port;

	if (argc > 2)
		return CMD_RET_USAGE;

	if (argc == 2)
		phy_addr = simple_strtoul(argv[1], NULL, 16);

	if (phy_addr != AQU_PHY_ADDR) {
		printf("Phy address is not correct: use 0x%x\n", AQU_PHY_ADDR);
		return 0;
	}

	node = fdt_path_offset(gd->fdt_blob, "/ess-switch");
	if (node < 0) {
		printf("Error: ess-switch not specified in dts");
		return 0;
	}

	aquantia_port = fdtdec_get_uint(gd->fdt_blob, node, "aquantia_port", -1);
	if (aquantia_port < 0) {
		printf("Error: aquantia_port not specified in dts");
		return 0;
	}

	aquantia_port = fdtdec_get_uint(gd->fdt_blob, node, "aquantia_gpio", -1);
	if (aquantia_port < 0) {
		printf("Error: aquantia_gpio not specified in dts");
		return 0;
	}

	miiphy_init();
	eth_clock_enable();
	ipq_sw_mdio_init("IPQ MDIO0");
	ipq_board_fw_download(phy_addr);
	return 0;
}

static int do_aq_phy_restart(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int phy_addr = AQU_PHY_ADDR;
	int node, aquantia_port;
	if (argc > 2)
		return CMD_RET_USAGE;

	if (argc == 2)
		phy_addr = simple_strtoul(argv[1], NULL, 16);

	aquantia_phy_restart_autoneg(phy_addr);
	return 0;
}


int ipq_board_fw_download(unsigned int phy_addr)
{
	char runcmd[256];
	int ret,i=0;
	uint32_t start;         /* block number */
	uint32_t size;          /* no. of blocks */
	qca_part_entry_t	ethphyfw;
	struct { char *name; qca_part_entry_t *part; } entries[] = {
		{ "0:ETHPHYFW", &ethphyfw },
	};
#ifdef CONFIG_QCA_MMC
	block_dev_desc_t *blk_dev;
	disk_partition_t disk_info;
#endif

#if 0
	/* check the smem info to see which flash used for booting */
	if (sfi->flash_type == SMEM_BOOT_SPI_FLASH) {
		if (debug) {
			printf("Using nor device \n");
		}
	} else if (sfi->flash_type == SMEM_BOOT_NAND_FLASH) {
		if (debug) {
			printf("Using nand device 0\n");
		}
	} else if (sfi->flash_type == SMEM_BOOT_MMC_FLASH) {
		if (debug) {
			printf("Using MMC device\n");
		}
	} else {
		printf("Unsupported BOOT flash type\n");
		return -1;
	}

	ret = smem_getpart(entries[i].name, &start, &size);
	if (ret < 0) {
		debug("cdp: get part failed for %s\n", entries[i].name);
	} else {
		qca_set_part_entry(entries[i].name, sfi, entries[i].part, start, size);
	}

	if  (sfi->flash_type == SMEM_BOOT_NAND_FLASH) {
		/*
		 * Kernel is in a separate partition
		 */
		snprintf(runcmd, sizeof(runcmd),
			 /* NOR is treated as psuedo NAND */
			 "nand read 0x%x 0x%llx 0x%llx && ",
			 CONFIG_SYS_LOAD_ADDR, ethphyfw.offset, ethphyfw.size);

		if (debug)
			printf("%s", runcmd);

		if (run_command(runcmd, 0) != CMD_RET_SUCCESS)
			return CMD_RET_FAILURE;
	} else if (sfi->flash_type == SMEM_BOOT_SPI_FLASH) {
		snprintf(runcmd, sizeof(runcmd),
			 "sf probe && "
			 "sf read 0x%x 0x%llx 0x%llx && ",
			 CONFIG_SYS_LOAD_ADDR, ethphyfw.offset, ethphyfw.size);

		if (debug)
			printf("%s", runcmd);

		if (run_command(runcmd, 0) != CMD_RET_SUCCESS)
			return CMD_RET_FAILURE;
#ifdef CONFIG_QCA_MMC
	} else if (sfi->flash_type == SMEM_BOOT_MMC_FLASH ) {
		blk_dev = mmc_get_dev(host->dev_num);
		 ret = get_partition_info_efi_by_name(blk_dev,
						"0:ETHPHYFW", &disk_info);
		if (ret == 0) {
			snprintf(runcmd, sizeof(runcmd), "mmc read 0x%x 0x%X 0x%X",
				 CONFIG_SYS_LOAD_ADDR,
				 (uint)disk_info.start, (uint)disk_info.size);

			if (run_command(runcmd, 0) != CMD_RET_SUCCESS)
				return CMD_RET_FAILURE;
		}
#endif
	}
#endif
		if (run_command("run set_ubi", 0) != CMD_RET_SUCCESS)
			return CMD_RET_FAILURE;
		if (run_command("ubi check ETHPHYFW", 0) != CMD_RET_SUCCESS)
			return CMD_RET_FAILURE;

		snprintf(runcmd, sizeof(runcmd),
			 "ubi read 0x%x ETHPHYFW",
			 CONFIG_SYS_LOAD_ADDR);

		if (run_command(runcmd, 0) != CMD_RET_SUCCESS)
			return CMD_RET_FAILURE;

	fwimg_header = CONFIG_SYS_LOAD_ADDR;

	if( fwimg_header->image_type == 0x13 && fwimg_header->header_vsn_num == 0x3) {
		program_ethphy_fw(phy_addr, (uint32_t )(CONFIG_SYS_LOAD_ADDR + sizeof(mbn_header_t)),
			(uint32_t )(fwimg_header->image_size));
	}else {
		printf("bad magic on ETHPHYFW partition\n");
		return -1;
	}
	return 0;
}


#define AQ_PHY_IMAGE_HEADER_CONTENT_OFFSET_HHD 0x300
static int program_ethphy_fw(unsigned int phy_addr, uint32_t load_addr, uint32_t file_size)
{
	int i;
	uint8_t *buf;
	uint16_t file_crc;
	uint16_t computed_crc;
	uint32_t reg1, reg2;
	uint16_t recorded_ggp8_val;
	uint16_t daisy_chain_dis;
	uint32_t primary_header_ptr = 0x00000000;
	uint32_t primary_iram_ptr = 0x00000000;
	uint32_t primary_dram_ptr = 0x00000000;
	uint32_t primary_iram_sz = 0x00000000;
	uint32_t primary_dram_sz = 0x00000000;
	uint32_t phy_img_hdr_off;
	uint32_t byte_sz;
	uint32_t dword_sz;
	uint32_t byte_ptr;
	uint16_t msw;
	uint16_t lsw;
	uint8_t msb1;
	uint8_t msb2;
	uint8_t lsb1;
	uint8_t lsb2;
	uint16_t mailbox_crc;

	aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0x300), 0xdead);
	aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0x301), 0xbeaf);
	reg1 = aq_phy_reg_read(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0x300));
	reg2 = aq_phy_reg_read(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0x301));

	if(reg1 != 0xdead && reg2 != 0xbeaf) {
		printf("PHY::Scratchpad Read/Write test fail\n");
		return 0;
	}
	buf = (uint8_t *)load_addr;
	file_crc = buf[file_size - 2] << 8 | buf[file_size - 1];
	computed_crc = crc16_ccitt(0, buf, file_size - 2);

	if (file_crc != computed_crc) {
		printf("CRC check failed on phy fw file\n");
		return 0;
	} else {
		printf ("CRC check good on phy fw file (0x%04X)\n",computed_crc);
	}

	daisy_chain_dis = aq_phy_reg_read(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0xc452));
	if (!(daisy_chain_dis & 0x1))
		aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0xc452), 0x1);

	aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0xc471), 0x40);
	recorded_ggp8_val = aq_phy_reg_read(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0xc447));
	if ((recorded_ggp8_val & 0x1f) != phy_addr)
		aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0xc447), phy_addr);
	aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0xc441), 0x4000);
	aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0xc001), 0x41);
	primary_header_ptr = (((buf[0x9] & 0x0F) << 8) | buf[0x8]) << 12;
	phy_img_hdr_off = AQ_PHY_IMAGE_HEADER_CONTENT_OFFSET_HHD;
	primary_iram_ptr = (buf[primary_header_ptr + phy_img_hdr_off + 0x4 + 2] << 16) |
			(buf[primary_header_ptr + phy_img_hdr_off + 0x4 + 1] << 8) |
			buf[primary_header_ptr + phy_img_hdr_off + 0x4];
	primary_iram_sz = (buf[primary_header_ptr + phy_img_hdr_off + 0x7 + 2] << 16) |
			(buf[primary_header_ptr + phy_img_hdr_off + 0x7 + 1] << 8) |
			buf[primary_header_ptr + phy_img_hdr_off + 0x7];
	primary_dram_ptr = (buf[primary_header_ptr + phy_img_hdr_off + 0xA + 2] << 16) |
			(buf[primary_header_ptr + phy_img_hdr_off + 0xA + 1] << 8) |
			buf[primary_header_ptr + phy_img_hdr_off + 0xA];
	primary_dram_sz = (buf[primary_header_ptr + phy_img_hdr_off + 0xD + 2] << 16) |
			(buf[primary_header_ptr + phy_img_hdr_off + 0xD + 1] << 8) |
			buf[primary_header_ptr + phy_img_hdr_off + 0xD];
	primary_iram_ptr += primary_header_ptr;
	primary_dram_ptr += primary_header_ptr;

	aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0x200), 0x1000);
	aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0x200), 0x0);
	computed_crc = 0;
	printf("PHYFW:Loading IRAM...........");
	aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0x202), 0x4000);
	aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0x203), 0x0);
	byte_sz = primary_iram_sz;
	dword_sz = byte_sz >> 2;
	byte_ptr = primary_iram_ptr;
	for (i = 0; i < dword_sz; i++) {
		lsw = (buf[byte_ptr + 1] << 8) | buf[byte_ptr];
		byte_ptr += 2;
		msw = (buf[byte_ptr + 1] << 8) | buf[byte_ptr];
		byte_ptr += 2;
		aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0x204), msw);
		aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0x205), lsw);
		aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0x200), 0xc000);
		msb1 = msw >> 8;
		msb2 = msw & 0xFF;
		lsb1 = lsw >> 8;
		lsb2 = lsw & 0xFF;
		computed_crc = crc16_ccitt(computed_crc, &msb1, 0x1);
		computed_crc = crc16_ccitt(computed_crc, &msb2, 0x1);
		computed_crc = crc16_ccitt(computed_crc, &lsb1, 0x1);
		computed_crc = crc16_ccitt(computed_crc, &lsb2, 0x1);
	}

	switch (byte_sz & 0x3) {
	case 0x1:
		lsw = buf[byte_ptr++];
		msw = 0x0000;
		break;
	case 0x2:
		lsw = (buf[byte_ptr + 1] << 8) | buf[byte_ptr];
		byte_ptr += 2;
		msw = 0x0000;
		break;
	case 0x3:
		lsw = (buf[byte_ptr + 1] << 8) | buf[byte_ptr];
		byte_ptr += 2;
		msw = buf[byte_ptr++];
		break;
	}

	if (byte_sz & 0x3) {
		aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0x204), msw);
		aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0x205), lsw);
		aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0x200), 0xc000);
		msb1 = msw >> 8;
		msb2 = msw & 0xFF;
		lsb1 = lsw >> 8;
		lsb2 = lsw & 0xFF;
		computed_crc = crc16_ccitt(computed_crc, &msb1, 0x1);
		computed_crc = crc16_ccitt(computed_crc, &msb2, 0x1);
		computed_crc = crc16_ccitt(computed_crc, &lsb1, 0x1);
		computed_crc = crc16_ccitt(computed_crc, &lsb2, 0x1);
	}
	printf("done.\n");
	printf("PHYFW:Loading DRAM..............");
	aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0x202), 0x3ffe);
	aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0x203), 0x0);
	byte_sz = primary_dram_sz;
	dword_sz = byte_sz >> 2;
	byte_ptr = primary_dram_ptr;
	for (i = 0; i < dword_sz; i++) {
		lsw = (buf[byte_ptr + 1] << 8) | buf[byte_ptr];
		byte_ptr += 2;
		msw = (buf[byte_ptr + 1] << 8) | buf[byte_ptr];
		byte_ptr += 2;
		aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0x204), msw);
		aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0x205), lsw);
		aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0x200), 0xc000);
		msb1 = msw >> 8;
		msb2 = msw & 0xFF;
		lsb1 = lsw >> 8;
		lsb2 = lsw & 0xFF;
		computed_crc = crc16_ccitt(computed_crc, &msb1, 0x1);
		computed_crc = crc16_ccitt(computed_crc, &msb2, 0x1);
		computed_crc = crc16_ccitt(computed_crc, &lsb1, 0x1);
		computed_crc = crc16_ccitt(computed_crc, &lsb2, 0x1);
	}

	switch (byte_sz & 0x3) {
	case 0x1:
		lsw = buf[byte_ptr++];
		msw = 0x0000;
		break;
	case 0x2:
		lsw = (buf[byte_ptr + 1] << 8) | buf[byte_ptr];
		byte_ptr += 2;
		msw = 0x0000;
		break;
	case 0x3:
		lsw = (buf[byte_ptr + 1] << 8) | buf[byte_ptr];
		byte_ptr += 2;
		msw = buf[byte_ptr++];
		break;
	}

	if (byte_sz & 0x3) {
		aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0x204), msw);
		aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0x205), lsw);
		aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0x200), 0xc000);
		msb1 = msw >> 8;
		msb2 = msw & 0xFF;
		lsb1 = lsw >> 8;
		lsb2 = lsw & 0xFF;
		computed_crc = crc16_ccitt(computed_crc, &msb1, 0x1);
		computed_crc = crc16_ccitt(computed_crc, &msb2, 0x1);
		computed_crc = crc16_ccitt(computed_crc, &lsb1, 0x1);
		computed_crc = crc16_ccitt(computed_crc, &lsb2, 0x1);
	}
	printf("done.\n");
	aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0xc441), 0x201);
	mailbox_crc = aq_phy_reg_read(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0x201));
	if (mailbox_crc != computed_crc)
		printf("phy fw image load  CRC-16 (0x%X) does not match calculated CRC-16 (0x%X)\n", mailbox_crc, computed_crc);
	else
		printf("phy fw image load good CRC-16 matches (0x%X)\n", mailbox_crc);

	aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0x0), 0x0);
	aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0xc001), 0x41);
	aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0xc001), 0x8041);
	mdelay(100);
	aq_phy_reg_write(0x0, phy_addr, AQUANTIA_REG_ADDRESS(0x1e, 0xc001), 0x40);
	mdelay(200);
	aquantia_phy_restart_autoneg(phy_addr);

	printf("FW     : %x.%x.%x\n", \
	            ipq_mdio_read(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(0x1e, 0x20), NULL) >> 8,
	            ipq_mdio_read(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(0x1e, 0x20), NULL) & 0xFF,
	            ipq_mdio_read(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(0x1e, 0xc885), NULL) >> 4);

	return 0;
}

int mfg_phy_reset(void)
{
    printf("GPIO 44 pull low\n");
    (*(volatile unsigned int *) GPIO_IN_OUT_ADDR(44)) = 0x0;
    mdelay(200);
    printf("GPIO 44 pull high\n");
    (*(volatile unsigned int *) GPIO_IN_OUT_ADDR(44)) = 0x2;
    return CMD_RET_SUCCESS;
}

int mfg_phy_status(void)
{
	int ret = 0;
	int retry = 10;

phy_read_an_statys:
	ret = ipq_mdio_read(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0x1), NULL);
	printf("mdio_%s 7.1    = %xh (AN status)\n", eth_get_name(), ret);
	if ((ret & 0x2D) != 0x2D && retry--) {
        printf("AN/link not ready\n");
        mdelay(1500);
        goto phy_read_an_statys;
    }
	else {
	    ret = ipq_mdio_read(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0xc400), NULL);
	    printf("mdio_%s 7.c400 = %xh\n", eth_get_name(), ret);
	    ret = ipq_mdio_read(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0xc800), NULL);
	    printf("mdio_%s 7.c800 = %xh\n", eth_get_name(), ret);
	    switch(ret) {
	        case 0xB:
	            printf("5G_Connected\n");
	            break;
	        case 9:
	            printf("2_5G_Connected\n");
	            break;
	        case 5:
	            printf("1G_Connected\n");
	            break;
	        case 3:
	            printf("100M_Connected\n");
	            break;
	        default:
	            printf("No connection!\n");
	            break;
	    }
	}
/*
	printf("mdio read %s %x.%x = %xh\n", eth_get_name(), MDIO_MMD_AN, MDIO_STAT1, phy_read(phydev, MDIO_MMD_AN, MDIO_STAT1));
	printf("mdio read %s 1.e800 = %d  (link)\n", eth_get_name(), phy_read(phydev, 1, 0xe800));
	printf("mdio read %s 7.0    = %xh (AN ctrl)\n", eth_get_name(), phy_read(phydev, MDIO_MMD_AN, 0));
	printf("Cable Length = %d Meter\n", phy_read(phydev, 0x1e, 0xc884));
*/
    return CMD_RET_SUCCESS;
}

int mfg_phy_neg(char *speed)
{
	printf("%s Using: %s\n", __func__, eth_get_name());

	if (!strcmp(speed, "auto")){
		aquantia_phy_restart_autoneg(AQU_PHY_ADDR);
		printf("SetAutonegotiationControl Auto\n");
	}
	else if (!strcmp(speed, "100")){
		ipq_mdio_write(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0x20), 0x1001);  /* no 5G&2.5G BZ*/
		ipq_mdio_write(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0xc400), 0x54);  /* Advertise only 100Mbps for AQFW 4.0.4 1G IOT bug */
		ipq_mdio_write(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0x0), 0x3200);   /* restart AN */
		/*mdelay(2500); */
		/*printf("mdio read %s 1.e800 = %d (link)\n", eth_get_name(), phy_read(phydev, 1, 0xe800)); */
		/*printf("mdio read %s 7.c400 = %xh (AN 54h=100M)\n", eth_get_name(), phy_read(phydev, MDIO_MMD_AN, 0xc400)); */
		/*printf("mdio read %s 7.c800 = %xh (STS 3h=100M)\n", eth_get_name(), phy_read(phydev, MDIO_MMD_AN, 0xc800)); */
		printf("SetAutonegotiationControl 100\n");
	}
	else if (!strcmp(speed, "1000")){
		ipq_mdio_write(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0x20), 0xa1);
		ipq_mdio_write(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0x3e), 0x0);
		ipq_mdio_write(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0x40), 0x0);
		ipq_mdio_write(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0x20), 0x1);
		ipq_mdio_write(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0xc400), 0x9040);
		ipq_mdio_write(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0x0), 0x3000);
		mdelay(100);
		ipq_mdio_write(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0x0), 0x3200);
		printf("SetAutonegotiationControl 1000\n");
	}
	else if (!strcmp(speed, "2500")){
		ipq_mdio_write(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0x20), 0x141);
		ipq_mdio_write(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0x3e), 0x1);
		ipq_mdio_write(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0x40), 0x8);
		ipq_mdio_write(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0x20), 0xa1);
		ipq_mdio_write(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0xc400), 0x1440);
		ipq_mdio_write(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0x0), 0x3000);
		mdelay(100);
		ipq_mdio_write(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0x0), 0x3200);
		printf("SetAutonegotiationControl 2500\n");
	}
	else if (!strcmp(speed, "5000")){
		ipq_mdio_write(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0x20), 0x141);
		ipq_mdio_write(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0x3e), 0x2);
		ipq_mdio_write(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0x40), 0x4);
		ipq_mdio_write(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0x20), 0x141);
		ipq_mdio_write(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0xc400), 0x1840);
		ipq_mdio_write(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0x0), 0x3000);
		mdelay(100);
		ipq_mdio_write(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0x0), 0x3200);
		printf("SetAutonegotiationControl 5000\n");
	}
	else {
		mfg_phy_reset();
		return CMD_RET_USAGE;
	}
    return CMD_RET_SUCCESS;
}

int mfg_phy_id_get(uchar id_only)
{
	printf("PhyID=%x%0x FW=%x.%x.%x (provision=%x)\n", \
			ipq_mdio_read(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0x2), NULL), \
			ipq_mdio_read(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0x3), NULL), \
			ipq_mdio_read(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(0x1e, 0x20), NULL) >> 8, \
			ipq_mdio_read(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(0x1e, 0x20), NULL) & 0xFF, \
			ipq_mdio_read(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(0x1e, 0xc885), NULL) >> 4, \
			ipq_mdio_read(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(0x1e, 0xc885), NULL) & 0xF);
	if (!id_only) {
		printf(" AN_DEV1 = %04xh\n", ipq_mdio_read(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0x5), NULL));
		printf(" AN_DEV2 = %04xh\n", ipq_mdio_read(AQU_PHY_ADDR, AQUANTIA_REG_ADDRESS(MDIO_MMD_AN, 0x6), NULL));
	}
    return CMD_RET_SUCCESS;
}

static int mfg_aqr_test (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int i;
	int result = 0;

	if (argc < 2 || argc > 3) {
		return CMD_RET_USAGE;
	}
	if (argc == 2) {
		if (!strcmp(argv [1], "reset"))
			mfg_phy_reset();
		else if (!strcmp(argv [1], "id"))
			mfg_phy_id_get(0);
		else if (!strcmp(argv [1], "status"))
			mfg_phy_status();
		else
			return CMD_RET_USAGE;
	} else if (argc == 3) {
		if (!strcmp(argv [1], "speed"))
			mfg_phy_neg(argv[2]);
		else
			return CMD_RET_USAGE;
	} else
		return CMD_RET_USAGE;

    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	aq_test,	7,	1,	mfg_aqr_test,
	"aquantia phy test\n",
	"<reset/id/status/speed> [auto/100/1000/2500/5000]\n"
);

U_BOOT_CMD(
	aq_load_fw,	5,	1,	do_load_fw,
	"LOAD aq-fw-binary",
	""
);

U_BOOT_CMD(
	aq_phy_restart,	5,	1,	do_aq_phy_restart,
	"Restart Aquantia phy",
	""
);
