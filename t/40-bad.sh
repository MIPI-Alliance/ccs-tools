#!/bin/bash
#
# Copyright (C) 2020 MIPI Alliance
# Copyright (C) 2019--2020 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

testBadInput()
{
	./ccs-yaml-to-bin -v -e examples/sensor-file-bad.yaml
	assertFalse $?
	./ccs-yaml-to-bin -v -e examples/sensor-file-bad-2.yaml
	assertFalse $?
	./ccs-yaml-to-bin -v -e examples/sensor-file-bad-3.yaml
	assertFalse $?
	./ccs-yaml-to-bin -v -e examples/sensor-file-bad-4.yaml
	assertFalse $?
}

. shunit2
