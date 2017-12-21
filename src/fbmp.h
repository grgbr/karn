#ifndef _KARN_FBMP_H
#define _KARN_FBMP_H

#ifndef CONFIG_FBMP
#error FBMP configuration disabled !
#endif

#include "utils.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static inline unsigned int
fbmp_word_nr(unsigned int nr)
{
	return (nr + sizeof(uintptr_t) - 1) / sizeof(uintptr_t);
}

static inline size_t
fbmp_size(unsigned int nr)
{
	return fbmp_word_nr(nr) * CHAR_BIT;
}

static inline bool
fbmp_test(const uintptr_t *bitmap, unsigned int index)
{
	unsigned int word = index / sizeof(*bitmap);
	unsigned int bit = index % sizeof(*bitmap);

	return !!(bitmap[word] & (__UINTPTR_C(1) << bit));
}

static inline void
fbmp_set(uintptr_t *bitmap, unsigned int index)
{
	unsigned int word = index / sizeof(*bitmap);
	unsigned int bit = index % sizeof(*bitmap);

	bitmap[word] |= __UINTPTR_C(1) << bit;
}

static inline void
fbmp_set_all(uintptr_t *bitmap, unsigned int nr)
{
	memset(bitmap, 0xff, fbmp_size(nr));
}

static inline void
fbmp_clear(uintptr_t *bitmap, unsigned int index)
{
	unsigned int word = index / sizeof(*bitmap);
	unsigned int bit = index % sizeof(*bitmap);

	bitmap[word] &= ~(__UINTPTR_C(1) << bit);
}

static inline void
fbmp_clear_all(uintptr_t *bitmap, unsigned int nr)
{
	memset(bitmap, 0, fbmp_size(nr));
}

static inline void
fbmp_toggle(uintptr_t *bitmap, unsigned int index)
{
	unsigned int word = index / sizeof(*bitmap);
	unsigned int bit = index % sizeof(*bitmap);

	bitmap[word] ^= __UINTPTR_C(1) << bit;
}

static inline void
fbmp_init(uintptr_t *bitmap, unsigned int nr)
{
	fbmp_clear_all(bitmap, nr);
}

static inline uintptr_t *
fbmp_create(unsigned int nr)
{
	return calloc(fbmp_word_nr(nr), sizeof(uintptr_t));
}

static inline void
fbmp_destroy(uintptr_t *bitmap)
{
	free(bitmap);
}

extern unsigned int fbmp_find_zero(const uintptr_t *bitmap, unsigned int nr);

#endif
