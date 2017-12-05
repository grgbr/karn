#include "pbnm_heap.h"

#define pbnm_heap_assert_node(_node)   \
	assert(_node);                 \
	assert((_node)->pbnm_handle);  \
	assert(*(_node)->pbnm_handle);

static struct pbnm_heap_node *
pbnm_heap_from_plcrs(const struct plcrs_node *node)
{
	assert(node);

	return plcrs_entry(node, struct pbnm_heap_node, pbnm_plcrs);
}

static struct pbnm_heap_node *
pbnm_heap_parent_node(const struct pbnm_heap_node *node)
{
	assert(node);

	const struct plcrs_node *parent;

	parent = plcrs_parent_node(&node->pbnm_plcrs);
	if (!parent)
		return NULL;

	return pbnm_heap_from_plcrs(parent);
}

static struct pbnm_heap_node *
pbnm_heap_next_node(const struct pbnm_heap_node *node)
{
	assert(node);

	const struct plcrs_node *nxt;

	nxt = plcrs_next_sibling(&node->pbnm_plcrs);
	if (!nxt)
		return NULL;

	return pbnm_heap_from_plcrs(nxt);
}

static void
pbnm_heap_swap(struct pbnm_heap_node *restrict first,
               struct pbnm_heap_node *restrict second)
{
	assert(first);
	assert(second);
	assert(first != second);

	struct pbnm_heap_node **restrict hndl = first->pbnm_handle;
	struct pbnm_heap_node  *restrict node = *hndl;

	*hndl = *(second->pbnm_handle);
	*(second->pbnm_handle) = node;

	first->pbnm_handle = second->pbnm_handle;
	second->pbnm_handle = hndl;
}

static struct pbnm_heap_node *
pbnm_heap_join(struct pbnm_heap_node *restrict first,
               struct pbnm_heap_node *restrict second,
               pbnm_heap_compare_fn  *compare)
{
	assert(first);
	assert(second);
	assert(first != second);
	assert(first->pbnm_rank == second->pbnm_rank);
	assert(compare);

	struct pbnm_heap_node *restrict parent;
	struct pbnm_heap_node *restrict child;

	if (compare(first, second) <= 0) {
		parent = first;
		child = second;
	}
	else {
		parent = second;
		child = first;
	}

	plcrs_join_tree(&child->pbnm_plcrs, &parent->pbnm_plcrs);

	parent->pbnm_rank++;

	return parent;
}

static struct plcrs_node *
pbnm_heap_merge_1root(struct pbnm_heap_node *root,
                      struct plcrs_node     *siblings,
                      pbnm_heap_compare_fn  *compare)
{
	assert(root);

	while (siblings) {
		struct plcrs_node     *nxt;
		struct pbnm_heap_node *curr;

		nxt = plcrs_next_sibling(siblings);

		curr = pbnm_heap_from_plcrs(siblings);
		if (root->pbnm_rank != curr->pbnm_rank)
			break;

		root = pbnm_heap_join(root, curr, compare);

		siblings = nxt;
	}

	plcrs_link_sibling(&root->pbnm_plcrs, siblings);

	return &root->pbnm_plcrs;
}

static struct pbnm_heap_node *
pbnm_heap_merge_2roots(struct plcrs_node    **restrict first,
                       struct plcrs_node    **restrict second,
                       pbnm_heap_compare_fn  *compare)
{
	assert(first);
	assert(second);
	assert(first != second);
	assert(compare);

	struct pbnm_heap_node *fst;
	struct pbnm_heap_node *snd;

	fst = pbnm_heap_from_plcrs(*first);
	snd = pbnm_heap_from_plcrs(*second);

	if (fst->pbnm_rank == snd->pbnm_rank) {
		*first = plcrs_next_sibling(*first);
		*second = plcrs_next_sibling(*second);

		return pbnm_heap_join(fst, snd, compare);
	}
	else if (fst->pbnm_rank < snd->pbnm_rank) {
		*first = plcrs_next_sibling(*first);

		return fst;
	}
	else {
		*second = plcrs_next_sibling(*second);

		return snd;
	}
}

static struct plcrs_node *
pbnm_heap_merge_trees(struct plcrs_node    *first,
                      struct plcrs_node    *second,
                      pbnm_heap_compare_fn *compare)
{
	assert(first);
	assert(second);
	assert(first != second);
	assert(compare);

	struct plcrs_node  *res;
	struct plcrs_node **tail = &res;

	res = &pbnm_heap_merge_2roots(&first, &second, compare)->pbnm_plcrs;

	while (first && second) {
		struct pbnm_heap_node *last;
		struct pbnm_heap_node *tmp;

		assert(tail);

		last = pbnm_heap_from_plcrs(*tail);
		tmp = pbnm_heap_merge_2roots(&first, &second, compare);
		assert(last->pbnm_rank <= tmp->pbnm_rank);

		if (last->pbnm_rank != tmp->pbnm_rank)
			tail = plcrs_sibling_ref(*tail);
		else
			tmp = pbnm_heap_join(last, tmp, compare);

		*tail = &tmp->pbnm_plcrs;
	}

	if (!first)
		first = second;

	*tail = pbnm_heap_merge_1root(pbnm_heap_from_plcrs(*tail), first,
	                             compare);

	return res;
}

static void
pbnm_heap_remove_root(struct pbnm_heap   *heap,
                      struct plcrs_node **previous,
                      struct plcrs_node  *root)
{
	assert(heap);
	assert(previous);
	assert(root);
	assert(*previous == root);

	struct plcrs_node *trees = NULL;

	heap->pbnm_count--;

	*previous = plcrs_next_sibling(root);

	root = plcrs_youngest_sibling(root);
	while (root) {
		struct plcrs_node *nxt;

		nxt = plcrs_next_sibling(root);

		root->plcrs_parent = NULL;
		plcrs_link_sibling(root, trees);
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

void
pbnm_heap_insert(struct pbnm_heap *heap, struct pbnm_heap_node *key)
{
	pbnm_heap_assert(heap);
	pbnm_heap_assert_node(key);

	plcrs_init_node(&key->pbnm_plcrs);
	key->pbnm_rank = 0;

	heap->pbnm_count++;

	heap->pbnm_roots = pbnm_heap_merge_1root(key, heap->pbnm_roots,
	                                         heap->pbnm_compare);
}

struct pbnm_heap_node *
pbnm_heap_peek(struct pbnm_heap *heap)
{
	pbnm_heap_assert(heap);
	assert(heap->pbnm_count);

	struct pbnm_heap_node *key;
	struct pbnm_heap_node *curr;

	key = pbnm_heap_from_plcrs(heap->pbnm_roots);

	curr = key;
	while (true) {
		struct pbnm_heap_node *nxt;

		nxt = pbnm_heap_next_node(curr);
		if (!nxt)
			break;

		if (heap->pbnm_compare(nxt, key) < 0)
			key = nxt;

		curr = nxt;
	}

	return key;
}

struct pbnm_heap_node *
pbnm_heap_extract(struct pbnm_heap *heap)
{
	pbnm_heap_assert(heap);
	assert(heap->pbnm_count);

	struct plcrs_node     **prev = &heap->pbnm_roots;
	struct pbnm_heap_node  *key;
	struct pbnm_heap_node  *curr;

	key = pbnm_heap_from_plcrs(*prev);

	curr = key;
	while (true) {
		struct pbnm_heap_node *nxt;

		nxt = pbnm_heap_next_node(curr);
		if (!nxt)
			break;

		if (heap->pbnm_compare(nxt, key) < 0) {
			prev = plcrs_sibling_ref(&curr->pbnm_plcrs);
			key = nxt;
		}

		curr = nxt;
	}

	pbnm_heap_remove_root(heap, prev, &key->pbnm_plcrs);

	return key;
}

static struct pbnm_heap_node *
_pbnm_heap_remove(struct pbnm_heap *heap, struct pbnm_heap_node *key)
{
	pbnm_heap_assert(heap);
	assert(heap->pbnm_count);

	struct plcrs_node **prev;
	
	while (true) {
		struct pbnm_heap_node *pkey;

		pkey = pbnm_heap_parent_node(key);
		if (!pkey)
			break;

		pbnm_heap_swap(pkey, key);

		key = pkey;
	}

	prev = &heap->pbnm_roots;
	while ((*prev) != &key->pbnm_plcrs)
		prev = plcrs_sibling_ref(*prev);

	pbnm_heap_remove_root(heap, prev, &key->pbnm_plcrs);

	return key;
}

void
pbnm_heap_remove(struct pbnm_heap *heap, struct pbnm_heap_node *key)
{
	_pbnm_heap_remove(heap, key);
}

void
pbnm_heap_promote(struct pbnm_heap *heap, struct pbnm_heap_node *key)
{
	pbnm_heap_assert(heap);
	assert(heap->pbnm_count);

	while (true) {
		struct pbnm_heap_node *pkey;

		pkey = pbnm_heap_parent_node(key);
		if (!pkey)
			break;

		if (heap->pbnm_compare(pkey, key) <= 0)
			break;

		pbnm_heap_swap(pkey, key);

		key = pkey;
	}
}

void
pbnm_heap_demote(struct pbnm_heap *heap, struct pbnm_heap_node *key)
{
	key = _pbnm_heap_remove(heap, key);

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
