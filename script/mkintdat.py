#!/usr/bin/python3

import sys
import os
import argparse as argp
import random as rand
import struct

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.ticker as tck
import plt_colors as clr

def perf_plot_order(keys, title, base_path):
	if len(keys) > 1:
		order = np.array(keys, np.int32)[1:] - \
		        np.array(keys, np.int32)[0:-1]
		cum = np.select([order > 0, order < 0], [1, -1])
		cum = np.cumsum(cum)
		seq = np.array(range(1, order.size + 1))
		order = np.select([order >= 0, order < 0], [1, -1])

		seqs = [0]
		lens = [order[0]]
		s = 1
		while s < order.size:
			if order[s] != order[s - 1]:
				seqs.append(s)
				lens.append(order[s])
			else:
				lens[-1] = lens[-1] + order[s]
			s = s + 1
	else:
		cum = [0]
		seq = [0]
		seqs = [0]
		lens = [0]

	plt.close()
	fig, axes = plt.subplots(2)
	fig.suptitle(title, y = 0.99)
	fig.subplots_adjust(top = 0.5)
	fig.set_tight_layout(True)

	axes[0].grid(True)
	axes[0].set_xlabel('sequence number')
	axes[0].set_ylabel('cumulative ordering')
	axes[0].set_ylim(ymin = min(cum) - 1, ymax = max(cum) + 1)
	axes[0].xaxis.set_major_locator(tck.MaxNLocator(integer=True))
	if len(keys) <= 8:
		style = 'o-'
	else:
		style = '-'
	axes[0].plot(seq, cum, style, color = clr.COLOR["blue"])
	axes[0].ticklabel_format(axis='both', style='sci', scilimits=(-3,3))

	bins = range(min(lens), max(lens) + 2, 1)
	axes[1].grid(True)
	axes[1].set_xlabel('ordering length')
	axes[1].set_ylabel('ordering length ratio')
	axes[1].set_ylim(ymax = 1.1)
	axes[1].set_xlim(xmin = min(bins) - 0.5, xmax = max(bins) - 0.5)
	axes[1].xaxis.set_major_locator(tck.MaxNLocator(integer=True))
	axes[1].hist(lens, bins=bins, normed=True, align = 'left', rwidth=0.8,
	             color = clr.COLOR["green"])

	plt.savefig(base_path + ".png", transparent = True)


class DataGenerator:
	def __init__(self):
		#rand.seed(0)
		rand.seed()

	def mk_presorted_uint32_keys(self, presort, key_nr):
		def mk_order_factor_uint32_keys(order_factor, key_nr):
			cur_min = (1 << 30) - 1
			cur_max = 1 << 30

			keys = np.ones(key_nr, np.uint32)
			keys[int(order_factor * key_nr / 100):] = 0
			rand.shuffle(keys,
			             lambda: (float(order_factor) / 100.0))

			for idx, order in enumerate(keys):
				if not order:
					keys[idx] = cur_min
					cur_min = cur_min - 1
				else:
					keys[idx] = cur_max
					cur_max = cur_max + 1
                        
			return keys

		if presort == 'fullrev':
			keys = np.array(range(key_nr, 0, -1), np.uint32)
		elif presort == 'rarerev':
			keys = mk_order_factor_uint32_keys(40, key_nr)
		elif presort == 'fullin':
			keys = np.array(range(0, key_nr, 1), np.uint32)
		elif presort == 'rarein':
			keys = mk_order_factor_uint32_keys(60, key_nr)
		elif presort == 'even':
			keys = mk_order_factor_uint32_keys(50, key_nr)
		elif presort == 'worstins':
			keys = np.array(range(0, key_nr - 1, 1), np.uint32)
			keys = np.insert(keys, 0, key_nr)
		elif presort == 'random':
			keys = np.random.choice(1 << 32, key_nr, np.uint32)

		return keys


if __name__ == '__main__':
	parse = argp.ArgumentParser(description = 'Generate integer data for ' +
	                                          'sorting performance ' +
	                                          'assessment.')
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
	args = parse.parse_args()
	
	key_nr = args.key_nr[0]
	if key_nr < 0 or key_nr > (1 << 30):
		parse.error("argument KEY_NR: invalid number of keys " +
		            "specified")
	presort = args.presort[0]

	gen = DataGenerator()
	
	keys = gen.mk_presorted_uint32_keys(presort, key_nr)
	
	base_path = os.path.join(args.dir_path[0],
	                         "type-int_keynr-" + str(key_nr) +
	                         "_presort-" + str(presort))
	try:
		with open(base_path + ".dat", 'wb') as file:
			for i in keys:
				file.write(struct.pack('I', i))
	except Exception as e:
		print(e)
		sys.exit(1)

	clr.light()
	perf_plot_order(keys, 'presort: ' + presort + ' #keys: ' + str(key_nr),
	                base_path)
