#!/bin/bash
#
# Copyright (C) 2020 MIPI Alliance
# Copyright (C) 2019--2020 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

oneTimeSetUp() {
	tmpfile=$(mktemp) || assertTrue false
	tmpfile2=$(mktemp) || assertTrue false
}

testSensorBin() {
	for i in examples/sensor-*.fw; do
		./ccs-bin-to-yaml -i $i -o $tmpfile || \
			assertTrue "ccs-bin-to-yaml $i $?" false
		./ccs-yaml-to-bin -o $tmpfile2 -s $tmpfile || \
			assertTrue "ccs-yaml-to-bin $i $?" false
		cmp $tmpfile2 $i || \
			assertTrue "cmp $i" false
	done
}

oneTimeTearDown() {
	rm $tmpfile $tmpfile2
}

. shunit2
