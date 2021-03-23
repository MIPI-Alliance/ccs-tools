/* Copyright (C) 2020 MIPI Alliance */
/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (C) 2019--2020 Intel Corporation */

#ifndef __CCS_DATA_H__
#define __CCS_DATA_H__

#include <stdint.h>

#include "ccs-os.h"

/**
 * struct ccs_data_block_version - CCS static data version
 * @version_major: Major version number
 * @version_minor: Minor version number
 * @date_year: Year
 * @date_month: Month
 * @date_day: Day
 */
struct ccs_data_block_version {
	uint16_t version_major;
	uint16_t version_minor;
	uint16_t date_year;
	uint8_t date_month;
	uint8_t date_day;
};

/**
 * struct ccs_reg - CCS register value
 * @addr: The 16-bit address of the register
 * @len: Length of the data
 * @value: Data
 */
struct ccs_reg {
	uint16_t addr;
	uint16_t len;
	uint8_t *value;
};

/**
 * struct ccs_if_rule - CCS static data if rule
 * @addr: Register address
 * @value: Register value
 * @mask: Value applied to both actual register value and @value
 */
struct ccs_if_rule {
	uint16_t addr;
	uint8_t value;
	uint8_t mask;
};

/**
 * struct ccs_frame_format_desc - CCS frame format descriptor
 * @pixelcode: The pixelcode; CCS_DATA_BLOCK_FFD_PIXELCODE_*
 * @value: Value related to the pixelcode
 */
struct ccs_frame_format_desc {
	uint8_t pixelcode;
	uint16_t value;
};

/**
 * struct ccs_frame_format_descs - A series of CCS frame format descriptors
 * @num_column_descs: Number of column descriptors
 * @num_row_descs: Number of row descriptors
 * @column_descs: Column descriptors
 * @row_descs: Row descriptors
 */
struct ccs_frame_format_descs {
	uint8_t num_column_descs;
	uint8_t num_row_descs;
	struct ccs_frame_format_desc *column_descs;
	struct ccs_frame_format_desc *row_descs;
};

/**
 * struct ccs_pdaf_readout - CCS PDAF data readout descriptor
 * @pdaf_readout_info_order: PDAF readout order
 * @ffd: Frame format of PDAF data
 */
struct ccs_pdaf_readout {
	uint8_t pdaf_readout_info_order;
	struct ccs_frame_format_descs *ffd;
};

/**
 * struct ccs_rule - A CCS static data rule
 * @num_if_rules: Number of if rules
 * @if_rules: If rules
 * @num_read_only_regs: Number of read-only registers
 * @read_only_regs: Read-only registers
 * @num_manufacturer_regs: Number of manufacturer-specific registers
 * @manufacturer_regs: Manufacturer-specific registers
 * @frame_format: Frame format
 * @pdaf_readout: PDAF readout
 */
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

/**
 * struct ccs_pdaf_pix_loc_block_desc - PDAF pixel location block descriptor
 * @block_type_id: Block type identifier, from 0 to n
 * @repeat_x: Number of times this block is repeated to right
 */
struct ccs_pdaf_pix_loc_block_desc {
	uint8_t block_type_id;
	uint16_t repeat_x;
};

/**
 * struct ccs_pdaf_pix_loc_block_desc_group - PDAF pixel location block
 *					      descriptor group
 * @repeat_y: Number of times the group is repeated down
 * @num_block_descs: Number of block descriptors in @block_descs
 * @block_descs: Block descriptors
 */
struct ccs_pdaf_pix_loc_block_desc_group {
	uint8_t repeat_y;
	uint16_t num_block_descs;
	struct ccs_pdaf_pix_loc_block_desc *block_descs;
};

/**
 * struct ccs_pdaf_pix_loc_pixel_desc - PDAF pixel location pixel descriptor
 * @pixel_type: Type of the pixel; CCS_DATA_PDAF_PIXEL_TYPE_*
 * @small_offset_x: offset X coordinate
 * @small_offset_y: offset Y coordinate
 */
struct ccs_pdaf_pix_loc_pixel_desc {
	uint8_t pixel_type;
	uint8_t small_offset_x;
	uint8_t small_offset_y;
};

/**
 * struct ccs_pdaf_pix_loc_pixel_desc_group - PDAF pixel location pixel
 *					      descriptor group
 * @num_descs: Number of descriptors in @descs
 * @descs: PDAF pixel location pixel descriptors
 */
struct ccs_pdaf_pix_loc_pixel_desc_group {
	uint8_t num_descs;
	struct ccs_pdaf_pix_loc_pixel_desc *descs;
};

/**
 * struct ccs_pdaf_pix_loc - PDAF pixel locations
 * @main_offset_x: Start X coordinate of PDAF pixel blocks
 * @main_offset_y: Start Y coordinate of PDAF pixel blocks
 * @global_pdaf_type: PDAF pattern type
 * @block_width: Width of a block in pixels
 * @block_height: Heigth of a block in pixels
 * @num_block_desc_groups: Number of block descriptor groups
 * @block_desc_groups: Block descriptor groups
 * @num_pixel_desc_grups: Number of pixel descriptor groups
 * @pixel_desc_groups: Pixel descriptor groups
 */
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

/**
 * struct ccs_data_container - In-memory CCS static data
 * @version: CCS static data version
 * @num_sensor_read_only_regs: Number of the read-only registers for the sensor
 * @sensor_read_only_regs: Read-only registers for the sensor
 * @num_sensor_manufacturer_regs: Number of the manufacturer-specific registers
 *				  for the sensor
 * @sensor_manufacturer_regs: Manufacturer-specific registers for the sensor
 * @num_sensor_rules: Number of rules for the sensor
 * @sensor_rules: Rules for the sensor
 * @num_module_read_only_regs: Number of the read-only registers for the module
 * @module_read_only_regs: Read-only registers for the module
 * @num_module_manufacturer_regs: Number of the manufacturer-specific registers
 *				  for the module
 * @module_manufacturer_regs: Manufacturer-specific registers for the module
 * @num_module_rules: Number of rules for the module
 * @module_rules: Rules for the module
 * @sensor_pdaf: PDAF data for the sensor
 * @module_pdaf: PDAF data for the module
 * @license_length: Lenght of the license data
 * @license: License data
 * @end: Whether or not there's an end block
 * @backing: Raw data, pointed to from elsewhere so keep it around
 */
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
