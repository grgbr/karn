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

class PerfResult:
	_avg_nsec = None

	def __init__(self, file):
		self._runs = []
		self._filt = []

		for line in file.readlines():
			run = {}

			for token in line.strip('\n').split(' '):
				try:
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
				except ValueError:
					continue

			self._runs.append(run)

		self._filt = self._reject_outliers(self._runs, 2.)


	def _reject_outliers(self, runs, threshold):
		nsecs = np.array([run['nsec'] for run in runs],
		                 dtype = np.uint32)
		dist = np.abs(nsecs - np.median(nsecs))
		med = np.median(dist)
		spl = dist / (med if med else 1.)

		return np.take(runs, np.nonzero(spl < threshold)[0])


	def average_time(self):
		if self._avg_nsec == None:
			nsecs = np.array([filt['nsec'] for filt in self._filt],
			                 dtype = np.uint32)
			self._avg_nsec = np.average(nsecs)
		return self._avg_nsec


if __name__ == '__main__':
	res = PerfResult(sys.stdin)
	print(res.average_time())
