#!/bin/sh

datasets="data/comedy.txt data/lipsum.txt data/nsfabs.txt data/reuters.txt data/wiley.txt"

cpu=1
while [ $cpu -le 8 ]; do
	for f in $datasets; do
		test/mapred.sh hattrie $f $cpu
		test/mapred.sh rbtree $f $cpu
	done
	cpu=$((cpu + 1))
done
