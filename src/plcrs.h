#ifndef _PLCRS_H
#define _PLCRS_H

#include "utils.h"
#include <assert.h>

struct plcrs_node {
	struct plcrs_node  *plcrs_sibling;
	struct plcrs_node  *plcrs_parent;
	struct plcrs_node  *plcrs_youngest;
};

static inline void
plcrs_link_sibling(struct plcrs_node       *restrict node,
                   const struct plcrs_node *restrict next)
{
	assert(node);
	assert(node != next);

	node->plcrs_sibling = (struct plcrs_node *)next;
}

static inline struct plcrs_node *
plcrs_next_sibling(const struct plcrs_node *node)
{
	assert(node);

	return node->plcrs_sibling;
}

static inline struct plcrs_node **
plcrs_sibling_ref(const struct plcrs_node *node)
{
	assert(node);

	return &((struct plcrs_node*)node)->plcrs_sibling;
}

static inline struct plcrs_node *
plcrs_parent_node(const struct plcrs_node *node)
{
	assert(node);

	return node->plcrs_parent;
}

static inline struct plcrs_node *
plcrs_youngest_sibling(const struct plcrs_node *node)
{
	assert(node);

	return node->plcrs_youngest;
}

static inline void
plcrs_join_tree(struct plcrs_node *restrict tree,
                struct plcrs_node *restrict parent)
{
	assert(tree);
	assert(parent);
	assert(tree != parent);

	tree->plcrs_sibling = parent->plcrs_youngest;
	tree->plcrs_parent = parent;

	parent->plcrs_youngest = tree;
}

static inline void
plcrs_init_node(struct plcrs_node *node)
{
	assert(node);

	node->plcrs_sibling = NULL;
	node->plcrs_parent = NULL;
	node->plcrs_youngest = NULL;
}

#define plcrs_entry(_node, _type, _member) \
	containerof(_node, _type, _member)

#endif
