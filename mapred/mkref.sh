#!/bin/bash -e

# Run me with mapred arguments, i.e. mapred.sh some_text_file [threads_count]

ref=$1

dir=build

# remove directory component
base=$(basename $ref)
# remove file extension
base=${base%.*}

# tr  : split input file into words and output one word per line
exec tr -s '[[:punct:][:space:]]' '\n' < $ref | \
        # drop empty lines
        egrep -v '^[[:space:]]*$' | \
        # well... sort alphanumerically
        sort | \
        # report number of occurences of adjacent repeated lines
        uniq -c | \
        # finally format output according to '<word>: <occurences>' format
	awk '/^.+$/ {print $2 ": " $1}' > $dir/${base}_ref.txt
        # for the file given in argument.
