#!/bin/bash -e

# Run me with mapred arguments, i.e. mapred.sh some_text_file [threads_count]

impl=$1
ref=$2

shift 2
arg=$*

dir=build

# remove directory component
base=$(basename $ref)
# remove file extension
base=${base%.*}

# Just to make sure we don't perform comparison with results from a previous
# run.
rm -f $dir/${base}_${impl}_res.txt

echo Running $dir/mapred_${impl} $ref $arg ...
$dir/mapred_${impl} $ref $arg > $dir/${base}_${impl}_res.txt
# remove last line
sed -i '$ d' $dir/${base}_${impl}_res.txt

echo Reference file: $dir/${base}_ref.txt
echo Test result file: $dir/${base}_${impl}_res.txt

if ! cmp $dir/${base}_ref.txt $dir/${base}_${impl}_res.txt; then
	echo Failure !
else
	echo Success.
fi
