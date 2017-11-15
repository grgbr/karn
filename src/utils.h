/******************************************************************************
 * This file is part of Karn
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

#include <stddef.h>
#include <limits.h>
#include <assert.h>

#if __WORDSIZE == 64
#define __UINTPTR_C(c) c ## UL
#elif __WORDSIZE == 32
#define __UINTPTR_C(c) c ## U
#else
#error "Unsupported machine word size !"
#endif

#define __unused     __attribute__((unused))
#define __align(...) __attribute__((aligned(__VA_ARGS__)))

/**
 * Retrieve the maximum number of slots a static array may contain
 *
 * @param _array statically declared array
 *
 * @return number of slots
 *
 * @ingroup farr
 */
#define array_nr(_array) \
	(sizeof(_array) / sizeof((_array)[0]))


#define min(_a, _b)                      \
	({                               \
		typeof(_a) __a = _a;     \
		typeof(_b) __b = _b;     \
		(__a < __b) ? __a : __b; \
	 })

#define max(_a, _b)                      \
	({                               \
		typeof(_a) __a = _a;     \
		typeof(_b) __b = _b;     \
		(__a > __b) ? __a : __b; \
	 })

#define containerof(ptr, type, member)                          \
	({                                                      \
		const typeof(((type *)0)->member) *__p = ptr;   \
		(type *)((char *)__p - offsetof(type, member)); \
	 })

static inline unsigned int
lower_pow2(unsigned int value)
{
	assert(value);

	return (sizeof(value) * CHAR_BIT) - 1 - __builtin_clz(value);
}

static inline unsigned int
upper_pow2(unsigned int value)
{
	/* Would overflow otherwise... */
	assert(value <= (1U << ((sizeof(value) * CHAR_BIT) - 1)));

	if (value == 1)
		return 0;

	return lower_pow2(value + (1U << lower_pow2(value)) - 1);
}

#define PREFETCH_ACCESS_RO     (0)
#define PREFETCH_ACCESS_RW     (1)
#define PREFETCH_LOCALITY_TMP  (0)
#define PREFETCH_LOCALITY_LOW  (1)
#define PREFETCH_LOCALITY_FAIR (2)
#define PREFETCH_LOCALITY_HIGH (3)

#define prefetch(_address, ...) __builtin_prefetch(_address, ## __VA_ARGS__)

#endif
