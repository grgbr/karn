#include "hpair_heap.h"

static struct plbst_node *
hpair_heap_link(struct plbst_node *first,
                struct plbst_node *second,
                hpair_compare_fn  *compare)
{
	struct hpair_node *fst;
	struct hpair_node *snd;
	struct plbst_node *parent;
	struct plbst_node *child;

	fst = plbst_entry(first, struct hpair_node, hpair_plbst);
	snd = plbst_entry(second, struct hpair_node, hpair_plbst);

	if (compare(fst, snd) <= 0) {
		parent = first;
		child = second;
	}
	else {
		parent = second;
		child = first;
	}

	plbst_join(plbst_child(parent, PLBST_LEFT), child, PLBST_RIGHT);
	plbst_join(child, parent, PLBST_LEFT);

	return parent;
}

static struct plbst_node *
hpair_heap_link_pair(struct plbst_node **trees, hpair_compare_fn *compare)
{
	struct plbst_node *curr = *trees;
	struct plbst_node *nxt;

	nxt = plbst_child(curr, PLBST_RIGHT);
	if (nxt) {
		*trees = plbst_child(nxt, PLBST_RIGHT);

		return hpair_heap_link(curr, nxt, compare);
	}

	*trees = NULL;

	return curr;
}

static struct plbst_node *
hpair_heap_1pass_link(struct plbst_node *trees, hpair_compare_fn *compare)
{
	struct plbst_node *root;

	root = hpair_heap_link_pair(&trees, compare);

	while (trees)
		root = hpair_heap_link(root,
		                       hpair_heap_link_pair(&trees, compare),
		                       compare);

	return root;
}

static struct plbst_node *
hpair_heap_2pass_link(struct plbst_node *trees, hpair_compare_fn *compare)
{
	struct plbst_node *tmp;
	struct plbst_node *root = NULL;

	do {
		tmp = hpair_heap_link_pair(&trees, compare);
		tmp->plbst_children[PLBST_RIGHT] = root;
		root = tmp;
	} while (trees);

	trees = plbst_child(root, PLBST_RIGHT);
	while (trees) {
		tmp = plbst_child(trees, PLBST_RIGHT);
		root = hpair_heap_link(root, trees, compare);
		trees = tmp;
	}

	return root;
}

static struct plbst_node *
hpair_heap_npass_link(struct plbst_node *trees, hpair_compare_fn *compare)
{
	while (plbst_child(trees, PLBST_RIGHT)) {
		struct plbst_node *root;
		struct plbst_node *tail;

		root = hpair_heap_link_pair(&trees, compare);
		tail = root;

		while (trees) {
			struct plbst_node *tmp;

			tmp = hpair_heap_link_pair(&trees, compare);
			tail->plbst_children[PLBST_RIGHT] = tmp;
			tail = tmp;
		}

		trees = root;
	}

	return trees;
}

static void
hpair_heap_cut(struct plbst_node *key)
{
	struct plbst_node *node;

	node = plbst_child(key, PLBST_RIGHT);
	if (node)
		plbst_replace(key, node);
	else
		plbst_split(key);
}

static struct plbst_node *
hpair_heap_remove_key(struct plbst_node *root,
                      struct plbst_node *key,
                      hpair_compare_fn  *compare)
{
	if (key != root) {
		hpair_heap_cut(key);
		plbst_join(plbst_child(key, PLBST_LEFT), root, PLBST_RIGHT);
	}
	else
		root = plbst_child(root, PLBST_LEFT);

	return hpair_heap_2pass_link(root, compare);
}

void
hpair_heap_insert(struct hpair_heap *heap, struct hpair_node *key)
{
	plbst_init_node(&key->hpair_plbst);

	if (heap->hpair_count++)
		heap->hpair_root = hpair_heap_link(heap->hpair_root,
		                                   &key->hpair_plbst,
		                                   heap->hpair_compare);
	else
		heap->hpair_root = &key->hpair_plbst;
}

struct hpair_node *
hpair_heap_extract(struct hpair_heap *heap)
{
	struct plbst_node *root = heap->hpair_root;

	if (--heap->hpair_count)
		heap->hpair_root =
			hpair_heap_2pass_link(plbst_child(root, PLBST_LEFT),
			                      heap->hpair_compare);
	else
		heap->hpair_root = NULL;

	return plbst_entry(root, struct hpair_node, hpair_plbst);
}

void
hpair_heap_merge(struct hpair_heap *result, struct hpair_heap *source)
{
	result->hpair_count += source->hpair_count;

	result->hpair_root = hpair_heap_link(result->hpair_root,
	                                     source->hpair_root,
	                                     result->hpair_compare);
}

void
hpair_heap_remove(struct hpair_heap *heap, struct hpair_node *key)
{
	if (--heap->hpair_count)
		heap->hpair_root = hpair_heap_remove_key(heap->hpair_root,
		                                         &key->hpair_plbst,
		                                         heap->hpair_compare);
	else
		heap->hpair_root = NULL;
}

void
hpair_heap_promote(struct hpair_heap *heap, struct hpair_node *key)
{
	if ((&key->hpair_plbst != heap->hpair_root) &&
	    (heap->hpair_count > 1)) {
		hpair_heap_cut(&key->hpair_plbst);

		heap->hpair_root = hpair_heap_link(heap->hpair_root,
		                                   &key->hpair_plbst,
		                                   heap->hpair_compare);
	}
}

void
hpair_heap_demote(struct hpair_heap *heap, struct hpair_node *key)
{
	if (heap->hpair_count > 1) {
		struct plbst_node *root;

		root = hpair_heap_remove_key(heap->hpair_root,
		                             &key->hpair_plbst,
		                             heap->hpair_compare);

		plbst_init_node(&key->hpair_plbst);

		heap->hpair_root = hpair_heap_link(root,
		                                   &key->hpair_plbst,
		                                   heap->hpair_compare);
	}
}
