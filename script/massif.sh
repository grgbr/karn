#!/bin/bash -e

# Run me with mapred arguments, i.e. memcheck some_text_file [threads_count]

ref=$1

shift 1
arg=$*

dir=build

echo Running valgrind massif over $dir/mapred $ref $arg ...

valgrind --tool=massif --stacks=yes --massif-out-file=$dir/massif.out \
	$dir/mapred $ref $arg >/dev/null

ms_print $dir/massif.out
