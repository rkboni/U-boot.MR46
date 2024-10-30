/*
 * Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <version.h>
#include <linux/compiler.h>
#ifdef CONFIG_SYS_COREBOOT
#include <asm/arch/sysinfo.h>
#endif

const char __weak version_string[] = U_BOOT_VERSION_STRING;

static int do_version(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	printf(display_options_get_banner(false, buf, sizeof(buf)));
	printf("\n%s\n", version_string);
#ifdef CONFIG_MERAKI_VERSION
#ifndef IKAREM_U_BOOT_VERSION
# error "Meraki u-boot version string not found"
#endif
	puts(IKAREM_U_BOOT_VERSION "\n");
#endif
#ifdef CC_VERSION_STRING
	puts(CC_VERSION_STRING "\n");
#endif
#ifdef LD_VERSION_STRING
	puts(LD_VERSION_STRING "\n");
#endif
#ifdef CONFIG_SYS_COREBOOT
	printf("coreboot-%s (%s)\n", lib_sysinfo.version, lib_sysinfo.build);
#endif
	return 0;
}

U_BOOT_CMD(
	version,	1,		1,	do_version,
	"print monitor, compiler and linker version",
	""
);
