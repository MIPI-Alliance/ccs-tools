/* Copyright (C) 2020 MIPI Alliance */
/* Copyright (C) 2019--2020 Intel Corporation */
/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef __CCS_OS_H__
#define __CCS_OS_H__

#include <errno.h>
#include <limits.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define align2(n, a) (((n) & ~((a) - 1)) + (a))

#define array_length(a) (sizeof(a) / sizeof(*(a)))

#define os_printf fprintf

typedef FILE * printf_ctx;

#define os_calloc(a) calloc(1, a)

#define os_free(a) free(a)

#endif /* __CCS_OS_H__ */
