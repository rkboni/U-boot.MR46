/*
 * Copyright (c) 2015-2017 The Linux Foundation. All rights reserved.
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
#include <command.h>
#include <image.h>
#include <nand.h>
#include <errno.h>
#include <asm/arch-qca-common/smem.h>
#include <asm/arch-qca-common/scm.h>
#include <linux/mtd/ubi.h>
#include <part.h>
#include "qca_common.h"

#define FUSEPROV_SUCCESS		0x0
#define FUSEPROV_INVALID_HASH		0x09
#define FUSEPROV_SECDAT_LOCK_BLOWN	0xB
#define TZ_BLOW_FUSE_SECDAT		0x20

typedef struct {
	unsigned int image_type;
	unsigned int header_vsn_num;
	unsigned int image_src;
	unsigned int image_dest_ptr;
	unsigned int image_size;
	unsigned int code_size;
	unsigned int signature_ptr;
	unsigned int signature_size;
	unsigned int cert_chain_ptr;
	unsigned int cert_chain_size;
} mbn_header_t;


int do_fuseipq(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret;
	uint32_t fuse_status = 0;
	struct fuse_blow {
		uint32_t address;
		uint32_t status;
	} fuseip;

	if (argc != 2) {
		printf("No Arguments provided\n");
		printf("Command format: fuseipq <address>\n");
		return 1;
	}

	fuseip.address = simple_strtoul(argv[1], NULL, 16);
	fuseip.status = (uint32_t)&fuse_status;

	ret = qca_scm_fuseipq(SCM_SVC_FUSE, TZ_BLOW_FUSE_SECDAT,
			&fuseip, sizeof(fuseip));


	if (ret || fuse_status)
		printf("%s: Error in QFPROM write (%d, %d)\n",
			__func__, ret, fuse_status);

	if (fuse_status == FUSEPROV_SECDAT_LOCK_BLOWN)
		printf("Fuse already blown\n");
	else if (fuse_status == FUSEPROV_INVALID_HASH)
		printf("Invalid sec.dat\n");
	else if (fuse_status  != FUSEPROV_SUCCESS)
		printf("Failed to Blow fuses");
	else
		printf("Blow Success\n");

	return 0;
}

U_BOOT_CMD(fuseipq, 2, 0, do_fuseipq,
		"fuse QFPROM registers from memory\n",
		"fuseipq [address]  - Load fuse(s) and blows in the qfprom\n");

static int do_checksecboot(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret;
	char buf;

	ret = qca_scm_call(SCM_SVC_FUSE, QFPROM_IS_AUTHENTICATE_CMD,
			   &buf, sizeof(char));
	if (ret) {
		printf("error: %d\n", ret);
		return CMD_RET_FAILURE;
	}
	if (buf == 1)
		printf("secure boot enabled\n");
	else
		printf("secure boot not enabled\n");
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(check_secboot, 1, 0, do_checksecboot,
	   "check secure boot enabled",
	   "check_secboot - prints if secure boot is enabled\n");

#define CE1_REG_USAGE		(0)
#define CE1_ADM_USAGE		(1)
#define CE1_RESOURCE		(1)

static int switch_ce_channel_buf(unsigned int channel_id)
{
	int ret = 0;
#if defined(CONFIG_ARCH_IPQ806x)
	struct switch_ce_chn_buf_t {
		unsigned int resource;
		unsigned int channel_id;
	} ce1_chn_buf;

	ce1_chn_buf.resource   = CE1_RESOURCE;
	ce1_chn_buf.channel_id = channel_id;

	ret = scm_call(SCM_SVC_TZ, CE_CHN_SWITCH_CMD, &ce1_chn_buf,
		sizeof(ce1_chn_buf), NULL, 0);
#endif
	return ret;
}

static int do_validate(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret;
	struct mbn {
		unsigned int addr;
		unsigned int size;
	} mbn;
	unsigned int mbn_size;

	if (argc < 2)
		return CMD_RET_USAGE;

#if defined(CONFIG_ARCH_IPQ806x)
	/* Akronite TZ requires validating at 0x44000000 and the size parameter
	 * to KERNEL_AUTH_CMD is reqd; compute from mbn header
	 */
	mbn.addr = 0x44000000;
	mbn_header_t *hdr = (mbn_header_t *)mbn.addr;
	mbn_size = (hdr->cert_chain_ptr - mbn.addr) + hdr->cert_chain_size;
	mbn.size = mbn_size;
#elif defined(CONFIG_ARCH_IPQ40xx)
	mbn.addr = simple_strtoul(argv[1], NULL, 16);
	mbn_header_t *hdr = (mbn_header_t *)mbn.addr;
	mbn_size = hdr->image_size;
	mbn.size = 0;
#elif defined(CONFIG_ARCH_IPQ807x)
	mbn.addr = simple_strtoul(argv[1], NULL, 16);
	mbn_header_t *hdr = (mbn_header_t *)mbn.addr;
	mbn_size = hdr->image_size;
	mbn.size = mbn_size;
#else
#error Unrecognized platform; support for secure boot?
#endif

	/* 100MB size limit */
	if (mbn_size > 0x6400000)
		return CMD_RET_FAILURE;

	/* This printf() is a crucial fix on the Akronite (ipq806x) platform.
	 * Without this print(), the KERNEL_AUTH_CMD TZ function hangs.
	 * One hypothesis is it avoids a race condition between loading the
	 * image from nand and running the XPU signing validation in TZ;
	 * the problem has not been disclosed by the vendor.
	 */
	printf("Validating %x bytes @ addr %x\n", mbn_size, mbn.addr);

	/* This sys call will switch the CE1 channel to register usage */
	ret = switch_ce_channel_buf(CE1_REG_USAGE);
	if (ret) {
		printf("switch ce to reg failure: %d\n", ret);
		return CMD_RET_FAILURE;
	}

	ret = qca_scm_auth_kernel(&mbn, sizeof(struct mbn));
	if (ret) {
		printf("Validate error: %d\n", ret);
		return CMD_RET_FAILURE;
	}

	/*
	 * This sys call will switch the CE1 channel to ADM usage
	 * so that HLOS can use it.
	 */
	ret = switch_ce_channel_buf(CE1_ADM_USAGE);
	if (ret) {
		printf("swtich ce to adm failure: %d\n", ret);
		return CMD_RET_FAILURE;
	}

	printf("Valid image\n");
	env_set_hex("mbnsize", mbn.size);
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(validate, 2, 0, do_validate,
	   "Validate signed mbn file",
	   "validate <addr> [size] - check signed mbn file @ given addr");

int do_chainload(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	ulong iflag = 0;
	char cmd[128];

	if (argc < 2)
		return CMD_RET_USAGE;

	iflag = bootm_disable_interrupts();

	sprintf(cmd, "go %s", argv[1]);
	run_command(cmd, 0);
	/* should not return here */

	if (iflag)
		enable_interrupts();
	printf("ERROR: chainloading @ %s\n", argv[1]);
	return CMD_RET_FAILURE;
}

U_BOOT_CMD(chainload, 2, 0, do_chainload,
		"go at supplied address after disabling interrupts\n",
		"chainload [address]  - disable interrupts and jump to secondary program\n");
