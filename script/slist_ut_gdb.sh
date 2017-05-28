#!/bin/sh -e

top=$(realpath $(dirname $0)/..)

make -C $top test
cd $top/script
exec gdb -x slist_ut.gdb --args $top/build/slist_utdbg "$@"
