#!/bin/bash
#
# Copyright (C) 2020 MIPI Alliance
# Copyright (C) 2019--2020 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

export seq_len="-1000 $(seq -10 -1) $(seq 1 40) 1000 100000 4194303"
export seq_inject=$seq_len
export target=-s
export files=examples/sensor-*.fw

t/bin-test.sh
