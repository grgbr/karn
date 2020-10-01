#ifndef _KARN_PAVL_H
#define _KARN_PAVL_H

#include <karn/common.h>

enum pavl_side {
	PAVL_LEFT_SIDE  = 0,
	PAVL_RIGHT_SIDE = 1,
	PAVL_SIDE_NR
};

struct pavl_node {
	struct pavl_node *children[PAVL_SIDE_NR];
	struct pavl_node *parent;
	signed char       balance;
};

struct pavl_scan {
	struct pavl_node *parent;
	struct pavl_node *top;
	enum pavl_side    side;
};

typedef int (pavl_compare_node_key_fn)(const struct pavl_node *node,
                                       const void             *key,
                                       const void             *data);

typedef void (pavl_release_node_fn)(struct pavl_node *node, void *data);

struct pavl_tree {
	unsigned long             count;
	struct pavl_node         *root;
	void                     *data;
	pavl_compare_node_key_fn *compare;
	pavl_release_node_fn     *release;
};

#define PAVL_INIT_TREE(_compare, _release, _data) \
	{ \
		.count   = 0, \
		.root    = NULL, \
		.data    = _data, \
		.compare = _compare, \
		.release = _release \
	}

static inline unsigned long
pavl_tree_count(const struct pavl_tree *tree)
{
	karn_assert(tree);

	return tree->count;
}

extern struct pavl_node *
pavl_scan_key(const struct pavl_tree *tree,
              const void             *key,
              struct pavl_scan       *scan);

extern void
pavl_append_scan_node(struct pavl_tree       *tree,
                      struct pavl_node       *node,
                      const struct pavl_scan *scan);

extern int
pavl_append_node(struct pavl_tree *tree,
                 struct pavl_node *node,
                 const void       *key);

extern void
pavl_replace_node(struct pavl_tree       *tree,
                  const struct pavl_node *old,
                  struct pavl_node       *new);

extern struct pavl_node *
pavl_insert_node(struct pavl_tree *tree,
                 struct pavl_node *node,
                 const void       *key);

extern void
pavl_delete_node(struct pavl_tree *tree, struct pavl_node *node);

extern struct pavl_node *
pavl_delete_key(struct pavl_tree *tree, const void *key);

extern void
pavl_clear_tree(struct pavl_tree *tree);

typedef struct pavl_node * (pavl_clone_node_fn)(const struct pavl_node *orig,
                                                void                   *data);

extern int
pavl_clone_tree(struct pavl_tree       *tree,
                const struct pavl_tree *orig,
                pavl_clone_node_fn     *clone,
                void                   *data);

extern struct pavl_node *
pavl_find_node(const struct pavl_tree *tree, const void *key);

typedef struct pavl_node * (pavl_get_node_byid_fn)(unsigned    index,
                                                   const void *keys);

extern void
pavl_load_tree_from_sorted(struct pavl_tree      *tree,
                           const void            *keys,
                           unsigned int           nr,
                           pavl_get_node_byid_fn *get);

extern void
pavl_init_tree(struct pavl_tree         *tree,
               pavl_compare_node_key_fn *compare,
               pavl_release_node_fn     *release,
               void                     *data);

extern void
pavl_fini_tree(struct pavl_tree *tree);

/******************************************************************************
 * "Parented" AVL tree inorder iterator / traversal
 ******************************************************************************/

extern struct pavl_node *
pavl_iter_first_inorder(const struct pavl_tree *tree);

extern struct pavl_node *
pavl_iter_next_inorder(const struct pavl_node *node);

#define pavl_walk_forward_inorder(_tree, _node) \
	for (_node = pavl_iter_first_inorder(_tree); \
	     _node; \
	     _node = pavl_iter_next_inorder(_node))

#define pavl_find_and_walk_forward_inorder(_tree, _node, _key) \
	for (_node = pavl_find_node(_tree, _key); \
	     _node; \
	     _node = pavl_iter_next_inorder(_node))

extern struct pavl_node *
pavl_iter_last_inorder(const struct pavl_tree *tree);

extern struct pavl_node *
pavl_iter_prev_inorder(const struct pavl_node *node);

#define pavl_walk_backward_inorder(_tree, _node) \
	for (_node = pavl_iter_last_inorder(_tree); \
	     _node; \
	     _node = pavl_iter_prev_inorder(_node))

#define pavl_find_and_walk_backward_inorder(_tree, _node, _key) \
	for (_node = pavl_find_node(_tree, _key); \
	     _node; \
	     _node = pavl_iter_prev_inorder(_node))

/******************************************************************************
 * "Parented" AVL tree preorder traversal
 ******************************************************************************/

extern struct pavl_node *
pavl_iter_first_preorder(const struct pavl_tree *tree);

extern struct pavl_node *
pavl_iter_next_preorder(const struct pavl_node *node);

#define pavl_walk_forward_preorder(_tree, _node) \
	for (_node = pavl_iter_first_preorder(_tree); \
	     _node; \
	     _node = pavl_iter_next_preorder(_node))

extern struct pavl_node *
pavl_iter_prev_preorder(const struct pavl_node *node);

#define pavl_walk_backward_preorder(_tree, _node) \
	for (_node = pavl_iter_first_preorder(_tree); \
	     _node; \
	     _node = pavl_iter_prev_preorder(_node))

/******************************************************************************
 * "Parented" AVL tree printer and checker
 ******************************************************************************/

#if defined(CONFIG_KARN_PAVL_TEST)

#include <stdbool.h>

typedef void (pavl_display_node_fn)(const struct pavl_node *node);

extern int
pavl_print_tree(const struct pavl_tree *tree,
                unsigned int            max_depth,
                pavl_display_node_fn   *display);

typedef int (pavl_compare_nodes_fn)(const struct pavl_node *first,
                                    const struct pavl_node *second);

extern bool
pavl_check_tree(const struct pavl_tree *tree,
                unsigned long           count,
                pavl_compare_nodes_fn  *compare);

#endif /* defined(CONFIG_KARN_PAVL_TEST) */

#endif /* _KARN_PAVL_H */
