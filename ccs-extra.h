/* Copyright (C) 2020 MIPI Alliance */
/* Copyright (C) 2019--2020 Intel Corporation */
/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef __CCS_EXTRA_H__
#define __CCS_EXTRA_H__

struct ccs_license {
	const char *filename;
	const char *text;
	size_t length;
};

struct ccs_reg_arg {
	const char *name;
	uint8_t min, max;
	uint8_t elsize;
};

struct ccs_reg_desc {
	uint32_t reg;
	uint16_t size;
	uint8_t num_args;
	const char *name;
	const struct ccs_reg_arg *args;
};

#endif
