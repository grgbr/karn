#!/bin/sh

usage()
{
	echo "Usage: $(basename $0) [OPTIONS] OBJECT_DIRECTORY"
	echo "with:"
	echo "    OBJECT_DIRECTORY  directory where build objects are located"
	echo "where OPTIONS:"
	echo "    -x | --xml        generate XML formatted output"
	echo "    -h | --help       this help message"
}

xml=0

cmdln=$(getopt --options hx --longoptions help,xml --name "$(basename $0)" \
       -- "$@")
if [ $? -gt 0 ]; then
	echo
	usage
	exit 1
fi

eval set -- "$cmdln"
while true; do
	case "$1" in
	-x|--xml)  xml=1; shift 1;;
	-h|--help) usage; exit 0;;
	--)        shift; break;;
	*)         break;;
	esac
done

if [ $# -ne 1 ]; then
	echo "missing build objects directory"
	echo
	usage
	exit 1
fi

objdir=$1
gcov_cmd="gcovr --exclude='.*_ut' --keep --object-directory=$objdir \
          --root=$(realpath $(dirname $0)/..) --sort-percentage"

if [ $xml -gt 0 ]; then
	# XML outputs both branch and line coverage stats
	eval $gcov_cmd --xml-pretty || exit 1
	exit 0
fi

echo
echo "                       GCC branch coverage statistic"
echo "=============================================================================="
eval $gcov_cmd --branch | sed '1,4d' || exit 1

echo
echo
echo "                       GCC line coverage statistics"
echo "=============================================================================="
eval $gcov_cmd | sed '1,4d' || exit 1
