/*
 * Copyright (c) 2011-2012, 2016-2017 The Linux Foundation. All rights reserved.
 * Source : APQ8064 LK Boot
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *  * Neither the name of Code Aurora Forum, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
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

#ifndef GPIO_H
#define GPIO_H

#ifndef CONFIG_ARCH_IPQ807x
/* GPIO TLMM: Direction */
#define GPIO_INPUT	0
#define GPIO_OUTPUT	1

/* GPIO TLMM: Output value */
#define GPIO_OUT_LOW	0
#define GPIO_OUT_HIGH	1

/* GPIO TLMM: Pullup/Pulldown */
#define GPIO_NO_PULL    0
#define GPIO_PULL_DOWN  1
#define GPIO_PULL_UP	2
#define GPIO_NOT_DEF	3

/* GPIO TLMM: Drive Strength */
#define GPIO_2MA        0
#define GPIO_4MA        1
#define GPIO_6MA        2
#define GPIO_8MA        3
#define GPIO_10MA       4
#define GPIO_12MA       5
#define GPIO_14MA       6
#define GPIO_16MA       7

/* GPIO TLMM: Status */
#define GPIO_OE_DISABLE	0
#define GPIO_OE_ENABLE	1

/* GPIO VM */
#define GPIO_VM_ENABLE	1
#define GPIO_VM_DISABLE	0

/* GPIO OD */
#define GPIO_OD_ENABLE	1
#define GPIO_OD_DISABLE	0

/* GPIO PULLUP RES */
#define GPIO_PULL_RES0	0
#define GPIO_PULL_RES1	1
#define GPIO_PULL_RES2	2
#define GPIO_PULL_RES3	3

#define GPIO_IN		(1 << 0)
#endif
#define GPIO_OUT	(1 << 1)

struct qca_gpio_config {
	unsigned int gpio;
	unsigned int func;
	unsigned int out;
	unsigned int pull;
	unsigned int drvstr;
	unsigned int oe;
	unsigned int vm;
	unsigned int od_en;
	unsigned int pu_res;
};

#ifdef CONFIG_ARCH_IPQ807x
void gpio_tlmm_config(struct qca_gpio_config *gpio_config);
#else
void gpio_tlmm_config(unsigned int gpio, unsigned int func,
		unsigned int out, unsigned int pull,
		unsigned int drvstr, unsigned int oe,
		unsigned int gpio_vm, unsigned int gpio_od_en,
		unsigned int gpio_pu_res);
#endif

void gpio_set_value(unsigned int gpio, unsigned int out);

int gpio_get_value(unsigned int gpio);

int qca_gpio_init(int offset);
int qca_gpio_deinit(int offset);

#endif
