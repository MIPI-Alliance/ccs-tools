#!/bin/bash
#
# Copyright (C) 2021 MIPI Alliance
# Copyright (C) 2019--2021 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

oneTimeSetUp() {
	tmpfile=$(mktemp) || assertTrue false
	tmpfile2=$(mktemp) || assertTrue false
	tmpfile3=$(mktemp) || assertTrue false
}

testSensorYAML() {
	for i in examples/sensor-*.yaml; do
		if [[ "$i" =~ "-bad" ]]; then
			continue;
		fi
		./ccs-yaml-to-bin -o $tmpfile -s $i || \
			assertTrue "ccs-yaml-to-bin $i $?" false
		./ccs-bin-to-yaml -i $tmpfile -o $tmpfile2 || \
			assertTrue "ccs-bin-to-yaml $tmpfile $?" false
		./ccs-yaml-to-bin -o $tmpfile3 -s $tmpfile2 || \
			assertTrue "ccs-yaml-to-bin $tmpefile2 $?" false
		cmp $tmpfile $tmpfile3 || \
			assertTrue "cmp $i" false
	done
}

oneTimeTearDown() {
	rm $tmpfile $tmpfile2 $tmpfile3
}

. shunit2
