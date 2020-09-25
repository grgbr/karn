#ifndef _KARN_AVL_H
#define _KARN_AVL_H

#include <karn/common.h>
#include <utils/pow2.h>
#include <stdbool.h>

#define AVL_MAX_DEPTH (32U)

/* Computed using avl_min_count() with AVL_MAX_DEPTH passed as argument. */
#define AVL_MAX_COUNT (5702886U)

enum avl_side {
	AVL_LEFT_SIDE   = 0,
	AVL_RIGHT_SIDE  = 1,
	AVL_SIDE_NR
};

struct avl_node {
	struct avl_node *children[AVL_SIDE_NR];
	signed char      balance;
};

struct avl_scan {
	unsigned long     height;
	struct avl_node **top_slot;
	unsigned long     children;
	struct avl_node **found_slot;
};

typedef int (avl_compare_node_key_fn)(const struct avl_node *node,
                                      const void            *key,
                                      const void            *data);

typedef void (avl_release_node_fn)(struct avl_node *node, void *data);

struct avl_tree {
	unsigned long            count;
	struct avl_node         *root;
	void                    *data;
	avl_compare_node_key_fn *compare;
	avl_release_node_fn     *release;
};

#define AVL_INIT_TREE(_compare, _release, _data) \
	{ \
		.count   = 0, \
		.root    = NULL, \
		.data    = _data, \
		.compare = _compare, \
		.release = _release \
	}

static inline unsigned long
avl_tree_count(const struct avl_tree *tree)
{
	karn_assert(tree);

	return tree->count;
}

static inline bool
avl_tree_full(const struct avl_tree *tree)
{
	karn_assert(tree);

	return (tree->count >= AVL_MAX_COUNT);
}

extern struct avl_node *
avl_scan_node(const struct avl_tree *tree,
              const void            *key,
              struct avl_scan       *scan);

extern void
avl_post_scan_append_node(struct avl_tree       *tree,
                          struct avl_node       *node,
                          const struct avl_scan *scan);

extern void
avl_post_scan_replace_node(struct avl_node *node, const struct avl_scan *scan);

extern int
avl_append_node(struct avl_tree *tree, struct avl_node *node, const void *key);

extern struct avl_node *
avl_replace_node(struct avl_tree *tree, struct avl_node *node, const void *key);

extern struct avl_node *
avl_insert_node(struct avl_tree *tree, struct avl_node *node, const void *key);

extern struct avl_node *
avl_delete_node(struct avl_tree *tree, const void *key);

extern void
avl_clear_tree(struct avl_tree *tree);

extern struct avl_node *
avl_find_node(const struct avl_tree *tree, const void *key);

extern void
avl_init_tree(struct avl_tree         *tree,
              avl_compare_node_key_fn *compare,
              avl_release_node_fn     *release,
              void                    *data);

extern void
avl_fini_tree(struct avl_tree *tree);

/******************************************************************************
 * AVL tree iterator / traversal
 ******************************************************************************/

struct avl_path {
	unsigned int depth;
	uintptr_t    stack[AVL_MAX_DEPTH];
};

struct avl_iter {
	struct avl_path path;
};

extern struct avl_node *
avl_iter_first(struct avl_iter *iter, const struct avl_tree *tree);

extern struct avl_node *
avl_iter_next(struct avl_iter *iter, const struct avl_node *node);

#define avl_walk_forward(_iter, _tree, _node) \
	for (_node = avl_iter_first(_iter, _tree); \
	     _node; \
	     _node = avl_iter_next(_iter, _node))

extern struct avl_node *
avl_iter_last(struct avl_iter *iter, const struct avl_tree *tree);

extern struct avl_node *
avl_iter_prev(struct avl_iter *iter, const struct avl_node *node);

#define avl_walk_backward(_iter, _tree, _node) \
	for (_node = avl_iter_last(_iter, _tree); \
	     _node; \
	     _node = avl_iter_prev(_iter, _node))

extern struct avl_node *
avl_iter_find(struct avl_iter       *iter,
              const struct avl_tree *tree,
              const void            *key);

#define avl_find_and_walk_forward(_iter, _tree, _node, _key) \
	for (_node = avl_iter_find(_iter, _tree, _key); \
	     _node; \
	     _node = avl_iter_next(_iter, _node))

#define avl_find_and_walk_backward(_iter, _tree, _node, _key) \
	for (_node = avl_iter_find(_iter, _tree, _key); \
	     _node; \
	     _node = avl_iter_prev(_iter, _node))

/******************************************************************************
 * AVL tree printer and checker
 ******************************************************************************/

#if defined(CONFIG_KARN_AVL_TEST)

typedef void (avl_display_node_fn)(const struct avl_node *node);

extern int
avl_print_tree(const struct avl_tree *tree,
               unsigned int           max_depth,
               avl_display_node_fn   *display);

typedef int (avl_compare_nodes_fn)(const struct avl_node *first,
                                   const struct avl_node *second);

extern bool
avl_check_tree(const struct avl_tree *tree,
               unsigned long          count,
               avl_compare_nodes_fn  *compare);

#endif /* defined(CONFIG_KARN_AVL_TEST) */

#endif /* _KARN_AVL_H */
