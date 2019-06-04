/* Copyright (C) 2020 MIPI Alliance */
/* Copyright (C) 2019--2020 Intel Corporation */
/* SPDX-License-Identifier: BSD-3-Clause */

#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ccs-data.h"
#include "ccs-extra.h"
#include "ccs-os.h"
#include "ccs-regs.h"

#define CHUNK_SIZE	(1024 * 16)

#define FIRST_MSR	0x3000
#define DEFAULT_MASK	0xff

#define INDENT_BASE	40

#define TAB_WIDTH	8

extern struct ccs_license ccs_licenses[];
extern struct ccs_reg_desc ccs_reg_desc[];

static void tabify(size_t pos, char *buf)
{
	bool non_space_seen = false;

	pos &= TAB_WIDTH - 1;
	buf -= pos;

again:
	while (buf[pos]) {
		size_t i, last = 0, first = TAB_WIDTH;

		for (i = pos; i < TAB_WIDTH && buf[i]; i++) {
			if (buf[i] == '\t') {
				buf++;
				pos = 0;
				goto again;
			}

			if (buf[i] == '-') {
				first = i + 1;
				continue;
			}

			if (buf[i] != ' ') {
				non_space_seen = true;
				first = i + 1;
				continue;
			}

			if (!non_space_seen)
				continue;

			last = i;
			if (first > i)
				first = i;
		}

		if (last == TAB_WIDTH - 1 && first <= last) {
			buf += first;
			buf[pos] = '\t';
			strcpy(buf + 1, buf + last + 1 - first);
			buf++;
		} else if (i == TAB_WIDTH) {
			buf += TAB_WIDTH;
		} else {
			break;
		}

		pos = 0;
	}
}

#define __tab_fprintf(file, pos, ...)					\
	({								\
		char __my_buf[256];					\
		snprintf(__my_buf, sizeof(__my_buf), __VA_ARGS__);	\
		tabify(pos, __my_buf);					\
		fputs(__my_buf, file);					\
	})

#define tab_fprintf(file, ...) __tab_fprintf(file, 0, __VA_ARGS__)

static int print_code_string(FILE *file, unsigned int pos,
			     const char * const *strings,
			     size_t array_length, unsigned int code,
			     const char * const format, unsigned int width)
{
	if (code >= array_length || !strings[code])
		return fprintf(file, "%u", code);

	if (width)
		return __tab_fprintf(file, pos, format, width, strings[code]);

	return __tab_fprintf(file, pos, format, strings[code]);
}

static void print_mapping(FILE *file, const char * const key,
			  unsigned int indent)
{
	tab_fprintf(file, "%*s%s:\n", indent, "", key);
}

static void print_key(FILE *file, const char * const key, unsigned int indent,
		      const char * const format, uint32_t value)
{
	tab_fprintf(file, "%*s%-*s: ", indent, "", INDENT_BASE - indent, key);
	tab_fprintf(file, format, value);
	fputc('\n', file);
}

static void print_key_str(FILE *file, const char * const key,
			  unsigned int indent, const char * const format,
			  const char * const value)
{
	tab_fprintf(file, "%*s%-*s: ", indent, "", INDENT_BASE - indent, key);
	tab_fprintf(file, format, value);
	fputc('\n', file);
}

static void print_key_stringentry(FILE *file, const char * const key,
				  unsigned int indent,
				  const char * const *strings,
				  size_t array_length, uint32_t value)
{
	tab_fprintf(file, "%*s%-*s: ", indent, "", INDENT_BASE - indent, key);
	print_code_string(file, indent, strings, array_length, value, "%s", 0);
	fputc('\n', file);
}

static void print_data_version(const struct ccs_data_block_version *v,
			       FILE *file)
{
	fputc('\n', file);
	print_mapping(file, "data-version", 0);
	print_key(file, "version-major", 2, "%u", v->version_major);
	print_key(file, "version-minor", 2, "%u", v->version_minor);
	print_key(file, "date-year", 2, "%4.4u", v->date_year);
	print_key(file, "date-month", 2, "%2.2u", v->date_month);
	print_key(file, "date-day", 2, "%2.2u", v->date_day);
}

static unsigned int reg_bits(const struct ccs_reg_desc *reg)
{
	if (reg->flags & CCS_FL_16BIT)
		return 16;

	if (reg->flags & CCS_FL_32BIT)
		return 32;

	return 8;
}

static const struct ccs_reg_desc *get_reg_desc(uint16_t addr)
{
	const struct ccs_reg_desc *rdesc = ccs_reg_desc;

	while (rdesc->name) {
		if (addr >= rdesc->addr &&
		    addr <= rdesc->addr + rdesc->size - 1)
			return rdesc;

		rdesc++;
	}

	return NULL;
}

static int print_regs(const struct ccs_reg *regs, unsigned int num,
		      const char * const name, unsigned int indent,
		      FILE *file)
{
	unsigned int i;

	fputc('\n', file);
	print_mapping(file, name, indent);

	for (i = 0; i < num; i++) {
		struct ccs_reg reg = regs[i];
		unsigned int j;

		if (reg.addr >= FIRST_MSR) {
			for (j = 0; j < reg.len; ) {
				unsigned int k;

				tab_fprintf(file, "%*s- 0x%4.4x%*s",
					    indent + 2, "", reg.addr + j,
					    INDENT_BASE - indent - 10, "");

				fputs("0x", file);

				for (k = j + 4; j < k && j < reg.len; j++)
					fprintf(file, "%2.2x", reg.value[j]);

				fputc('\n', file);
			}

			continue;
		}

		do {
			const struct ccs_reg_desc *rdesc =
				get_reg_desc(reg.addr);
			unsigned int bits, i;
			char __buf[32] = { 0 }, *buf = __buf;
			uint16_t offset;

			if (!rdesc) {
				fprintf(stderr,
					"cannot find register at 0x%4.4x\n",
					reg.addr);
				return -ENOENT;
			}

			offset = reg.addr - rdesc->addr;

			bits = reg_bits(rdesc);

			if (rdesc->num_args)
				buf += snprintf(buf,
						sizeof(__buf) - (buf - __buf),
						"(");

			for (i = 0; i < rdesc->num_args; i++) {
				const struct ccs_reg_arg *rarg =
					&rdesc->args[i];
				uint16_t n;

				n = offset / rarg->elsize;
				n += rarg->min;
				if (n < rarg->min || n > rarg->max) {
					fprintf(stderr,
						"internal error, addr 0x%4.4x; register %s, argument %s, min %u, max %u, value %u\n",
						reg.addr, rdesc->name,
						rarg->name, rarg->min,
						rarg->max, n);
					return -EINVAL;
				}

				buf += snprintf(buf,
						sizeof(__buf) - (buf - __buf),
						"%u", n);
				if (i + 1 < rdesc->num_args)
					buf += snprintf(buf,
							sizeof(__buf) -
							(buf - __buf),
							", ");

				offset -= n * rarg->elsize;
			}

			if (rdesc->num_args)
				buf += snprintf(buf,
						sizeof(__buf) - (buf - __buf),
						")");

			tab_fprintf(file, "%*s- %s%-*s 0x", indent + 2, "",
				    rdesc->name,
				    INDENT_BASE - 5 - indent -
				    (int)strlen(rdesc->name),
				    __buf);

			if (bits >> 3 > reg.len) {
				fprintf(stderr,
					"not enough data for addr 0x%4.4x\n",
					reg.addr);
				return -ENODATA;
			}

			for (j = 0; j < bits >> 3; j++)
				fprintf(file, "%2.2x", reg.value[j]);

			reg.addr += bits >> 3;
			reg.value += bits >> 3;
			reg.len -= bits >> 3;

			fputc('\n', file);
		} while (reg.len);

	}

	return 0;
}

static int print_ffd(const struct ccs_frame_format_descs *descs,
		     const char * const name, unsigned int indent, FILE *file)
{
	static const char * const pixelcodes[] = {
		NULL,				/* 0 */
		"embedded",
		"dummy",
		"black",
		"dark",
		"visible",			/* 5 */
		NULL,
		NULL,
		"manufacturer-specific-0",
		"manufacturer-specific-1",
		"manufacturer-specific-2",	/* 10 */
		"manufacturer-specific-3",
		"manufacturer-specific-4",
		"manufacturer-specific-5",
		"manufacturer-specific-6",
		NULL,				/* 15 */
		"top-OB",
		"bottom-OB",
		"left-OB",
		"right-OB",
		"top-left-OB",			/* 20 */
		"top-right-OB",
		"bottom-left-OB",
		"bottom-right-OB",
		"total",
		NULL,				/* 25 */
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,				/* 30 */
		NULL,
		"top-PDAF",
		"bottom-PDAF",
		"left-PDAF",
		"right-PDAF",			/* 35 */
		"top-left-PDAF",
		"top-right-PDAF",
		"bottom-left-PDAF",
		"bottom-right-PDAF",
		"separated-PDAF",		/* 40 */
		"original-order-PDAF",
		"vendor-PDAF",
	};
	const struct {
		struct ccs_frame_format_desc * const desc;
		size_t num;
		const char * const name;
	} ptrs[] = {
		{
			.desc = descs->column_descs,
			.num = descs->num_column_descs,
			.name = "columns",
		}, {
			.desc = descs->row_descs,
			.num = descs->num_row_descs,
			.name = "rows",
		}
	}, *ptr = ptrs;
	unsigned int i;

	fputc('\n', file);
	print_mapping(file, name, indent);

	for (i = 0; i < array_length(ptrs); i++, ptr++) {
		unsigned int j;

		print_mapping(file, ptr->name, indent + 2);

		for (j = 0; j < ptr->num; j++) {
			tab_fprintf(file, "%*s- ", indent + 4, "");

			print_code_string(file, indent + 6, pixelcodes,
					  array_length(pixelcodes),
					  ptr->desc[j].pixelcode, "%-*s",
					  INDENT_BASE - 6 - indent);

			fprintf(file, "%u\n", ptr->desc[j].value);
		}
	}

	return 0;
}

static int print_pdaf_readout(const struct ccs_pdaf_readout *pdaf_readout,
			      const char * const name, unsigned int indent,
			      FILE *file)
{
	static const char * const readout_types[] = {
		NULL,
		"original-order",
		"separate-line-order",
		"separate-types-separate-lines",
	};

	fputc('\n', file);
	print_mapping(file, name, indent);

	print_key_stringentry(file, "pdaf-readout-info", indent + 2,
			      readout_types, array_length(readout_types),
			      pdaf_readout->pdaf_readout_info_order);

	return print_ffd(pdaf_readout->ffd, "frame-format-descriptor",
			 indent + 2, file);
}

static int print_rules(struct ccs_rule *rules, unsigned int num,
		       const char * const name, unsigned int indent,
		       FILE *file)
{
	unsigned int i;

	fputc('\n', file);
	print_mapping(file, name, indent);

	for (i = 0; i < num; i++, rules++) {
		unsigned int j;
		int rval;

		if (i)
			fputc('\n', file);

		print_mapping(file, "- condition", indent + 2);

		for (j = 0; j < rules->num_if_rules; j++) {
			const struct ccs_reg_desc *rdesc =
				get_reg_desc(rules->if_rules[j].addr);
			char buf[7];
			const char * str = buf;

			if (!rdesc)
				snprintf(buf, sizeof(buf), "0x%4.4x",
					 rules->if_rules[j].addr);
			else
				str = rdesc->name;

			tab_fprintf(file, "%*s - %-*s0x%2.2x",
				    indent + 5, "", INDENT_BASE - 8 - indent,
				    str, rules->if_rules[j].value);
			if (rules->if_rules[j].mask != DEFAULT_MASK)
				fprintf(file, "\t0x%2.2x",
					rules->if_rules[j].mask);
			fputc('\n', file);
		}

		if (rules->read_only_regs) {
			rval = print_regs(rules->read_only_regs,
					  rules->num_read_only_regs,
					  "read-only-regs", 4, file);
			if (rval)
				return rval;
		}

		if (rules->frame_format) {
			rval = print_ffd(rules->frame_format,
					 "frame-format-descriptor", 4, file);
			if (rval)
				return rval;
		}

		if (rules->manufacturer_regs) {
			rval = print_regs(rules->manufacturer_regs,
					  rules->num_manufacturer_regs,
					  "manufacturer-regs", 4, file);
			if (rval)
				return rval;
		}

		if (rules->pdaf_readout) {
			rval = print_pdaf_readout(rules->pdaf_readout,
						  "pdaf-readout", 4, file);
		}
	}

	return 0;
}

static int print_pdaf_pixel_location(const struct ccs_pdaf_pix_loc *pdaf,
				     const char * const name,
				     unsigned int indent, FILE *file)
{
	const char * const pdaf_types[] = {
		"separated",
		"side-by-side",
		"multiple side-by-side",
	};
	const char * const pixel_types[] = {
		"left-separated",
		"right-separated",
		"top-separated",
		"bottom-separated",
		"left-side-by-side",
		"right-side-by-side",
		"top-side-by-side",
		"bottom-side-by-side",
		"top-left",
		"top-right",
		"bottom-left",
		"bottom-right",
	};
	unsigned int i;

	fputc('\n', file);
	print_mapping(file, name, indent);

	indent += 2;

	print_key(file, "main-offset-x", indent, "%u", pdaf->main_offset_x);
	print_key(file, "main-offset-y", indent, "%u", pdaf->main_offset_y);
	print_key_stringentry(file, "global-pdaf-type", indent, pdaf_types,
			      array_length(pdaf_types), pdaf->global_pdaf_type);
	print_key(file, "block-width", indent, "%u", pdaf->block_width);
	print_key(file, "block-height", indent, "%u", pdaf->block_height);

	fputc('\n', file);
	print_mapping(file, "block-desc-groups", indent);

	for (i = 0; i < pdaf->num_block_desc_groups; i++) {
		struct ccs_pdaf_pix_loc_block_desc_group *bdgroup =
			&pdaf->block_desc_groups[i];
		unsigned int j;

		print_key(file, "- repeat-y", indent + 2, "%u",
			  bdgroup->repeat_y);

		fputc('\n', file);
		print_mapping(file, "block-descs", indent + 4);

		for (j = 0; j < bdgroup->num_block_descs; j++) {
			struct ccs_pdaf_pix_loc_block_desc *bdesc =
				&bdgroup->block_descs[j];

			print_key(file, "- block-type-id", indent + 6, "%u",
				  bdesc->block_type_id);
			print_key(file, "repeat-x", indent + 8, "%u",
				  bdesc->repeat_x);
		}
	}

	fputc('\n', file);
	print_mapping(file, "pixel-desc-groups", indent);

	for (i = 0; i < pdaf->num_pixel_desc_grups; i++) {
		struct ccs_pdaf_pix_loc_pixel_desc_group *pgroup =
			&pdaf->pixel_desc_groups[i];
		unsigned int j;

		tab_fprintf(file, "%*s-\n", indent + 2, "");

		for (j = 0; j < pgroup->num_descs; j++) {
			struct ccs_pdaf_pix_loc_pixel_desc *pdesc =
				&pgroup->descs[j];

			print_key_stringentry(file, "- pixel-type",
					      indent + 4, pixel_types,
					      array_length(pixel_types),
					      pdesc->pixel_type);
			print_key(file, "small-offset-x", indent + 6, "%u",
				  pdesc->small_offset_x);
			print_key(file, "small-offset-y", indent + 6, "%u",
				  pdesc->small_offset_y);
		}
	}

	return 0;
}

static int print_license(const char *license, unsigned int license_length,
			 FILE *file)
{
	const struct ccs_license *li = ccs_licenses;

	while (li->text) {
		if (li->length != license_length ||
		    strncmp(li->text, license, license_length)) {
			li++;
			continue;
		}

		fputc('\n', file);
		print_mapping(file, "license", 0);
		print_key_str(file, "filename", 2, "%s", li->filename);

		return 0;
	}

	fputs("cannot find license:\n", stderr);
	fwrite(license, license_length, 1, stderr);

	return -ENOENT;
}

static void print_end(FILE *file)
{
	fputs("\nend:\n", file);
}

static int to_yaml(struct ccs_data_container *ccsdata, FILE *file)
{
	int rval = 0;

	fputs("---", file);
	if (ccsdata->version)
		print_data_version(ccsdata->version, file);
	if (ccsdata->sensor_read_only_regs)
		rval = print_regs(ccsdata->sensor_read_only_regs,
				  ccsdata->num_sensor_read_only_regs,
				  "sensor-read-only-regs", 0, file);
	if (!rval && ccsdata->module_read_only_regs)
		rval = print_regs(ccsdata->module_read_only_regs,
				  ccsdata->num_module_read_only_regs,
				  "module-read-only-regs", 0, file);
	if (!rval && ccsdata->sensor_manufacturer_regs)
		rval = print_regs(ccsdata->sensor_manufacturer_regs,
				  ccsdata->num_sensor_manufacturer_regs,
				  "sensor-manufacturer-regs", 0, file);
	if (!rval && ccsdata->module_manufacturer_regs)
		rval = print_regs(ccsdata->module_manufacturer_regs,
				  ccsdata->num_module_manufacturer_regs,
				  "module-manufacturer-regs", 0, file);
	if (!rval && ccsdata->sensor_rules)
		rval = print_rules(ccsdata->sensor_rules,
				   ccsdata->num_sensor_rules,
				   "sensor-rule-based-block", 0, file);
	if (!rval && ccsdata->module_rules)
		rval = print_rules(ccsdata->module_rules,
				   ccsdata->num_module_rules,
				   "module-rule-based-block", 0, file);
	if (!rval && ccsdata->sensor_pdaf)
		rval = print_pdaf_pixel_location(ccsdata->sensor_pdaf,
				  "sensor-pdaf-pixel-location-block", 0, file);
	if (!rval && ccsdata->module_pdaf)
		rval = print_pdaf_pixel_location(ccsdata->module_pdaf,
				  "module-pdaf-pixel-location-block", 0, file);
	if (!rval && ccsdata->license)
		rval = print_license(ccsdata->license, ccsdata->license_length,
				     file);
	if (!rval && ccsdata->end)
		print_end(file);

	return rval;
}

void help(const char *const name)
{
	printf("%s - Convert a CCS static data binary to YAML format\n"
	       "\n"
	       "usage: %s [-i|--input input-file -o|--output output-file]\n"
	       "\t[-h|--help] [-v|--verbose]\n"
	       "\n"
	       "where\n"
	       "\tinput-file: CCS static data binary\n"
	       "\toutput-file: CCS static data YAML format\n",
	       name, name);
}

int main(int argc, char **argv)
{
	struct ccs_data_container ccsdata;
	char *data = NULL;
	long size, len = 0;
	char *in_fname = NULL, *out_fname = NULL;
	FILE *in_file = stdin, *out_file = stdout;
	bool verbose = false;
	int rval;

	const struct option long_opts[] = {
		{ "input", required_argument, NULL, 'i' },
		{ "noout", no_argument, NULL, 'n' },
		{ "output", required_argument, NULL, 'o' },
		{ "verbose", no_argument, NULL, 'v' },
		{ "help", no_argument, NULL, 'h' },
		{ NULL, 0, NULL, 0 },
	};

	while (true) {
		int c;

		c = getopt_long(argc, argv, "hi:o:nv", long_opts, NULL);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			help(argv[0]);
			return 0;
		case 'i':
			in_fname = optarg;
			break;
		case 'n':
			out_fname = "/dev/null";
			break;
		case 'o':
			out_fname = optarg;
			break;
		case 'v':
			verbose = true;
			break;
		default:
			return EINVAL;
		}
	}

	if (in_fname && !(in_file = fopen(in_fname, "rb"))) {
		int err = errno;

		fprintf(stderr, "%s: %s\n", in_fname, strerror(errno));

		return err;
	}

	do {
		size = len + CHUNK_SIZE;

		data = realloc(data, size);
		if (!data)
			return ENOMEM;

		len += fread(data + len, 1, CHUNK_SIZE, in_file);

		if (ferror(in_file)) {
			fprintf(stderr, "error reading from %s\n",
				in_fname ?: "stdin");
			return EIO;
		}

	} while (!feof(in_file));

	if (in_fname)
		fclose(in_file);

	rval = ccs_data_parse(&ccsdata, data, len, stderr, verbose);
	if (rval < 0) {
		fprintf(stderr, "Can't parse CCS static data: %s\n",
			strerror(-rval));
		return -rval;
	}

	if (out_fname) {
		int out_fd;

		out_fd = open(out_fname,
			      O_CREAT | O_WRONLY | O_TRUNC | O_NOFOLLOW,
			      S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

		if (out_fd != -1)
			out_file = fdopen(out_fd, "w");

		if (out_fd == -1 || !out_file) {
			int err = errno;
			fprintf(stderr, "%s: %s\n", out_fname, strerror(errno));
			return err;
		}
	}

	rval = to_yaml(&ccsdata, out_file);
	if (rval < 0)
		return -rval;

	if (ferror(out_file) || (out_fname && fclose(out_file) == -1)) {
		fprintf(stderr, "error writing to %s\n",
			out_fname ?: "stdout");
		return EIO;
	}

	return 0;
}
