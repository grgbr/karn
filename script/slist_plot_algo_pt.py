#!/usr/bin/python3

import sys
import os
import glob
import argparse as argp
import subprocess as subproc
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.ticker as tck
import plt_colors as clr

class SlistPerfResult:
	_avg_nsec = None

	def __init__(self, file_path):
		self._runs = []

		with open(file_path) as f:
			for token in f.readline().strip('\n').split(' '):
				prop, val = token.split('=')
				if prop == 'type':
					self.type = val
				elif prop == 'keynr':
					self.keynr = int(val)
				elif prop == 'presort':
					self.presort = val
				elif prop == 'algo':
					self.algo = val
				else:
					print("unknow header property " +
					      "{0}".format(prop),
					      file = sys.stderr)

			for line in f.readlines():
				run = {}

				for token in line.strip('\n').split(' '):
					prop, val = token.split('=')
					if prop == 'nsec':
						run['nsec'] = val
					elif prop == 'cmp':
						run['cmp'] = val
					elif prop == 'swap':
						run['swap'] = val
					else:
						print(("unknow run property " +
						       "{0}").format(prop),
						      file = sys.stderr)

				self._runs.append(run)

	def average_time(self):
		if self._avg_nsec == None:
			runs = np.array([run['nsec'] for run in self._runs],
			                dtype = np.uint32)
			self._avg_nsec = np.average(runs)
		return self._avg_nsec


class SlistPerfResultAggregator:
	_results = []
	_presorts = None
	_algorithms = None
	_registry = None

	def register(self, result):
		# Register new result
		self._results.append(result)

	def presorts(self):
		if self._presorts == None:
			# Build list of unique ordering schemes
			self._presorts = sorted(set([r.presort for r in
			                             self._results]))
		return self._presorts

	def algorithms(self):
		if self._algorithms == None:
			# Build list of unique algorithms
			self._algorithms = sorted(set([r.algo
			                               for r in self._results]))
		return self._algorithms

	def select_runs(self, presort, algorithm):
		# Return list of runs matching presort ordering scheme and
		# algorithm passed in argument.
		sort = self._presorts[presort]
		algo = self._algorithms[algorithm]

		return sorted([r for r in self._results
		               if r.presort == sort and r.algo == algo],
		              key = lambda run: run.keynr)

	def results_bypresort(self, presort):
		self.presorts()
		self.algorithms()

		results = []
		for a in range(0, len(self._algorithms)):
			results.append(self.select_runs(presort, a))

		return results

	def results_byalgorithm(self, algorithm):
		self.presorts()
		self.algorithms()

		results = []
		for s in range(0, len(self._presorts)):
			results.append(self.select_runs(s, algorithm))

		return results


class SlistPerfResultPlotter:
	def __init__(self, aggregator):
		self._agg = aggregator
		clr.light()

	def plot_algo(self, dir_path, algorithm_name, interactive):
		for algo, name in enumerate(self._agg.algorithms()):
			if name == algorithm_name:
				break

		for a in range(0, len(self._agg.algorithms())):
			def log2_formatter(x, pos):
				if x >= 2**30:
					return '%dGi' % (x / 2**30)
				elif x >= 2**20:
					return '%dMi' % (x / 2**20)
				elif x >= 2**10:
					return '%dKi' % (x / 2**10)
				else:
					return x

			plt.close()
			fig, axes = plt.subplots(1)

			# Ensure elements are properly located (with enought
			# space between them) and visible.
			fig.set_tight_layout(True)

			# Show grid lines
			axes.grid(True)
			# Setup x axis title
			axes.set_xlabel('Number of keys')
			# Setup y axis title
			axes.set_ylabel('Time ($\mu$Sec)')

			results = self._agg.results_byalgorithm(algo)
			max_keys = 0
			for p in range(0, len(results)):
				keys = [r.keynr for r in results[p]]
				if keys[-1] > max_keys:
					max_keys = keys[-1]
				times = [r.average_time() * 1e-3
				         for r in results[p]]
				plt.plot(keys, times, 'o-',
				         label = str(self._agg.presorts()[p]))
                                
			xaxis = axes.get_xaxis()
                        
			# Request grid lines and ticks to appear according
                        # to a logarithmic scheme
			locator = tck.LogLocator(base = 2,
			                         subs = (max_keys / 16,
			                                 max_keys / 8))
			xaxis.set_major_locator(locator)

			# Format tick labels as multiples of power of 2 using
			# using SI unit prefixes.
			formatter = tck.FuncFormatter(log2_formatter)
			xaxis.set_major_formatter(formatter)

			# Rotate and place ticks labels in a readable manner.
			plt.xticks(rotation = -45, horizontalalignment = 'left')

			axes.ticklabel_format(axis='y', style='sci',
			                      scilimits=(-3,3))

			# Display legend for each plot in the upper left corner
			plt.legend(loc = 'upper left')
			# Set main title
			plt.title(algorithm_name +
			          ' sorting performance by presort scheme')

			if interactive:
				plt.show()
			else:
				# Generate final image
				plt.savefig(os.path.join(dir_path,
				                         'type-int_algo-' +
				                         algorithm_name +
				                         '.png'),
                                            transparent = True)


if __name__ == '__main__':
	parse = argp.ArgumentParser(description = 'Plot sorting performance ' +
	                                          'results by algorithm.')
	parse.add_argument('dir_path', metavar = 'OUT_DIR',
	                   type = str, nargs = 1,
	                   help = 'output directory where to generate data')
	parse.add_argument('algo', metavar = 'ALGORITHM',
	                   type = str, nargs = 1,
	                   choices = ['insertion', 'selection', 'bubble',
	                              'merge'],
	                   help = 'sorting algorithm')
	parse.add_argument('-i', '--interactive', action = 'store_true',
	                   help = 'request interactive plot viewing')
	args = parse.parse_args()
	dir_path = parse.parse_args().dir_path[0]
	
	agg = SlistPerfResultAggregator()
	for file_path in glob.glob(os.path.join(dir_path, '*.txt')):
		try:
			res = SlistPerfResult(file_path)
		except ValueError:
			print(("{0} file is corrupted, " +
			       "skipping...").format(file_path),
			      file = sys.stderr)
			continue

		agg.register(res)

	plot = SlistPerfResultPlotter(agg)
	plot.plot_algo(dir_path, args.algo[0], args.interactive)
