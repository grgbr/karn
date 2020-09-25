#ifndef _KARN_PLCRS_H
#define _KARN_PLCRS_H

#include <karn/common.h>

struct plcrs_node {
	struct plcrs_node  *plcrs_sibling;
	struct plcrs_node  *plcrs_parent;
	struct plcrs_node  *plcrs_youngest;
};

static inline void
plcrs_link_sibling(struct plcrs_node       *restrict node,
                   const struct plcrs_node *restrict next)
{
	karn_assert(node);
	karn_assert(node != next);

	node->plcrs_sibling = (struct plcrs_node *)next;
}

static inline struct plcrs_node *
plcrs_next_sibling(const struct plcrs_node *node)
{
	karn_assert(node);

	return node->plcrs_sibling;
}

static inline struct plcrs_node **
plcrs_sibling_ref(const struct plcrs_node *node)
{
	karn_assert(node);

	return &((struct plcrs_node*)node)->plcrs_sibling;
}

static inline struct plcrs_node *
plcrs_parent_node(const struct plcrs_node *node)
{
	karn_assert(node);

	return node->plcrs_parent;
}

static inline struct plcrs_node *
plcrs_youngest_sibling(const struct plcrs_node *node)
{
	karn_assert(node);

	return node->plcrs_youngest;
}

static inline void
plcrs_join_tree(struct plcrs_node *restrict tree,
                struct plcrs_node *restrict parent)
{
	karn_assert(tree);
	karn_assert(parent);
	karn_assert(tree != parent);

	tree->plcrs_sibling = parent->plcrs_youngest;
	tree->plcrs_parent = parent;

	parent->plcrs_youngest = tree;
}

static inline void
plcrs_init_node(struct plcrs_node *node)
{
	karn_assert(node);

	node->plcrs_sibling = NULL;
	node->plcrs_parent = NULL;
	node->plcrs_youngest = NULL;
}

#define plcrs_entry(_node, _type, _member) \
	containerof(_node, _type, _member)

#endif /* _KARN_PLCRS_H */
