/******************************************************************************
 * This file is part of Mapred
 *
 * Copyright (C) 2017 Grégor Boirie <gregor.boirie@free.fr>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef _UTILS_H
#define _UTILS_H

#include <stdbool.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>

#define mapred_assert(_cond) assert(_cond)

#define __unused __attribute__((unused))

#define min(_a, _b)                      \
	({                               \
		typeof(_a) __a = _a;     \
		typeof(_b) __b = _b;     \
		(__a < __b) ? __a : __b; \
	 })

#define array_count(_array) \
	(sizeof(_array) / sizeof((_array)[0]))

extern size_t mapred_forward_delim_len(const char *, size_t);
extern size_t mapred_forward_token_len(const char *, size_t);
extern size_t mapred_backward_token_len(const char *, size_t);

static inline bool
mapred_ischar_delim(int c)
{
	return isspace(c) || ispunct(c);
}

static inline int
mapred_compare_strings(const char *restrict first_string,
                       size_t               first_length,
                       const char *restrict second_string,
                       size_t               second_length)
{
	int cmp = memcmp(first_string, second_string,
	                 min(first_length, second_length));

	if (!cmp)
		cmp = first_length - second_length;

	return cmp;
}

#endif
