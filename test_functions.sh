#!/bin/sh

goto_dir() 
{
	dir=$1
	mkdir -p ${dir}
	cd ${dir}
}

generate_files_group()
{
	local n=$1 # count of files
	local dupes=$2 # maximum percent of files which has one or more duplicates
	local groupsf=$3 # maximum percent of files of [dupes] which each group contains
	local size=$4 # <= 0: random TODO: make working
	       	# otherwise: =size in dd format
	
	local dps=$(($n * $dupes / 100))
	local in_grp=$(($dps * $groupsf / 100))
	local grps=$((dps / in_grp))
	for i in $(seq 1 1 $grps); do 
		(dd if=/dev/urandom of=$i bs=$size count=1) > /dev/null 2>&1; 
	done

	local FILES="$(pwd)"/*
	for f in $FILES; do
		for i in $(seq 1 1 $(($in_grp - 1))); do
  			cp $f "$f($i)"
		done
	done

	for i in $(seq $(($grps * $in_grp + 1)) 1 $n); do 
		(dd if=/dev/urandom of=$i bs=$size count=1) > /dev/null 2>&1; 
	done
}

randomize_file_names()
{
	FILES="$(pwd)"/*
	for f in $FILES; do
  		mv $f "$f(1)" # TODO: rewrite
	done

}

