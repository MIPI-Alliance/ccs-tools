#!/bin/bash
#
# Copyright (C) 2020 MIPI Alliance
# Copyright (C) 2019--2020 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

oneTimeSetUp() {
	tmpfile=$(mktemp) || assertTrue false
	tmpfile2=$(mktemp) || assertTrue false
	tmpfile3=$(mktemp) || assertTrue false
	EINVAL=$(perl -e 'use POSIX; print EINVAL')
	ENODATA=$(perl -e 'use POSIX; print ENODATA')
	ENOENT=$(perl -e 'use POSIX; print ENOENT')
}

runCmd() {
	local ignore=$1
	local orig=$2
	local cmd=$3
	local r

	$cmd

	r=$?
	if [ $r == 99 ]; then
		assertTrue "" $SHUNIT_TRUE
		return 99
	elif [ $ignore ]; then
		return 0;
	elif [ $r ]; then
		r=$?
		local t=$(mktemp)
		cp $tmpfile $t
		assertTrue "$cmd $t" $SHUNIT_TRUE
		return 0
	fi

	./ccs-bin-to-yaml -i $tmpfile2 -o $tmpfile3 > /dev/null 2>&1
	r=$?
	if [ ! $r ]; then
		cmp $tmpfile2 $orig
		r=$?
		assertFalse "$cmd rval $r" $r
		if [ ! $r ]; then
			local t=$(mktemp)
			cp $tmpfile2 $t
		fi
	else
		assertTrue "unexpected return value $r" \
			"[ $r == $EINVAL ] || [ $r == $ENODATA ] || [ $r == $ENOENT ]"
	fi

	return 0
}

testBin() {
	local max
	local r
	local orig
	for orig in $files; do
		./ccs-bin-to-yaml -i $orig -o $tmpfile || \
			assertTrue "ccs-bin-to-yaml $orig $?" false

		for ((max=1; r != 99; max++)); do
			runCmd 1 $orig "./ccs-yaml-to-bin -o $tmpfile2 $target $tmpfile \
				--break-len-count=$max"
			r=$?
		done

		for j in $seq_len; do
			echo testing break len $j
			for k in $seq_inject; do
				for ((l=1; $l < $max; l++)); do
					for ((m=1; $m < $max; m++)); do
						runCmd 0 $orig "./ccs-yaml-to-bin -o $tmpfile2 $target $tmpfile \
							--break-len-count=$l --break-len=$j --break-inject-count=$m --break-inject=$k"
					done
				done
			done
		done
	done

	return 0
}

oneTimeTearDown() {
	rm $tmpfile $tmpfile2 $tmpfile3
}

. shunit2
