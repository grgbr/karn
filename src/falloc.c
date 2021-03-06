#include <karn/falloc.h>
#include <stdint.h>
#include <sys/user.h>
#include <sys/mman.h>
#include <asm/mman.h>

#define falloc_assert(_allocator) \
	karn_assert(_allocator); \
	karn_assert((_allocator)->falloc_nr); \
	karn_assert((_allocator)->falloc_size >= sizeof(unsigned int)); \
	karn_assert((sizeof(struct falloc_page) + \
	             ((_allocator)->falloc_nr * (_allocator)->falloc_size)) <= \
	             page_size())

#define falloc_assert_page(_allocator, _page) \
	karn_assert(_allocator); \
	karn_assert(_page); \
	karn_assert((_page)->falloc_free <= (_page)->falloc_valid); \
	karn_assert((_page)->falloc_valid <= (_allocator)->falloc_nr)

static inline size_t
page_size(void)
{
	return PAGE_SIZE;
}

static inline void *
page_alloc(void)
{
	void *page;

	page = mmap(NULL, page_size(), PROT_READ | PROT_WRITE,
	            MAP_PRIVATE | MAP_ANONYMOUS | MAP_UNINITIALIZED, -1, 0);

	return (page != MAP_FAILED) ? page : NULL;
}

static inline void
page_free(void *page)
{
       munmap(page, PAGE_SIZE);
}

static inline void *
page_start(const void *ptr)
{
	return (void *)((uintptr_t)ptr & ~((uintptr_t)PAGE_SIZE - 1));
}

static void *
falloc_alloc_page(struct falloc *allocator)
{
	struct falloc_page *page;
	void               *chunk;

	page = page_alloc();
	if (!page)
		return NULL;

	page->falloc_valid = 1;
	page->falloc_free = 0;

	chunk = (void *)&page->falloc_chunks[0];

	dlist_append(&allocator->falloc_pages, &page->falloc_node);

	return chunk;
}

void *
falloc_alloc(struct falloc *allocator)
{
	falloc_assert(allocator);

	if (!dlist_empty(&allocator->falloc_pages)) {
		struct falloc_page *page;

		page = dlist_entry(dlist_next(&allocator->falloc_pages),
		                   typeof(*page), falloc_node);
		falloc_assert_page(allocator, page);

		if (page->falloc_valid < allocator->falloc_nr) {
			/* There are still some unused chunk(s) in this page. */
			size_t  off;
			void   *chunk;

			if (page->falloc_free) {
				off = (size_t)page->falloc_next *
				      allocator->falloc_size;

				chunk = (void *)&page->falloc_chunks[off];

				page->falloc_next = *(unsigned int *)chunk;
				page->falloc_free--;
			}
			else {
				off = (size_t)(page->falloc_valid) *
				      allocator->falloc_size;

				chunk = (void *)&page->falloc_chunks[off];

				page->falloc_valid++;
			}

			return chunk;
		}

		/* Page is full: move it to page list tail. */
		dlist_move_after(&allocator->falloc_pages, &page->falloc_node);
	}

	/* Allocate a new page and return its first chunk. */
	return falloc_alloc_page(allocator);
}

void
falloc_free(struct falloc *allocator, void *chunk)
{
	falloc_assert(allocator);
	karn_assert(chunk);

	struct falloc_page *page;

	page = page_start(chunk);
	falloc_assert_page(allocator, page);

	*((unsigned int *)chunk) = page->falloc_next;
	page->falloc_next = (unsigned int)(((size_t)chunk -
	                                    (size_t)&page->falloc_chunks[0]) /
	                                   allocator->falloc_size);

	page->falloc_free++;

	if (page->falloc_valid ==
	    (allocator->falloc_nr + page->falloc_free - 1)) {
		/* Page was full just before free: move it to page list head. */
		dlist_move_after(&allocator->falloc_pages, &page->falloc_node);
	}
	else if (page->falloc_valid == page->falloc_free) {
		/* Page is empty: release it. */
		dlist_remove(&page->falloc_node);
		page_free(page);
	}
}

void
falloc_init(struct falloc *allocator, size_t chunk_size)
{
	karn_assert(allocator);
	karn_assert(chunk_size >= sizeof(unsigned int));
	karn_assert((sizeof(struct falloc_page) + (2 * chunk_size)) <=
	            page_size());

	allocator->falloc_nr = (page_size() - sizeof(struct falloc_page)) /
	                       chunk_size;

	dlist_init(&allocator->falloc_pages);

	allocator->falloc_size = chunk_size;
}

void
falloc_fini(struct falloc *allocator)
{
	falloc_assert(allocator);

	while (!dlist_empty(&allocator->falloc_pages)) {
		struct falloc_page *page;

		page = dlist_entry(dlist_next(&allocator->falloc_pages),
		                   typeof(*page), falloc_node);
		falloc_assert_page(allocator, page);

		dlist_remove(&page->falloc_node);

		page_free(page);
	}
}
