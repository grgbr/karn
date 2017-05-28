#!/usr/bin/python3

import sys
import os
import argparse as argp
import subprocess as subproc

if __name__ == '__main__':
	parse = argp.ArgumentParser(description = 'Sort input integer data ' +
	                                          'file for performance ' +
	                                          'assessment.')
	parse.add_argument('bin_path', metavar = 'BIN_FILE',
	                   type = str, nargs = 1,
	                   help = 'binary file to execute')
	parse.add_argument('dir_path', metavar = 'OUT_DIR',
	                   type = str, nargs = 1,
	                   help = 'output directory where to generate data')
	parse.add_argument('key_nr', metavar = 'KEY_NR',
	                   type = int, nargs = 1,
	                   help = 'number of keys to generate')
	parse.add_argument('presort', metavar = 'PRESORT',
	                   type = str, nargs = 1,
	                   choices = ['fullrev', 'rarerev', 'even', 'rarein',
	                              'fullin', 'worstins', 'random'],
	                   help = 'presorting scheme')
	parse.add_argument('algo', metavar = 'ALGORITHM',
	                   type = str, nargs = 1,
	                   choices = ['insertion', 'selection', 'bubble',
	                              'merge'],
	                   help = 'sorting algorithm')
	parse.add_argument('loop_nr', metavar = 'LOOP_NR',
	                   type = int, nargs = 1,
	                   help = 'number of measurement loops to run')
	args = parse.parse_args()
	
	key_nr = args.key_nr[0]
	if key_nr < 0 or key_nr > (1 << 30):
		parse.error("argument KEY_NR: invalid number of keys " +
		            "specified")
	presort = args.presort[0]
	loop_nr = args.loop_nr[0]
	if loop_nr < 0:
		parse.error("argument LOOP_NR: invalid number of " +
		            "measurement loops")
	data_path = os.path.join(args.dir_path[0],
	                         "type-int_keynr-" + str(key_nr) +
	                         "_presort-" + presort +
	                         ".dat")

	try:
		out = subproc.check_output([args.bin_path[0], data_path,
		                            args.algo[0], str(loop_nr)],
		                            timeout = 120)
	except Exception as e:
		print(e)
		sys.exit(1)

	print("type=int keynr=%d presort=%s algo=%s" %
              (key_nr, presort, args.algo[0]))
	for line in out.decode(sys.stdout.encoding).split('\n'):
		if not line:
			# Skip empty lines
			continue
		print(line)
