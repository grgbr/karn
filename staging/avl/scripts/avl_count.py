#!/usr/bin/python3

import sys

def avl_min_count(depth):
	if depth >= 3:
		cnt = 0
		cnt_1 = 2
		cnt_2 = 1
		for d in range(2, depth):
			cnt = 1 + cnt_1 + cnt_2
			cnt_2 = cnt_1
			cnt_1 = cnt

		return cnt

	return depth

if __name__ == '__main__':
	print(avl_min_count(int(sys.argv[1])))
