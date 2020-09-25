#ifndef _KARN_FALLOC_H
#define _KARN_FALLOC_H

#include <karn/dlist.h>

#ifndef CONFIG_KARN_FALLOC
#error FALLOC configuration disabled !
#endif

struct falloc_page {
	unsigned int      falloc_next;
	unsigned int      falloc_free;
	unsigned int      falloc_valid;
	struct dlist_node falloc_node;
	char              falloc_chunks[0];
};

struct falloc {
	struct dlist_node falloc_pages;
	unsigned int      falloc_nr;
	size_t            falloc_size;
};

extern void * falloc_alloc(struct falloc *allocator);

extern void falloc_free(struct falloc *allocator, void *chunk);

extern void falloc_init(struct falloc *allocator, size_t chunk_size);

extern void falloc_fini(struct falloc *allocator);

#endif /* _KARN_FALLOC_H */
