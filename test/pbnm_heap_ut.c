#include "pbnm_heap.h"
#include <cute/cute.h>
#include <stdlib.h>
#include <errno.h>

struct pbnmhut_entry {
	struct pbnm_heap_node *heap;
	int                    key;
};

#define PBNMHUT_INIT_ENTRY(_key) \
	{                      \
		.key = _key    \
	}

static struct pbnm_heap pbnmhut_heap;

static int pbnmhut_compare_min(const struct pbnm_heap_node *restrict first,
                               const struct pbnm_heap_node *restrict second)
{
	cute_ensure(first);
	cute_ensure(second);

	return pbnm_heap_entry(first, struct pbnmhut_entry, heap)->key -
	       pbnm_heap_entry(second, struct pbnmhut_entry, heap)->key;
}

static void
pbnmhut_check_node_entry(const struct pbnm_heap_node *node,
                         const struct pbnmhut_entry  *entry)
{
	cute_ensure(entry->heap == node);
	cute_ensure(pbnm_heap_entry(node, typeof(*entry), heap) == entry);
}

static void
pbnmhut_init_entries(struct pbnmhut_entry *entries, unsigned int count)
{
	unsigned int c;
	int          err;

	for (c = 0, err = 0; !err && c < count; c++) {
		struct pbnm_heap_node *node;

		entries[c].heap = malloc(sizeof(*entries[c].heap));
		if (!entries[c].heap) {
			err = -ENOMEM;
			break;
		}

		pbnm_heap_init_node(entries[c].heap, &entries[c].heap);

		node = entries[c].heap;
		node->pbnm_sibling = (struct pbnm_heap_node *)0xdeadbeef;
		node->pbnm_parent = (struct pbnm_heap_node *)0xdeadbeef;
		node->pbnm_youngest = (struct pbnm_heap_node *)0xdeadbeef;
	}

	if (err) {
		while (c--)
			free(entries[c].heap);
	}

	cute_ensure(!err);
}

static void
pbnmhut_fini_entries(struct pbnmhut_entry *entries, unsigned int count)
{
	while (count--)
		free(entries[count].heap);
}

static unsigned int
pbnmhut_check_tree_prop(struct pbnm_heap            *heap,
                        const struct pbnm_heap_node *root)
{
	struct pbnm_heap_node *node = root->pbnm_youngest;
	unsigned int           cnt = 1;

	if (node) {
		do {
			cute_ensure(heap->pbnm_compare(node->pbnm_parent,
			                               node) <= 0);
			cnt += pbnmhut_check_tree_prop(heap, node);
			node = node->pbnm_sibling;
		} while (node);

		return cnt;
	}

	return cnt;
}

static void
pbnmhut_check_heap_prop(struct pbnm_heap *heap, unsigned int count)
{
	const struct pbnm_heap_node *node = heap->pbnm_roots;
	unsigned int                cnt = count;
	int                         rank = -1;

	if (!cnt) {
		cute_ensure(!node);
		return;
	}

	cute_ensure(node);

	node = heap->pbnm_roots;
	do {
		while (!(cnt & 1)) {
			cnt >>= 1;
			rank++;
		}
		rank++;
		cnt >>= 1;

		cute_ensure(!node->pbnm_parent);
		cute_ensure(node->pbnm_rank == (unsigned int)rank);

		cute_ensure(pbnmhut_check_tree_prop(heap, node) == (1U << rank));

		node = node->pbnm_sibling;
	} while (node);

	cute_ensure(cnt == 0);
}

static void
pbnmhut_fill_heap(struct pbnm_heap     *heap,
                  struct pbnmhut_entry *entries,
                  unsigned int          count)
{
	unsigned int c;

	pbnmhut_init_entries(entries, count);

	for (c = 0; c < count; c++) {
		pbnm_heap_insert(heap, entries[c].heap);
		pbnmhut_check_heap_prop(heap, c + 1);
	}
}

static void
pbnmhut_check_heap_entries(struct pbnm_heap      *heap,
                           struct pbnmhut_entry **checks,
                           unsigned int           count)
{
	unsigned int c;

	for (c = 0; c < count; c++) {
		const struct pbnm_heap_node *node = NULL;
		const struct pbnmhut_entry  *check = checks[c];

		node = pbnm_heap_peek(heap);
		pbnmhut_check_node_entry(node, check);

		node = NULL;
		node = pbnm_heap_extract(heap);
		cute_ensure(pbnm_heap_count(heap) == count - c - 1);
		pbnmhut_check_node_entry(node, check);

		pbnmhut_check_heap_prop(heap, count - c - 1);
	}
}

static void pbnmhut_check_heap(struct pbnm_heap      *heap,
                               struct pbnmhut_entry  *entries,
                               struct pbnmhut_entry **checks,
                               unsigned int           count)
{
	pbnmhut_fill_heap(heap, entries, count);
	pbnmhut_check_heap_entries(heap, checks, count);
	pbnmhut_fini_entries(entries, count);
}

static CUTE_PNP_SUITE(pbnmhut, NULL);

static void pbnmhut_setup_empty(void)
{
	pbnm_heap_init(&pbnmhut_heap, pbnmhut_compare_min);
}

static CUTE_PNP_FIXTURED_SUITE(pbnmhut_empty, &pbnmhut, pbnmhut_setup_empty,
                               NULL);

CUTE_PNP_TEST(pbnmhut_check_emptiness, &pbnmhut_empty)
{
	cute_ensure(pbnm_heap_empty(&pbnmhut_heap) == true);
}

CUTE_PNP_TEST(pbnmhut_peek_single, &pbnmhut_empty)
{
	struct pbnmhut_entry entry = PBNMHUT_INIT_ENTRY(2);

	pbnmhut_init_entries(&entry, 1);

	pbnm_heap_insert(&pbnmhut_heap, entry.heap);

	cute_ensure(pbnm_heap_count(&pbnmhut_heap) == 1U);
	pbnmhut_check_node_entry(pbnm_heap_peek(&pbnmhut_heap), &entry);

	pbnmhut_fini_entries(&entry, 1);
}

CUTE_PNP_TEST(pbnmhut_extract_single, &pbnmhut_empty)
{
	struct pbnmhut_entry entry = PBNMHUT_INIT_ENTRY(2);

	pbnmhut_init_entries(&entry, 1);

	pbnm_heap_insert(&pbnmhut_heap, entry.heap);

	cute_ensure(pbnm_heap_count(&pbnmhut_heap) == 1U);
	pbnmhut_check_node_entry(pbnm_heap_extract(&pbnmhut_heap), &entry);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) == 0U);

	pbnmhut_fini_entries(&entry, 1);
}

CUTE_PNP_TEST(pbnmhut_remove_single, &pbnmhut_empty)
{
	struct pbnmhut_entry entry = PBNMHUT_INIT_ENTRY(2);

	pbnmhut_init_entries(&entry, 1);

	pbnm_heap_insert(&pbnmhut_heap, entry.heap);

	cute_ensure(pbnm_heap_count(&pbnmhut_heap) == 1U);
	pbnm_heap_remove(&pbnmhut_heap, entry.heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) == 0U);

	pbnmhut_fini_entries(&entry, 1);
}

static CUTE_PNP_FIXTURED_SUITE(pbnmhut_inorder, &pbnmhut,
                               pbnmhut_setup_empty, NULL);

static struct pbnmhut_entry pbnmhut_inorder_entries[] = {
	PBNMHUT_INIT_ENTRY(0),
	PBNMHUT_INIT_ENTRY(1),
	PBNMHUT_INIT_ENTRY(2),
	PBNMHUT_INIT_ENTRY(3),
	PBNMHUT_INIT_ENTRY(4),
	PBNMHUT_INIT_ENTRY(5),
	PBNMHUT_INIT_ENTRY(6),
	PBNMHUT_INIT_ENTRY(7),
	PBNMHUT_INIT_ENTRY(8),
	PBNMHUT_INIT_ENTRY(9),
	PBNMHUT_INIT_ENTRY(10),
	PBNMHUT_INIT_ENTRY(11),
	PBNMHUT_INIT_ENTRY(12),
	PBNMHUT_INIT_ENTRY(13),
	PBNMHUT_INIT_ENTRY(14),
	PBNMHUT_INIT_ENTRY(15),
	PBNMHUT_INIT_ENTRY(16)
};

static struct pbnmhut_entry *pbnmhut_inorder_checks[] = {
	&pbnmhut_inorder_entries[0],
	&pbnmhut_inorder_entries[1],
	&pbnmhut_inorder_entries[2],
	&pbnmhut_inorder_entries[3],
	&pbnmhut_inorder_entries[4],
	&pbnmhut_inorder_entries[5],
	&pbnmhut_inorder_entries[6],
	&pbnmhut_inorder_entries[7],
	&pbnmhut_inorder_entries[8],
	&pbnmhut_inorder_entries[9],
	&pbnmhut_inorder_entries[10],
	&pbnmhut_inorder_entries[11],
	&pbnmhut_inorder_entries[12],
	&pbnmhut_inorder_entries[13],
	&pbnmhut_inorder_entries[14],
	&pbnmhut_inorder_entries[15],
	&pbnmhut_inorder_entries[16]
};

CUTE_PNP_TEST(pbnmhut_inorder2, &pbnmhut_inorder)
{
	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_inorder_entries,
	                  pbnmhut_inorder_checks, 2);
}

CUTE_PNP_TEST(pbnmhut_inorder3, &pbnmhut_inorder)
{
	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_inorder_entries,
	                   pbnmhut_inorder_checks, 3);
}

CUTE_PNP_TEST(pbnmhut_inorder4, &pbnmhut_inorder)
{
	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_inorder_entries,
	                   pbnmhut_inorder_checks, 4);
}

CUTE_PNP_TEST(pbnmhut_inorder5, &pbnmhut_inorder)
{
	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_inorder_entries,
	                   pbnmhut_inorder_checks, 5);
}

CUTE_PNP_TEST(pbnmhut_inorder6, &pbnmhut_inorder)
{
	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_inorder_entries,
	                   pbnmhut_inorder_checks, 6);
}

CUTE_PNP_TEST(pbnmhut_inorder7, &pbnmhut_inorder)
{
	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_inorder_entries,
	                   pbnmhut_inorder_checks, 7);
}

CUTE_PNP_TEST(pbnmhut_inorder8, &pbnmhut_inorder)
{
	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_inorder_entries,
	                   pbnmhut_inorder_checks, 8);
}

CUTE_PNP_TEST(pbnmhut_inorder9, &pbnmhut_inorder)
{
	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_inorder_entries,
	                   pbnmhut_inorder_checks, 9);
}

CUTE_PNP_TEST(pbnmhut_inorder10, &pbnmhut_inorder)
{
	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_inorder_entries,
	                   pbnmhut_inorder_checks, 10);
}

CUTE_PNP_TEST(pbnmhut_inorder11, &pbnmhut_inorder)
{
	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_inorder_entries,
	                   pbnmhut_inorder_checks, 11);
}

CUTE_PNP_TEST(pbnmhut_inorder12, &pbnmhut_inorder)
{
	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_inorder_entries,
	                   pbnmhut_inorder_checks, 12);
}

CUTE_PNP_TEST(pbnmhut_inorder13, &pbnmhut_inorder)
{
	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_inorder_entries,
	                   pbnmhut_inorder_checks, 13);
}

CUTE_PNP_TEST(pbnmhut_inorder14, &pbnmhut_inorder)
{
	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_inorder_entries,
	                   pbnmhut_inorder_checks, 14);
}

CUTE_PNP_TEST(pbnmhut_inorder15, &pbnmhut_inorder)
{
	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_inorder_entries,
	                   pbnmhut_inorder_checks, 15);
}

CUTE_PNP_TEST(pbnmhut_inorder16, &pbnmhut_inorder)
{
	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_inorder_entries,
	                   pbnmhut_inorder_checks, 16);
}

CUTE_PNP_TEST(pbnmhut_inorder17, &pbnmhut_inorder)
{
	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_inorder_entries,
	                   pbnmhut_inorder_checks, 17);
}

static CUTE_PNP_FIXTURED_SUITE(pbnmhut_revorder, &pbnmhut,
                               pbnmhut_setup_empty, NULL);

static struct pbnmhut_entry pbnmhut_revorder_entries[] = {
	PBNMHUT_INIT_ENTRY(16),
	PBNMHUT_INIT_ENTRY(15),
	PBNMHUT_INIT_ENTRY(14),
	PBNMHUT_INIT_ENTRY(13),
	PBNMHUT_INIT_ENTRY(12),
	PBNMHUT_INIT_ENTRY(11),
	PBNMHUT_INIT_ENTRY(10),
	PBNMHUT_INIT_ENTRY(9),
	PBNMHUT_INIT_ENTRY(8),
	PBNMHUT_INIT_ENTRY(7),
	PBNMHUT_INIT_ENTRY(6),
	PBNMHUT_INIT_ENTRY(5),
	PBNMHUT_INIT_ENTRY(4),
	PBNMHUT_INIT_ENTRY(3),
	PBNMHUT_INIT_ENTRY(2),
	PBNMHUT_INIT_ENTRY(1),
	PBNMHUT_INIT_ENTRY(0)
};

static struct pbnmhut_entry *pbnmhut_revorder_checks[] = {
	&pbnmhut_revorder_entries[16],
	&pbnmhut_revorder_entries[15],
	&pbnmhut_revorder_entries[14],
	&pbnmhut_revorder_entries[13],
	&pbnmhut_revorder_entries[12],
	&pbnmhut_revorder_entries[11],
	&pbnmhut_revorder_entries[10],
	&pbnmhut_revorder_entries[9],
	&pbnmhut_revorder_entries[8],
	&pbnmhut_revorder_entries[7],
	&pbnmhut_revorder_entries[6],
	&pbnmhut_revorder_entries[5],
	&pbnmhut_revorder_entries[4],
	&pbnmhut_revorder_entries[3],
	&pbnmhut_revorder_entries[2],
	&pbnmhut_revorder_entries[1],
	&pbnmhut_revorder_entries[0]
};

CUTE_PNP_TEST(pbnmhut_revorder2, &pbnmhut_revorder)
{
	unsigned int           count = 2;
	struct pbnmhut_entry **checks =
		&pbnmhut_revorder_checks[array_nr(pbnmhut_revorder_checks) -
		                         count];

	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_revorder_entries, checks,
	                   count);
}

CUTE_PNP_TEST(pbnmhut_revorder3, &pbnmhut_revorder)
{
	unsigned int           count = 3;
	struct pbnmhut_entry **checks =
		&pbnmhut_revorder_checks[array_nr(pbnmhut_revorder_checks) -
		                         count];

	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_revorder_entries, checks,
	                   count);
}

CUTE_PNP_TEST(pbnmhut_revorder4, &pbnmhut_revorder)
{
	unsigned int           count = 4;
	struct pbnmhut_entry **checks =
		&pbnmhut_revorder_checks[array_nr(pbnmhut_revorder_checks) -
		                         count];

	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_revorder_entries, checks,
	                   count);
}

CUTE_PNP_TEST(pbnmhut_revorder5, &pbnmhut_revorder)
{
	unsigned int           count = 5;
	struct pbnmhut_entry **checks =
		&pbnmhut_revorder_checks[array_nr(pbnmhut_revorder_checks) -
		                         count];

	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_revorder_entries, checks,
	                   count);
}

CUTE_PNP_TEST(pbnmhut_revorder6, &pbnmhut_revorder)
{
	unsigned int           count = 6;
	struct pbnmhut_entry **checks =
		&pbnmhut_revorder_checks[array_nr(pbnmhut_revorder_checks) -
		                         count];

	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_revorder_entries, checks,
	                   count);
}

CUTE_PNP_TEST(pbnmhut_revorder7, &pbnmhut_revorder)
{
	unsigned int           count = 7;
	struct pbnmhut_entry **checks =
		&pbnmhut_revorder_checks[array_nr(pbnmhut_revorder_checks) -
		                         count];

	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_revorder_entries, checks,
	                   count);
}

CUTE_PNP_TEST(pbnmhut_revorder8, &pbnmhut_revorder)
{
	unsigned int           count = 8;
	struct pbnmhut_entry **checks =
		&pbnmhut_revorder_checks[array_nr(pbnmhut_revorder_checks) -
		                         count];

	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_revorder_entries, checks,
	                   count);
}

CUTE_PNP_TEST(pbnmhut_revorder9, &pbnmhut_revorder)
{
	unsigned int           count = 9;
	struct pbnmhut_entry **checks =
		&pbnmhut_revorder_checks[array_nr(pbnmhut_revorder_checks) -
		                         count];

	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_revorder_entries, checks,
	                   count);
}

CUTE_PNP_TEST(pbnmhut_revorder10, &pbnmhut_revorder)
{
	unsigned int           count = 10;
	struct pbnmhut_entry **checks =
		&pbnmhut_revorder_checks[array_nr(pbnmhut_revorder_checks) -
		                         count];

	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_revorder_entries, checks,
	                   count);
}

CUTE_PNP_TEST(pbnmhut_revorder11, &pbnmhut_revorder)
{
	unsigned int           count = 11;
	struct pbnmhut_entry **checks =
		&pbnmhut_revorder_checks[array_nr(pbnmhut_revorder_checks) -
		                         count];

	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_revorder_entries, checks,
	                   count);
}

CUTE_PNP_TEST(pbnmhut_revorder12, &pbnmhut_revorder)
{
	unsigned int           count = 12;
	struct pbnmhut_entry **checks =
		&pbnmhut_revorder_checks[array_nr(pbnmhut_revorder_checks) -
		                         count];

	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_revorder_entries, checks,
	                   count);
}

CUTE_PNP_TEST(pbnmhut_revorder13, &pbnmhut_revorder)
{
	unsigned int           count = 13;
	struct pbnmhut_entry **checks =
		&pbnmhut_revorder_checks[array_nr(pbnmhut_revorder_checks) -
		                         count];

	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_revorder_entries, checks,
	                   count);
}

CUTE_PNP_TEST(pbnmhut_revorder14, &pbnmhut_revorder)
{
	unsigned int           count = 14;
	struct pbnmhut_entry **checks =
		&pbnmhut_revorder_checks[array_nr(pbnmhut_revorder_checks) -
		                         count];

	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_revorder_entries, checks,
	                   count);
}

CUTE_PNP_TEST(pbnmhut_revorder15, &pbnmhut_revorder)
{
	unsigned int           count = 15;
	struct pbnmhut_entry **checks =
		&pbnmhut_revorder_checks[array_nr(pbnmhut_revorder_checks) -
		                         count];

	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_revorder_entries, checks,
	                   count);
}

CUTE_PNP_TEST(pbnmhut_revorder16, &pbnmhut_revorder)
{
	unsigned int           count = 16;
	struct pbnmhut_entry **checks =
		&pbnmhut_revorder_checks[array_nr(pbnmhut_revorder_checks) -
		                         count];

	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_revorder_entries, checks,
	                   count);
}

CUTE_PNP_TEST(pbnmhut_revorder17, &pbnmhut_revorder)
{
	unsigned int           count = 17;
	struct pbnmhut_entry **checks =
		&pbnmhut_revorder_checks[array_nr(pbnmhut_revorder_checks) -
		                         count];

	pbnmhut_check_heap(&pbnmhut_heap, pbnmhut_revorder_entries, checks,
	                   count);
}

static CUTE_PNP_FIXTURED_SUITE(pbnmhut_unsorted, &pbnmhut,
                               pbnmhut_setup_empty, NULL);

CUTE_PNP_TEST(pbnmhut_unsorted_increasing, &pbnmhut_unsorted)
{
	struct pbnmhut_entry  entries[] = {
		PBNMHUT_INIT_ENTRY(0),  /* 0 */
		PBNMHUT_INIT_ENTRY(4),
		PBNMHUT_INIT_ENTRY(5),
		PBNMHUT_INIT_ENTRY(6),
		PBNMHUT_INIT_ENTRY(1),
		PBNMHUT_INIT_ENTRY(2),  /* 5 */
		PBNMHUT_INIT_ENTRY(3),
		PBNMHUT_INIT_ENTRY(10),
		PBNMHUT_INIT_ENTRY(11),
		PBNMHUT_INIT_ENTRY(12),
		PBNMHUT_INIT_ENTRY(7),  /* 10 */
		PBNMHUT_INIT_ENTRY(8),
		PBNMHUT_INIT_ENTRY(9),
		PBNMHUT_INIT_ENTRY(16),
		PBNMHUT_INIT_ENTRY(13),
		PBNMHUT_INIT_ENTRY(14), /* 15 */
		PBNMHUT_INIT_ENTRY(15)
	};

	struct pbnmhut_entry *checks[] = {
		&entries[0],
		&entries[4],
		&entries[5],
		&entries[6],
		&entries[1],
		&entries[2],
		&entries[3],
		&entries[10],
		&entries[11],
		&entries[12],
		&entries[7],
		&entries[8],
		&entries[9],
		&entries[14],
		&entries[15],
		&entries[16],
		&entries[13]
	};

	pbnmhut_check_heap(&pbnmhut_heap, entries, checks, array_nr(entries));
}

CUTE_PNP_TEST(pbnmhut_unsorted_decreasing, &pbnmhut_unsorted)
{
	struct pbnmhut_entry  entries[] = {
		PBNMHUT_INIT_ENTRY(6),  /* 0 */
		PBNMHUT_INIT_ENTRY(5),
		PBNMHUT_INIT_ENTRY(4),
		PBNMHUT_INIT_ENTRY(0),
		PBNMHUT_INIT_ENTRY(3),
		PBNMHUT_INIT_ENTRY(2),  /* 5 */
		PBNMHUT_INIT_ENTRY(1),
		PBNMHUT_INIT_ENTRY(9),
		PBNMHUT_INIT_ENTRY(8),
		PBNMHUT_INIT_ENTRY(7),
		PBNMHUT_INIT_ENTRY(16), /* 10 */
		PBNMHUT_INIT_ENTRY(12),
		PBNMHUT_INIT_ENTRY(11),
		PBNMHUT_INIT_ENTRY(10),
		PBNMHUT_INIT_ENTRY(15),
		PBNMHUT_INIT_ENTRY(14), /* 15 */
		PBNMHUT_INIT_ENTRY(13)
	};

	struct pbnmhut_entry *checks[] = {
		&entries[3],
		&entries[6],
		&entries[5],
		&entries[4],
		&entries[2],
		&entries[1],
		&entries[0],
		&entries[9],
		&entries[8],
		&entries[7],
		&entries[13],
		&entries[12],
		&entries[11],
		&entries[16],
		&entries[15],
		&entries[14],
		&entries[10]
	};

	pbnmhut_check_heap(&pbnmhut_heap, entries, checks, array_nr(entries));
}

CUTE_PNP_TEST(pbnmhut_unsorted_diverge, &pbnmhut_unsorted)
{
	struct pbnmhut_entry  entries[] = {
		PBNMHUT_INIT_ENTRY(4),  /* 0 */
		PBNMHUT_INIT_ENTRY(5),
		PBNMHUT_INIT_ENTRY(6),
		PBNMHUT_INIT_ENTRY(3),
		PBNMHUT_INIT_ENTRY(2),
		PBNMHUT_INIT_ENTRY(1),  /* 5 */
		PBNMHUT_INIT_ENTRY(0),
		PBNMHUT_INIT_ENTRY(10),
		PBNMHUT_INIT_ENTRY(11),
		PBNMHUT_INIT_ENTRY(12),
		PBNMHUT_INIT_ENTRY(9),  /* 10 */
		PBNMHUT_INIT_ENTRY(8),
		PBNMHUT_INIT_ENTRY(7),
		PBNMHUT_INIT_ENTRY(15),
		PBNMHUT_INIT_ENTRY(14),
		PBNMHUT_INIT_ENTRY(16), /* 15 */
		PBNMHUT_INIT_ENTRY(13)
	};

	struct pbnmhut_entry *checks[] = {
		&entries[6],
		&entries[5],
		&entries[4],
		&entries[3],
		&entries[0],
		&entries[1],
		&entries[2],
		&entries[12],
		&entries[11],
		&entries[10],
		&entries[7],
		&entries[8],
		&entries[9],
		&entries[16],
		&entries[14],
		&entries[13],
		&entries[15]
	};

	pbnmhut_check_heap(&pbnmhut_heap, entries, checks, array_nr(entries));
}

CUTE_PNP_TEST(pbnmhut_unsorted_converge, &pbnmhut_unsorted)
{
	struct pbnmhut_entry  entries[] = {
		PBNMHUT_INIT_ENTRY(16), /* 0 */
		PBNMHUT_INIT_ENTRY(15),
		PBNMHUT_INIT_ENTRY(14),
		PBNMHUT_INIT_ENTRY(13),
		PBNMHUT_INIT_ENTRY(0),
		PBNMHUT_INIT_ENTRY(1),  /* 5 */
		PBNMHUT_INIT_ENTRY(2),
		PBNMHUT_INIT_ENTRY(3),
		PBNMHUT_INIT_ENTRY(12),
		PBNMHUT_INIT_ENTRY(11),
		PBNMHUT_INIT_ENTRY(10), /* 10 */
		PBNMHUT_INIT_ENTRY(4),
		PBNMHUT_INIT_ENTRY(5),
		PBNMHUT_INIT_ENTRY(6),
		PBNMHUT_INIT_ENTRY(9),
		PBNMHUT_INIT_ENTRY(7),  /* 15 */
		PBNMHUT_INIT_ENTRY(8)
	};

	struct pbnmhut_entry *checks[] = {
		&entries[4],
		&entries[5],
		&entries[6],
		&entries[7],
		&entries[11],
		&entries[12],
		&entries[13],
		&entries[15],
		&entries[16],
		&entries[14],
		&entries[10],
		&entries[9],
		&entries[8],
		&entries[3],
		&entries[2],
		&entries[1],
		&entries[0]
	};

	pbnmhut_check_heap(&pbnmhut_heap, entries, checks, array_nr(entries));
}

static CUTE_PNP_FIXTURED_SUITE(pbnmhut_duplicates, &pbnmhut,
                               pbnmhut_setup_empty, NULL);

CUTE_PNP_TEST(pbnmhut_duplicates2, &pbnmhut_duplicates)
{
	struct pbnmhut_entry  entries[] = {
		PBNMHUT_INIT_ENTRY(0),
		PBNMHUT_INIT_ENTRY(0)
	};

	struct pbnmhut_entry *checks[] = {
		&entries[1],
		&entries[0]
	};

	pbnmhut_check_heap(&pbnmhut_heap, entries, checks, array_nr(entries));
}

CUTE_PNP_TEST(pbnmhut_duplicates3, &pbnmhut_duplicates)
{
	struct pbnmhut_entry  entries[] = {
		PBNMHUT_INIT_ENTRY(0),
		PBNMHUT_INIT_ENTRY(0),
		PBNMHUT_INIT_ENTRY(0)
	};

	struct pbnmhut_entry *checks[] = {
		&entries[2],
		&entries[1],
		&entries[0]
	};

	pbnmhut_check_heap(&pbnmhut_heap, entries, checks, array_nr(entries));
}

CUTE_PNP_TEST(pbnmhut_duplicates_leading, &pbnmhut_duplicates)
{
	struct pbnmhut_entry  entries[] = {
		PBNMHUT_INIT_ENTRY(0),
		PBNMHUT_INIT_ENTRY(0),
		PBNMHUT_INIT_ENTRY(2)
	};

	struct pbnmhut_entry *checks[] = {
		&entries[1],
		&entries[0],
		&entries[2]
	};

	pbnmhut_check_heap(&pbnmhut_heap, entries, checks, array_nr(entries));
}

CUTE_PNP_TEST(pbnmhut_duplicates_trailing, &pbnmhut_duplicates)
{
	struct pbnmhut_entry  entries[] = {
		PBNMHUT_INIT_ENTRY(2),
		PBNMHUT_INIT_ENTRY(0),
		PBNMHUT_INIT_ENTRY(0)
	};

	struct pbnmhut_entry *checks[] = {
		&entries[2],
		&entries[1],
		&entries[0]
	};

	pbnmhut_check_heap(&pbnmhut_heap, entries, checks, array_nr(entries));
}

CUTE_PNP_TEST(pbnmhut_duplicates_interleave, &pbnmhut_duplicates)
{
	struct pbnmhut_entry  entries[] = {
		PBNMHUT_INIT_ENTRY(0),
		PBNMHUT_INIT_ENTRY(2),
		PBNMHUT_INIT_ENTRY(0)
	};

	struct pbnmhut_entry *checks[] = {
		&entries[2],
		&entries[0],
		&entries[1]
	};

	pbnmhut_check_heap(&pbnmhut_heap, entries, checks, array_nr(entries));
}

CUTE_PNP_TEST(pbnmhut_duplicates_mix, &pbnmhut_duplicates)
{
	struct pbnmhut_entry  entries[] = {
		PBNMHUT_INIT_ENTRY(2),  /* 0 */
		PBNMHUT_INIT_ENTRY(2),
		PBNMHUT_INIT_ENTRY(0),
		PBNMHUT_INIT_ENTRY(1),
		PBNMHUT_INIT_ENTRY(3),
		PBNMHUT_INIT_ENTRY(8),  /* 5 */
		PBNMHUT_INIT_ENTRY(7),
		PBNMHUT_INIT_ENTRY(6),
		PBNMHUT_INIT_ENTRY(5),
		PBNMHUT_INIT_ENTRY(4),
		PBNMHUT_INIT_ENTRY(4),  /* 10 */
		PBNMHUT_INIT_ENTRY(10),
		PBNMHUT_INIT_ENTRY(11),
		PBNMHUT_INIT_ENTRY(13),
		PBNMHUT_INIT_ENTRY(8),
		PBNMHUT_INIT_ENTRY(12), /* 15 */
		PBNMHUT_INIT_ENTRY(9),
		PBNMHUT_INIT_ENTRY(9)
	};

	struct pbnmhut_entry *checks[] = {
		&entries[2],
		&entries[3],
		&entries[1],
		&entries[0],
		&entries[4],
		&entries[10],
		&entries[9],
		&entries[8],
		&entries[7],
		&entries[6],
		&entries[5],
		&entries[14],
		&entries[17],
		&entries[16],
		&entries[11],
		&entries[12],
		&entries[15],
		&entries[13]
	};

	pbnmhut_check_heap(&pbnmhut_heap, entries, checks, array_nr(entries));
}

static CUTE_PNP_SUITE(pbnmhut_merge, &pbnmhut);

static void pbnmhut_check_heap_merge(struct pbnmhut_entry  *first,
                                     unsigned int           first_count,
                                     struct pbnmhut_entry  *second,
                                     unsigned int           second_count,
                                     struct pbnmhut_entry **checks)
{
	struct pbnm_heap fst;
	struct pbnm_heap snd;

	pbnm_heap_init(&fst, pbnmhut_compare_min);
	pbnmhut_fill_heap(&fst, first, first_count);
	pbnm_heap_init(&snd, pbnmhut_compare_min);
	pbnmhut_fill_heap(&snd, second, second_count);

	pbnm_heap_merge(&fst, &snd);
	pbnmhut_check_heap_entries(&fst, checks, first_count + second_count);

	pbnmhut_fini_entries(first, first_count);
	pbnmhut_fini_entries(second, second_count);
}

CUTE_PNP_TEST(pbnmhut_merge_inorder11, &pbnmhut_merge)
{
	struct pbnmhut_entry  fst[] = {
		PBNMHUT_INIT_ENTRY(0)
	};
	struct pbnmhut_entry  snd[] = {
		PBNMHUT_INIT_ENTRY(1)
	};
	struct pbnmhut_entry *checks[] = {
		&fst[0],
		&snd[0]
	};

	pbnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                         checks);
}

CUTE_PNP_TEST(pbnmhut_merge_revorder11, &pbnmhut_merge)
{
	struct pbnmhut_entry  fst[] = {
		PBNMHUT_INIT_ENTRY(1)
	};
	struct pbnmhut_entry  snd[] = {
		PBNMHUT_INIT_ENTRY(0)
	};
	struct pbnmhut_entry *checks[] = {
		&snd[0],
		&fst[0]
	};

	pbnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                         checks);
}

CUTE_PNP_TEST(pbnmhut_merge_inorder12, &pbnmhut_merge)
{
	struct pbnmhut_entry  fst[] = {
		PBNMHUT_INIT_ENTRY(0)
	};
	struct pbnmhut_entry  snd[] = {
		PBNMHUT_INIT_ENTRY(1),
		PBNMHUT_INIT_ENTRY(2)
	};
	struct pbnmhut_entry *checks[] = {
		&fst[0],
		&snd[0],
		&snd[1]
	};

	pbnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                         checks);
}

CUTE_PNP_TEST(pbnmhut_merge_revorder12, &pbnmhut_merge)
{
	struct pbnmhut_entry  fst[] = {
		PBNMHUT_INIT_ENTRY(2)
	};
	struct pbnmhut_entry  snd[] = {
		PBNMHUT_INIT_ENTRY(1),
		PBNMHUT_INIT_ENTRY(0)
	};
	struct pbnmhut_entry *checks[] = {
		&snd[1],
		&snd[0],
		&fst[0]
	};

	pbnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                         checks);
}

CUTE_PNP_TEST(pbnmhut_merge_unsorted12, &pbnmhut_merge)
{
	struct pbnmhut_entry  fst[] = {
		PBNMHUT_INIT_ENTRY(1)
	};
	struct pbnmhut_entry  snd[] = {
		PBNMHUT_INIT_ENTRY(2),
		PBNMHUT_INIT_ENTRY(0)
	};
	struct pbnmhut_entry *checks[] = {
		&snd[1],
		&fst[0],
		&snd[0]
	};

	pbnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                         checks);
}

CUTE_PNP_TEST(pbnmhut_merge_unsorted22, &pbnmhut_merge)
{
	struct pbnmhut_entry  fst[] = {
		PBNMHUT_INIT_ENTRY(1),
		PBNMHUT_INIT_ENTRY(2)
	};
	struct pbnmhut_entry  snd[] = {
		PBNMHUT_INIT_ENTRY(3),
		PBNMHUT_INIT_ENTRY(0)
	};
	struct pbnmhut_entry *checks[] = {
		&snd[1],
		&fst[0],
		&fst[1],
		&snd[0]
	};

	pbnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                         checks);
}

CUTE_PNP_TEST(pbnmhut_merge_unsorted31, &pbnmhut_merge)
{
	struct pbnmhut_entry  fst[] = {
		PBNMHUT_INIT_ENTRY(3),
		PBNMHUT_INIT_ENTRY(0),
		PBNMHUT_INIT_ENTRY(1)
	};
	struct pbnmhut_entry  snd[] = {
		PBNMHUT_INIT_ENTRY(2),
	};
	struct pbnmhut_entry *checks[] = {
		&fst[1],
		&fst[2],
		&snd[0],
		&fst[0]
	};

	pbnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                         checks);
}

CUTE_PNP_TEST(pbnmhut_merge_mit, &pbnmhut_merge)
{
	struct pbnmhut_entry  fst[] = {
		PBNMHUT_INIT_ENTRY(41),
		PBNMHUT_INIT_ENTRY(28),
		PBNMHUT_INIT_ENTRY(33),
		PBNMHUT_INIT_ENTRY(15),
		PBNMHUT_INIT_ENTRY(7),
		PBNMHUT_INIT_ENTRY(25),
		PBNMHUT_INIT_ENTRY(12)
	};
	struct pbnmhut_entry  snd[] = {
		PBNMHUT_INIT_ENTRY(17),
		PBNMHUT_INIT_ENTRY(10),
		PBNMHUT_INIT_ENTRY(44),
		PBNMHUT_INIT_ENTRY(50),
		PBNMHUT_INIT_ENTRY(31),
		PBNMHUT_INIT_ENTRY(48),
		PBNMHUT_INIT_ENTRY(29),
		PBNMHUT_INIT_ENTRY(8),
		PBNMHUT_INIT_ENTRY(6),
		PBNMHUT_INIT_ENTRY(24),
		PBNMHUT_INIT_ENTRY(22),
		PBNMHUT_INIT_ENTRY(23),
		PBNMHUT_INIT_ENTRY(55),
		PBNMHUT_INIT_ENTRY(32),
		PBNMHUT_INIT_ENTRY(45),
		PBNMHUT_INIT_ENTRY(30),
		PBNMHUT_INIT_ENTRY(37),
		PBNMHUT_INIT_ENTRY(3),
		PBNMHUT_INIT_ENTRY(18)
	};
	struct pbnmhut_entry *checks[] = {
		&snd[17],
		&snd[8],
		&fst[4],
		&snd[7],
		&snd[1],
		&fst[6],
		&fst[3],
		&snd[0],
		&snd[18],
		&snd[10],
		&snd[11],
		&snd[9],
		&fst[5],
		&fst[1],
		&snd[6],
		&snd[15],
		&snd[4],
		&snd[13],
		&fst[2],
		&snd[16],
		&fst[0],
		&snd[2],
		&snd[14],
		&snd[5],
		&snd[3],
		&snd[12]
	};

	pbnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                         checks);
}

static CUTE_PNP_FIXTURED_SUITE(pbnmhut_remove, &pbnmhut, pbnmhut_setup_empty,
                               NULL);

static void pbnmhut_check_heap_remove(struct pbnmhut_entry  *entries,
                                      struct pbnmhut_entry  *removed,
                                      struct pbnmhut_entry **checks,
                                      unsigned int           count)
{
	pbnmhut_fill_heap(&pbnmhut_heap, entries, count);

	pbnm_heap_remove(&pbnmhut_heap, removed->heap);
	pbnmhut_check_heap_entries(&pbnmhut_heap, checks, count - 1);

	pbnmhut_fini_entries(entries, count);
}

CUTE_PNP_TEST(pbnmhut_remove_top, &pbnmhut_remove)
{
	struct pbnmhut_entry  entries[] = {
		PBNMHUT_INIT_ENTRY(0),
		PBNMHUT_INIT_ENTRY(2)
	};
	struct pbnmhut_entry *checks[] = {
		&entries[1]
	};

	pbnmhut_check_heap_remove(entries, &entries[0], checks,
	                          array_nr(entries));
}

CUTE_PNP_TEST(pbnmhut_remove_bottom, &pbnmhut_remove)
{
	struct pbnmhut_entry  entries[] = {
		PBNMHUT_INIT_ENTRY(0),
		PBNMHUT_INIT_ENTRY(2)
	};
	struct pbnmhut_entry *checks[] = {
		&entries[0]
	};

	pbnmhut_check_heap_remove(entries, &entries[1], checks,
	                          array_nr(entries));
}

CUTE_PNP_TEST(pbnmhut_remove_middle, &pbnmhut_remove)
{
	struct pbnmhut_entry  entries[] = {
		PBNMHUT_INIT_ENTRY(1),
		PBNMHUT_INIT_ENTRY(2),
		PBNMHUT_INIT_ENTRY(0)
	};
	struct pbnmhut_entry *checks[] = {
		&entries[2],
		&entries[0],
	};

	pbnmhut_check_heap_remove(entries, &entries[1], checks,
	                          array_nr(entries));
}

static struct pbnmhut_entry pbnmhut_entries[] = {
	PBNMHUT_INIT_ENTRY(11),
	PBNMHUT_INIT_ENTRY(12),
	PBNMHUT_INIT_ENTRY(18),
	PBNMHUT_INIT_ENTRY(10),
	PBNMHUT_INIT_ENTRY(14),
	PBNMHUT_INIT_ENTRY(15),
	PBNMHUT_INIT_ENTRY(21),
	PBNMHUT_INIT_ENTRY(17),
	PBNMHUT_INIT_ENTRY(13),
	PBNMHUT_INIT_ENTRY(16),
	PBNMHUT_INIT_ENTRY(20),
	PBNMHUT_INIT_ENTRY(19)
};

CUTE_PNP_TEST(pbnmhut_remove_inorder, &pbnmhut_remove)
{
	struct pbnmhut_entry *checks[] = {
		&pbnmhut_entries[3],
		/* &pbnmhut_entries[0], */
		/* &pbnmhut_entries[1], */
		&pbnmhut_entries[8],
		/* &pbnmhut_entries[4], */
		&pbnmhut_entries[5],
		&pbnmhut_entries[9],
		/* &pbnmhut_entries[7], */
		&pbnmhut_entries[2],
		&pbnmhut_entries[11],
		&pbnmhut_entries[10],
		/* &pbnmhut_entries[6] */
	};

	pbnmhut_fill_heap(&pbnmhut_heap, pbnmhut_entries,
	                  array_nr(pbnmhut_entries));

	pbnm_heap_remove(&pbnmhut_heap, pbnmhut_entries[0].heap);
	pbnm_heap_remove(&pbnmhut_heap, pbnmhut_entries[1].heap);
	pbnm_heap_remove(&pbnmhut_heap, pbnmhut_entries[4].heap);
	pbnm_heap_remove(&pbnmhut_heap, pbnmhut_entries[7].heap);
	pbnm_heap_remove(&pbnmhut_heap, pbnmhut_entries[6].heap);

	pbnmhut_check_heap_entries(&pbnmhut_heap, checks,
	                           array_nr(pbnmhut_entries) - 5);

	pbnmhut_fini_entries(pbnmhut_entries, array_nr(pbnmhut_entries));
}

CUTE_PNP_TEST(pbnmhut_remove_revorder, &pbnmhut_remove)
{
	struct pbnmhut_entry *checks[] = {
		/* &pbnmhut_entries[3], */
		&pbnmhut_entries[0],
		&pbnmhut_entries[1],
		&pbnmhut_entries[8],
		/* &pbnmhut_entries[4], */
		&pbnmhut_entries[5],
		&pbnmhut_entries[9],
		/* &pbnmhut_entries[7], */
		&pbnmhut_entries[2],
		/* &pbnmhut_entries[11], */
		/* &pbnmhut_entries[10], */
		&pbnmhut_entries[6]
	};

	pbnmhut_fill_heap(&pbnmhut_heap, pbnmhut_entries,
	                  array_nr(pbnmhut_entries));

	pbnm_heap_remove(&pbnmhut_heap, pbnmhut_entries[10].heap);
	pbnm_heap_remove(&pbnmhut_heap, pbnmhut_entries[11].heap);
	pbnm_heap_remove(&pbnmhut_heap, pbnmhut_entries[7].heap);
	pbnm_heap_remove(&pbnmhut_heap, pbnmhut_entries[4].heap);
	pbnm_heap_remove(&pbnmhut_heap, pbnmhut_entries[3].heap);

	pbnmhut_check_heap_entries(&pbnmhut_heap, checks,
	                           array_nr(pbnmhut_entries) - 5);

	pbnmhut_fini_entries(pbnmhut_entries, array_nr(pbnmhut_entries));
}

CUTE_PNP_TEST(pbnmhut_remove_altorder, &pbnmhut_remove)
{
	struct pbnmhut_entry *checks[] = {
		/* &pbnmhut_entries[3], */
		&pbnmhut_entries[0],
		&pbnmhut_entries[1],
		/* &pbnmhut_entries[8], */
		&pbnmhut_entries[4],
		/* &pbnmhut_entries[5], */
		/* &pbnmhut_entries[9], */
		&pbnmhut_entries[7],
		&pbnmhut_entries[2],
		/* &pbnmhut_entries[11], */
		&pbnmhut_entries[10],
		&pbnmhut_entries[6]
	};

	pbnmhut_fill_heap(&pbnmhut_heap, pbnmhut_entries,
	                  array_nr(pbnmhut_entries));

	pbnm_heap_remove(&pbnmhut_heap, pbnmhut_entries[5].heap);
	pbnm_heap_remove(&pbnmhut_heap, pbnmhut_entries[9].heap);
	pbnm_heap_remove(&pbnmhut_heap, pbnmhut_entries[8].heap);
	pbnm_heap_remove(&pbnmhut_heap, pbnmhut_entries[11].heap);
	pbnm_heap_remove(&pbnmhut_heap, pbnmhut_entries[3].heap);

	pbnmhut_check_heap_entries(&pbnmhut_heap, checks,
	                           array_nr(pbnmhut_entries) - 5);

	pbnmhut_fini_entries(pbnmhut_entries, array_nr(pbnmhut_entries));
}

static CUTE_PNP_FIXTURED_SUITE(pbnmhut_promote, &pbnmhut,
                               pbnmhut_setup_empty, NULL);

CUTE_PNP_TEST(pbnmhut_promote_inorder, &pbnmhut_promote)
{
	struct pbnmhut_entry *checks[] = {
		&pbnmhut_entries[4],  /* 8 */
		&pbnmhut_entries[1],  /* 9 */
		&pbnmhut_entries[3],  /* 10 */
		&pbnmhut_entries[0],  /* 10 */
		&pbnmhut_entries[7],  /* 12 */
		&pbnmhut_entries[8],  /* 13 */
		&pbnmhut_entries[5],  /* 15 */
		&pbnmhut_entries[9],  /* 16 */
		&pbnmhut_entries[6],  /* 17 */
		&pbnmhut_entries[2],  /* 18 */
		&pbnmhut_entries[11], /* 19 */
		&pbnmhut_entries[10]  /* 20 */
	};

	pbnmhut_fill_heap(&pbnmhut_heap, pbnmhut_entries,
	                  array_nr(pbnmhut_entries));

	pbnmhut_entries[0].key -= 1;
	pbnm_heap_promote(&pbnmhut_heap, pbnmhut_entries[0].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_entries[1].key -= 3;
	pbnm_heap_promote(&pbnmhut_heap, pbnmhut_entries[1].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_entries[4].key -= 6;
	pbnm_heap_promote(&pbnmhut_heap, pbnmhut_entries[4].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_entries[7].key -= 5;
	pbnm_heap_promote(&pbnmhut_heap, pbnmhut_entries[7].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_entries[6].key -= 4;
	pbnm_heap_promote(&pbnmhut_heap, pbnmhut_entries[6].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_check_heap_entries(&pbnmhut_heap, checks,
	                           array_nr(pbnmhut_entries));

	pbnmhut_fini_entries(pbnmhut_entries, array_nr(pbnmhut_entries));
}

CUTE_PNP_TEST(pbnmhut_promote_revorder, &pbnmhut_promote)
{
	struct pbnmhut_entry *checks[] = {
		&pbnmhut_entries[7],  /* 7 */
		&pbnmhut_entries[3],  /* 8 */
		&pbnmhut_entries[4],  /* 9 */
		&pbnmhut_entries[0],  /* 11 */
		&pbnmhut_entries[1],  /* 12 */
		&pbnmhut_entries[8],  /* 13 */
		&pbnmhut_entries[5],  /* 15 */
		&pbnmhut_entries[9],  /* 16 */
		&pbnmhut_entries[11], /* 16 */
		&pbnmhut_entries[2],  /* 18 */
		&pbnmhut_entries[10], /* 19 */
		&pbnmhut_entries[6]   /* 21 */
	};

	pbnmhut_fill_heap(&pbnmhut_heap, pbnmhut_entries,
	                  array_nr(pbnmhut_entries));

	pbnmhut_entries[10].key -= 1;
	pbnm_heap_promote(&pbnmhut_heap, pbnmhut_entries[10].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_entries[11].key -= 3;
	pbnm_heap_promote(&pbnmhut_heap, pbnmhut_entries[11].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_entries[7].key -= 10;
	pbnm_heap_promote(&pbnmhut_heap, pbnmhut_entries[7].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_entries[4].key -= 5;
	pbnm_heap_promote(&pbnmhut_heap, pbnmhut_entries[4].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_entries[3].key -= 2;
	pbnm_heap_promote(&pbnmhut_heap, pbnmhut_entries[3].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_check_heap_entries(&pbnmhut_heap, checks,
	                           array_nr(pbnmhut_entries));

	pbnmhut_fini_entries(pbnmhut_entries, array_nr(pbnmhut_entries));
}

CUTE_PNP_TEST(pbnmhut_promote_altorder, &pbnmhut_promote)
{
	struct pbnmhut_entry *checks[] = {
		&pbnmhut_entries[8],  /* 8 */
		&pbnmhut_entries[9],  /* 9 */
		&pbnmhut_entries[3],  /* 9 */
		&pbnmhut_entries[0],  /* 11 */
		&pbnmhut_entries[1],  /* 12 */
		&pbnmhut_entries[5],  /* 13 */
		&pbnmhut_entries[4],  /* 14 */
		&pbnmhut_entries[11], /* 16 */
		&pbnmhut_entries[7],  /* 17 */
		&pbnmhut_entries[2],  /* 18 */
		&pbnmhut_entries[10], /* 20 */
		&pbnmhut_entries[6]   /* 21 */
	};

	pbnmhut_fill_heap(&pbnmhut_heap, pbnmhut_entries,
	                  array_nr(pbnmhut_entries));

	pbnmhut_entries[5].key -= 2;
	pbnm_heap_promote(&pbnmhut_heap, pbnmhut_entries[5].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_entries[9].key -= 7;
	pbnm_heap_promote(&pbnmhut_heap, pbnmhut_entries[9].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_entries[8].key -= 5;
	pbnm_heap_promote(&pbnmhut_heap, pbnmhut_entries[8].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_entries[11].key -= 3;
	pbnm_heap_promote(&pbnmhut_heap, pbnmhut_entries[11].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_entries[3].key -= 1;
	pbnm_heap_promote(&pbnmhut_heap, pbnmhut_entries[3].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_check_heap_entries(&pbnmhut_heap, checks,
	                           array_nr(pbnmhut_entries));

	pbnmhut_fini_entries(pbnmhut_entries, array_nr(pbnmhut_entries));
}

static CUTE_PNP_FIXTURED_SUITE(pbnmhut_demote, &pbnmhut,
                               pbnmhut_setup_empty, NULL);

CUTE_PNP_TEST(pbnmhut_demote_single, &pbnmhut_demote)
{
	static struct pbnmhut_entry  entries[] = {
		PBNMHUT_INIT_ENTRY(0)
	};

	struct pbnmhut_entry        *checks[] = {
		&entries[0]
	};

	pbnmhut_fill_heap(&pbnmhut_heap, entries, array_nr(entries));

	entries[0].key += 1;
	pbnm_heap_demote(&pbnmhut_heap, entries[0].heap);

	pbnmhut_check_heap_entries(&pbnmhut_heap, checks, array_nr(entries));

	pbnmhut_fini_entries(entries, array_nr(entries));
}

CUTE_PNP_TEST(pbnmhut_demote_top, &pbnmhut_demote)
{
	static struct pbnmhut_entry  entries[] = {
		PBNMHUT_INIT_ENTRY(0),
		PBNMHUT_INIT_ENTRY(1)
	};
	struct pbnmhut_entry        *checks[] = {
		&entries[1],
		&entries[0]
	};

	pbnmhut_fill_heap(&pbnmhut_heap, entries, array_nr(entries));

	entries[0].key += 2;
	pbnm_heap_demote(&pbnmhut_heap, entries[0].heap);

	pbnmhut_check_heap_entries(&pbnmhut_heap, checks, array_nr(entries));

	pbnmhut_fini_entries(entries, array_nr(entries));
}

CUTE_PNP_TEST(pbnmhut_demote_bottom, &pbnmhut_demote)
{
	static struct pbnmhut_entry  entries[] = {
		PBNMHUT_INIT_ENTRY(0),
		PBNMHUT_INIT_ENTRY(1)
	};
	struct pbnmhut_entry        *checks[] = {
		&entries[0],
		&entries[1]
	};

	pbnmhut_fill_heap(&pbnmhut_heap, entries, array_nr(entries));

	entries[1].key += 2;
	pbnm_heap_demote(&pbnmhut_heap, entries[1].heap);

	pbnmhut_check_heap_entries(&pbnmhut_heap, checks, array_nr(entries));

	pbnmhut_fini_entries(entries, array_nr(entries));
}

CUTE_PNP_TEST(pbnmhut_demote_inorder, &pbnmhut_demote)
{
	struct pbnmhut_entry *checks[] = {
		&pbnmhut_entries[3],  /* 10 */
		&pbnmhut_entries[0],  /* 12 */
		&pbnmhut_entries[8],  /* 13 */
		&pbnmhut_entries[5],  /* 15 */
		&pbnmhut_entries[1],  /* 15 */
		&pbnmhut_entries[9],  /* 16 */
		&pbnmhut_entries[2],  /* 18 */
		&pbnmhut_entries[11], /* 19 */
		&pbnmhut_entries[10], /* 20 */
		&pbnmhut_entries[4],  /* 20 */
		&pbnmhut_entries[7],  /* 22 */
		&pbnmhut_entries[6]   /* 25 */
	};

	pbnmhut_fill_heap(&pbnmhut_heap, pbnmhut_entries,
	                  array_nr(pbnmhut_entries));

	pbnmhut_entries[0].key += 1;
	pbnm_heap_demote(&pbnmhut_heap, pbnmhut_entries[0].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_entries[1].key += 3;
	pbnm_heap_demote(&pbnmhut_heap, pbnmhut_entries[1].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_entries[4].key += 6;
	pbnm_heap_demote(&pbnmhut_heap, pbnmhut_entries[4].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_entries[7].key += 5;
	pbnm_heap_demote(&pbnmhut_heap, pbnmhut_entries[7].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_entries[6].key += 4;
	pbnm_heap_demote(&pbnmhut_heap, pbnmhut_entries[6].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_check_heap_entries(&pbnmhut_heap, checks,
	                           array_nr(pbnmhut_entries));

	pbnmhut_fini_entries(pbnmhut_entries, array_nr(pbnmhut_entries));
}

CUTE_PNP_TEST(pbnmhut_demote_revorder, &pbnmhut_demote)
{
	struct pbnmhut_entry *checks[] = {
		&pbnmhut_entries[0],  /* 11 */
		&pbnmhut_entries[1],  /* 12 */
		&pbnmhut_entries[8],  /* 13 */
		&pbnmhut_entries[5],  /* 15 */
		&pbnmhut_entries[9],  /* 16 */
		&pbnmhut_entries[4],  /* 17 */
		&pbnmhut_entries[2],  /* 18 */
		&pbnmhut_entries[3],  /* 19 */
		&pbnmhut_entries[11], /* 20 */
		&pbnmhut_entries[6],  /* 21 */
		&pbnmhut_entries[10], /* 22 */
		&pbnmhut_entries[7]   /* 24 */
	};

	pbnmhut_fill_heap(&pbnmhut_heap, pbnmhut_entries,
	                  array_nr(pbnmhut_entries));

	pbnmhut_entries[10].key += 2;
	pbnm_heap_demote(&pbnmhut_heap, pbnmhut_entries[10].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_entries[11].key += 1;
	pbnm_heap_demote(&pbnmhut_heap, pbnmhut_entries[11].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_entries[7].key += 7;
	pbnm_heap_demote(&pbnmhut_heap, pbnmhut_entries[7].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_entries[4].key += 3;
	pbnm_heap_demote(&pbnmhut_heap, pbnmhut_entries[4].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_entries[3].key += 9;
	pbnm_heap_demote(&pbnmhut_heap, pbnmhut_entries[3].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_check_heap_entries(&pbnmhut_heap, checks,
	                           array_nr(pbnmhut_entries));

	pbnmhut_fini_entries(pbnmhut_entries, array_nr(pbnmhut_entries));
}

CUTE_PNP_TEST(pbnmhut_demote_altorder, &pbnmhut_demote)
{
	struct pbnmhut_entry *checks[] = {
		&pbnmhut_entries[0],  /* 11 */
		&pbnmhut_entries[1],  /* 12 */
		&pbnmhut_entries[3],  /* 13 */
		&pbnmhut_entries[4],  /* 14 */
		&pbnmhut_entries[8],  /* 15 */
		&pbnmhut_entries[5],  /* 16 */
		&pbnmhut_entries[7],  /* 17 */
		&pbnmhut_entries[2],  /* 18 */
		&pbnmhut_entries[10], /* 20 */
		&pbnmhut_entries[6],  /* 21 */
		&pbnmhut_entries[11], /* 22 */
		&pbnmhut_entries[9]   /* 24 */
	};

	pbnmhut_fill_heap(&pbnmhut_heap, pbnmhut_entries,
	                  array_nr(pbnmhut_entries));

	pbnmhut_entries[5].key += 1;
	pbnm_heap_demote(&pbnmhut_heap, pbnmhut_entries[5].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_entries[9].key += 8;
	pbnm_heap_demote(&pbnmhut_heap, pbnmhut_entries[9].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_entries[8].key += 2;
	pbnm_heap_demote(&pbnmhut_heap, pbnmhut_entries[8].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_entries[11].key += 3;
	pbnm_heap_demote(&pbnmhut_heap, pbnmhut_entries[11].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_entries[3].key += 3;
	pbnm_heap_demote(&pbnmhut_heap, pbnmhut_entries[3].heap);
	cute_ensure(pbnm_heap_count(&pbnmhut_heap) ==
	            array_nr(pbnmhut_entries));

	pbnmhut_check_heap_entries(&pbnmhut_heap, checks,
	                           array_nr(pbnmhut_entries));

	pbnmhut_fini_entries(pbnmhut_entries, array_nr(pbnmhut_entries));
}
