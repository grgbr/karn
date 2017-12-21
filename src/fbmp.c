#include "fbmp.h"

unsigned int
fbmp_find_zero(const uintptr_t *bitmap, unsigned int nr)
{
	uintptr_t    word;
	unsigned int idx;

	nr = fbmp_word_nr(nr) - 1;

	for (word = 0; word < nr; word++) {
		idx = ffsw(bitmap[word] ^ ~(__UINTPTR_C(0)));
		if (idx)
			break;
	}

	assert(idx);
	assert((word < nr) ^ (idx <= (nr % sizeof(*bitmap))));

	return (word * sizeof(uintptr_t) * CHAR_BIT) + idx - 1;
}
