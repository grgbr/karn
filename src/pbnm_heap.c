#include "pbnm_heap.h"

#define pbnm_heap_assert_node(_node)             \
	assert(_node);                           \
	assert((_node)->pbnm_handle);            \
	assert(*(_node)->pbnm_handle == (_node))

static struct pbnm_heap_node *
pbnm_heap_swap(struct pbnm_heap_node *child, struct pbnm_heap_node *parent)
{
	pbnm_heap_assert_node(child);
	pbnm_heap_assert_node(parent);
	assert(child != parent);

	struct pbnm_heap_node **hndl = child->pbnm_handle;
	struct pbnm_heap_node  *node = *hndl;

	*hndl = *(parent->pbnm_handle);
	*(parent->pbnm_handle) = node;

	child->pbnm_handle = parent->pbnm_handle;
	parent->pbnm_handle = hndl;

	return parent;
}

static struct pbnm_heap_node *
pbnm_heap_join(struct pbnm_heap_node *first,
               struct pbnm_heap_node *second,
               pbnm_heap_compare_fn  *compare)
{
	pbnm_heap_assert_node(first);
	pbnm_heap_assert_node(second);
	assert(first != second);
	assert(first->pbnm_rank == second->pbnm_rank);
	assert(compare);

	struct pbnm_heap_node *parent;
	struct pbnm_heap_node *child;

	if (compare(first, second) <= 0) {
		parent = first;
		child = second;
	}
	else {
		parent = second;
		child = first;
	}

	child->pbnm_sibling = parent->pbnm_youngest;
	child->pbnm_parent = parent;

	parent->pbnm_youngest = child;
	parent->pbnm_rank++;

	return parent;
}

static struct pbnm_heap_node *
pbnm_heap_1way_merge_roots(struct pbnm_heap_node *node,
                           struct pbnm_heap_node *roots,
                           pbnm_heap_compare_fn  *compare)
{
	pbnm_heap_assert_node(node);
	assert(compare);

	while (roots && (node->pbnm_rank == roots->pbnm_rank)) {
		struct pbnm_heap_node *nxt = roots->pbnm_sibling;

		node = pbnm_heap_join(node, roots, compare);

		roots = nxt;
	}

	node->pbnm_sibling = roots;

	return node;
}

static struct pbnm_heap_node *
pbnm_heap_2way_merge_roots(struct pbnm_heap_node **first,
                           struct pbnm_heap_node **second,
                           pbnm_heap_compare_fn   *compare)
{
	assert(first);
	assert(second);
	pbnm_heap_assert_node(*first);
	pbnm_heap_assert_node(*second);
	assert(first != second);
	assert(compare);

	struct pbnm_heap_node *fst = *first;
	struct pbnm_heap_node *snd = *second;

	if (fst->pbnm_rank == snd->pbnm_rank) {
		*first = fst->pbnm_sibling;
		*second = snd->pbnm_sibling;

		return pbnm_heap_join(fst, snd, compare);
	}
	else if (fst->pbnm_rank < snd->pbnm_rank) {
		*first = fst->pbnm_sibling;

		return fst;
	}
	else {
		*second = snd->pbnm_sibling;

		return snd;
	}
}

static struct pbnm_heap_node *
pbnm_heap_merge_trees(struct pbnm_heap_node *first,
                      struct pbnm_heap_node *second,
                      pbnm_heap_compare_fn  *compare)
{
	assert(first);
	assert(second);
	pbnm_heap_assert_node(first);
	pbnm_heap_assert_node(second);
	assert(first != second);
	assert(compare);

	struct pbnm_heap_node  *res;
	struct pbnm_heap_node **tail = &res;

	res = pbnm_heap_2way_merge_roots(&first, &second, compare);

	while (first && second) {
		struct pbnm_heap_node *tmp;

		tmp = pbnm_heap_2way_merge_roots(&first, &second, compare);

		assert(tail);
		assert((*tail)->pbnm_rank <= tmp->pbnm_rank);

		if ((*tail)->pbnm_rank != tmp->pbnm_rank)
			tail = &(*tail)->pbnm_sibling;
		else
			tmp = pbnm_heap_join(*tail, tmp, compare);

		*tail = tmp;
	}

	if (!first)
		first = second;

	*tail = pbnm_heap_1way_merge_roots(*tail, first, compare);

	return res;
}

static void
pbnm_heap_remove_root(struct pbnm_heap       *heap,
                      struct pbnm_heap_node **previous,
                      struct pbnm_heap_node  *root)
{
	assert(heap);
	assert(previous);
	assert(*previous == root);
	pbnm_heap_assert_node(root);

	struct pbnm_heap_node *trees = NULL;

	heap->pbnm_count--;

	*previous = root->pbnm_sibling;

	root = root->pbnm_youngest;
	while (root) {
		struct pbnm_heap_node *nxt = root->pbnm_sibling;

		root->pbnm_parent = NULL;
		root->pbnm_sibling = trees;
		trees = root;

		root = nxt;
	}

	if (!heap->pbnm_roots) {
		heap->pbnm_roots = trees;

		return;
	}

	if (!trees)
		return;

	heap->pbnm_roots = pbnm_heap_merge_trees(heap->pbnm_roots, trees,
	                                         heap->pbnm_compare);
}

static struct pbnm_heap_node *
pbnm_heap_remove_key(struct pbnm_heap *heap, struct pbnm_heap_node *key)
{
	pbnm_heap_assert(heap);
	assert(heap->pbnm_count);

	struct pbnm_heap_node **prev;
	
	while (key->pbnm_parent)
		key = pbnm_heap_swap(key, key->pbnm_parent);

	prev = &heap->pbnm_roots;
	while (*prev != key)
		prev = &(*prev)->pbnm_sibling;

	pbnm_heap_remove_root(heap, prev, key);

	return key;
}

void
pbnm_heap_insert(struct pbnm_heap *heap, struct pbnm_heap_node *key)
{
	pbnm_heap_assert(heap);
	pbnm_heap_assert_node(key);

	key->pbnm_sibling = NULL;
	key->pbnm_parent = NULL;
	key->pbnm_youngest = NULL;
	key->pbnm_rank = 0;

	heap->pbnm_count++;

	heap->pbnm_roots = pbnm_heap_1way_merge_roots(key, heap->pbnm_roots,
	                                              heap->pbnm_compare);
}

struct pbnm_heap_node *
pbnm_heap_peek(struct pbnm_heap *heap)
{
	pbnm_heap_assert(heap);
	assert(heap->pbnm_count);

	const struct pbnm_heap_node *key = heap->pbnm_roots;
	const struct pbnm_heap_node *root = key;

	while (true) {
		const struct pbnm_heap_node *nxt = root->pbnm_sibling;

		if (!nxt)
			break;

		if (heap->pbnm_compare(nxt, key) < 0)
			key = nxt;

		root = nxt;
	}

	return (struct pbnm_heap_node *)key;
}

struct pbnm_heap_node *
pbnm_heap_extract(struct pbnm_heap *heap)
{
	pbnm_heap_assert(heap);
	assert(heap->pbnm_count);

	struct pbnm_heap_node **prev = &heap->pbnm_roots;
	struct pbnm_heap_node  *key = *prev;
	struct pbnm_heap_node  *root = key;

	while (true) {
		struct pbnm_heap_node *nxt = root->pbnm_sibling;

		if (!nxt)
			break;

		if (heap->pbnm_compare(nxt, key) < 0) {
			prev = &root->pbnm_sibling;
			key = nxt;
		}

		root = nxt;
	}

	pbnm_heap_remove_root(heap, prev, key);

	return key;
}

void
pbnm_heap_remove(struct pbnm_heap *heap, struct pbnm_heap_node *key)
{
	pbnm_heap_remove_key(heap, key);
}

void
pbnm_heap_promote(struct pbnm_heap *heap, struct pbnm_heap_node *key)
{
	pbnm_heap_assert(heap);
	assert(heap->pbnm_count);

	while (key->pbnm_parent &&
	       (heap->pbnm_compare(key, key->pbnm_parent) < 0))
		key = pbnm_heap_swap(key, key->pbnm_parent);
}

void
pbnm_heap_demote(struct pbnm_heap *heap, struct pbnm_heap_node *key)
{
	key = pbnm_heap_remove_key(heap, key);

	pbnm_heap_insert(heap, key);
}

void
pbnm_heap_merge(struct pbnm_heap *result, struct pbnm_heap *source)
{
	pbnm_heap_assert(result);
	assert(result->pbnm_count);
	pbnm_heap_assert(source);
	assert(source->pbnm_count);
	assert(result->pbnm_compare == source->pbnm_compare);

	result->pbnm_roots = pbnm_heap_merge_trees(result->pbnm_roots,
	                                           source->pbnm_roots,
	                                           result->pbnm_compare);

	result->pbnm_count += source->pbnm_count;
}
