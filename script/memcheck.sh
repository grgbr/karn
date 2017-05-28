#!/bin/bash -e

# Run me with mapred arguments, i.e. memcheck some_text_file [threads_count]

impl=$1
ref=$2

shift 2
arg=$*

dir=build

echo Running valgrind memcheck over $dir/mapred_${impl} $ref $arg ...

exec valgrind \
	--tool=memcheck --leak-check=full \
	--sim-hints=no-nptl-pthread-stackcache \
	$dir/mapred_${impl} $ref $arg >/dev/null
