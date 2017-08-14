#!/bin/sh -e

top=$(realpath $(dirname $0)/..)

make -C $top test
cd $top/script
gdb -x karn_ut.gdb --args $top/build/karn_utdbg "$@"
