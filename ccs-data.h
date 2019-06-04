/* Copyright (C) 2020 MIPI Alliance */
/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (C) 2019--2020 Intel Corporation */

#ifndef __CCS_DATA_H__
#define __CCS_DATA_H__

#include "ccs-os.h"

struct ccs_data_block_version {
	uint16_t version_major;
	uint16_t version_minor;
	uint16_t date_year;
	uint8_t date_month;
	uint8_t date_day;
};

struct ccs_reg {
	uint16_t addr;
	uint16_t len;
	uint8_t *value;
};

struct ccs_if_rule {
	uint16_t addr;
	uint8_t value;
	uint8_t mask;
};

struct ccs_frame_format_desc {
	uint8_t pixelcode;
	uint16_t value;
};

struct ccs_frame_format_descs {
	uint8_t num_column_descs;
	uint8_t num_row_descs;
	struct ccs_frame_format_desc *column_descs;
	struct ccs_frame_format_desc *row_descs;
};

struct ccs_pdaf_readout {
	uint8_t pdaf_readout_info_order;
	struct ccs_frame_format_descs *ffd;
};

struct ccs_rule {
	size_t num_if_rules;
	struct ccs_if_rule *if_rules;
	size_t num_read_only_regs;
	struct ccs_reg *read_only_regs;
	size_t num_manufacturer_regs;
	struct ccs_reg *manufacturer_regs;
	struct ccs_frame_format_descs *frame_format;
	struct ccs_pdaf_readout *pdaf_readout;
};

struct ccs_pdaf_pix_loc_block_desc {
	uint8_t block_type_id;
	uint16_t repeat_x;
};

struct ccs_pdaf_pix_loc_block_desc_group {
	uint8_t repeat_y;
	uint16_t num_block_descs;
	struct ccs_pdaf_pix_loc_block_desc *block_descs;
};

struct ccs_pdaf_pix_loc_pixel_desc {
	uint8_t pixel_type;
	uint8_t small_offset_x;
	uint8_t small_offset_y;
};

struct ccs_pdaf_pix_loc_pixel_desc_group {
	uint8_t num_descs;
	struct ccs_pdaf_pix_loc_pixel_desc *descs;
};

struct ccs_pdaf_pix_loc {
	uint16_t main_offset_x;
	uint16_t main_offset_y;
	uint8_t global_pdaf_type;
	uint8_t block_width;
	uint8_t block_height;
	uint16_t num_block_desc_groups;
	struct ccs_pdaf_pix_loc_block_desc_group *block_desc_groups;
	uint8_t num_pixel_desc_grups;
	struct ccs_pdaf_pix_loc_pixel_desc_group *pixel_desc_groups;
};

struct ccs_data_container {
	struct ccs_data_block_version *version;
	size_t num_sensor_read_only_regs;
	struct ccs_reg *sensor_read_only_regs;
	size_t num_sensor_manufacturer_regs;
	struct ccs_reg *sensor_manufacturer_regs;
	size_t num_sensor_rules;
	struct ccs_rule *sensor_rules;
	size_t num_module_read_only_regs;
	struct ccs_reg *module_read_only_regs;
	size_t num_module_manufacturer_regs;
	struct ccs_reg *module_manufacturer_regs;
	size_t num_module_rules;
	struct ccs_rule *module_rules;
	struct ccs_pdaf_pix_loc *sensor_pdaf;
	struct ccs_pdaf_pix_loc *module_pdaf;
	size_t license_length;
	char *license;
	bool end;
	void *backing;
};

int ccs_data_parse(struct ccs_data_container *ccsdata, const void *data,
		   size_t len, printf_ctx fh, bool verbose);

#endif /* __CCS_DATA_H__ */
