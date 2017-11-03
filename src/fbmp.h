#ifndef _FBMP_H
#define _FBMP_H

#include "utils.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct fbmp {
	uintptr_t *fbmp_bits;
};

static inline unsigned int fbmp_word_nr(unsigned int nr)
{
	return (nr + sizeof(uintptr_t) - 1) / sizeof(uintptr_t);
}

static inline size_t fbmp_size(unsigned int nr)
{
	return fbmp_word_nr(nr) * CHAR_BIT;
}

static inline bool fbmp_test(const struct fbmp *bitmap, unsigned int index)
{
	unsigned int word = index / sizeof(*bitmap->fbmp_bits);
	unsigned int bit = index % sizeof(*bitmap->fbmp_bits);

	return !!(bitmap->fbmp_bits[word] & (__UINTPTR_C(1) << bit));
}

static inline void fbmp_set(const struct fbmp *bitmap, unsigned int index)
{
	unsigned int word = index / sizeof(*bitmap->fbmp_bits);
	unsigned int bit = index % sizeof(*bitmap->fbmp_bits);

	bitmap->fbmp_bits[word] |= __UINTPTR_C(1) << bit;
}

static inline void fbmp_set_all(const struct fbmp *bitmap, unsigned int nr)
{
	memset(bitmap->fbmp_bits, 0xff, fbmp_size(nr));
}

static inline void fbmp_clear(const struct fbmp *bitmap, unsigned int index)
{
	unsigned int word = index / sizeof(*bitmap->fbmp_bits);
	unsigned int bit = index % sizeof(*bitmap->fbmp_bits);

	bitmap->fbmp_bits[word] &= ~(__UINTPTR_C(1) << bit);
}

static inline void fbmp_clear_all(const struct fbmp *bitmap, unsigned int nr)
{
	memset(bitmap->fbmp_bits, 0, fbmp_size(nr));
}

static inline void fbmp_toggle(const struct fbmp *bitmap, unsigned int index)
{
	unsigned int word = index / sizeof(*bitmap->fbmp_bits);
	unsigned int bit = index % sizeof(*bitmap->fbmp_bits);

	bitmap->fbmp_bits[word] ^= __UINTPTR_C(1) << bit;
}

static inline int fbmp_init(struct fbmp *bitmap, unsigned int nr)
{
	bitmap->fbmp_bits = calloc(fbmp_word_nr(nr), sizeof(uintptr_t));
	if (!bitmap->fbmp_bits)
		return -ENOMEM;

	return 0;
}

static inline void fbmp_fini(struct fbmp *bitmap)
{
	free(bitmap->fbmp_bits);
}

#endif
