#ifndef _KARN_PLBST_H
#define _KARN_PLBST_H

#ifndef CONFIG_PLBST
#error PLBST configuration disabled !
#endif

#include <utils.h>

#define PLBST_LEFT  (0U)
#define PLBST_RIGHT (1U)
#define PLBST_NR    (2U)

struct plbst_node {
	uintptr_t          plbst_parent;
	struct plbst_node *plbst_children[PLBST_NR];
};

#define plbst_entry(_node, _type, _member) \
	containerof(_node, _type, _member)

static inline uintptr_t
plbst_link(const struct plbst_node *node, unsigned int sib)
{
	assert(sib < PLBST_NR);
	assert(node || !(node || sib));

	return (uintptr_t)node | (uintptr_t)sib;
}

static inline struct plbst_node *
plbst_node_from_link(uintptr_t link)
{
	return (struct plbst_node *)(link & ~((uintptr_t)(PLBST_NR - 1)));
}

static inline unsigned int
plbst_sib_from_link(uintptr_t link)
{
	return (unsigned int)(link & (uintptr_t)(PLBST_NR - 1));
}

static inline struct plbst_node *
plbst_parent(const struct plbst_node *node)
{
	assert(node);

	return plbst_node_from_link(node->plbst_parent);
}

static inline void
plbst_assign_parent(struct plbst_node       *node,
                    const struct plbst_node *parent,
                    unsigned int             sib)
{
	assert(node);

	node->plbst_parent = plbst_link(parent, sib);
}

static inline struct plbst_node *
plbst_child(const struct plbst_node *node, unsigned int sib)
{
	assert(node);
	assert(sib < PLBST_NR);

	return (struct plbst_node *)node->plbst_children[sib];
}

static inline void
plbst_assign_child(struct plbst_node       *node,
                   const struct plbst_node *child,
                   unsigned int             sib)
{
	assert(node);

	node->plbst_children[sib] = (struct plbst_node *)child;
}

static inline void
plbst_join(struct plbst_node *node,
           struct plbst_node *parent,
           unsigned int       sib)
{
	assert(node);
	assert(parent);

	plbst_assign_parent(node, parent, sib);
	plbst_assign_child(parent, node, sib);
}

static inline void
plbst_split(const struct plbst_node *node)
{
	assert(node);

	struct plbst_node *parent;
	unsigned int       sib;

	parent = plbst_node_from_link(node->plbst_parent);
	sib = plbst_sib_from_link(node->plbst_parent);

	assert(plbst_child(parent, sib) == node);

	parent->plbst_children[sib] = NULL;
}

static inline void
plbst_replace(const struct plbst_node *node, struct plbst_node *replacement)
{
	assert(node);
	assert(replacement);

	struct plbst_node *parent;
	unsigned int       sib;

	parent = plbst_node_from_link(node->plbst_parent);
	sib = plbst_sib_from_link(node->plbst_parent);

	assert(parent);
	assert(parent->plbst_children[sib] == node);

	plbst_join(replacement, parent, sib);
}

static inline void
plbst_init_node(struct plbst_node *node)
{
	node->plbst_parent = (uintptr_t)0;
	node->plbst_children[PLBST_LEFT] = NULL;
	node->plbst_children[PLBST_RIGHT] = NULL;
}

extern void plbst_rotate(struct plbst_node *node,
                         struct plbst_node *pivot,
                         unsigned int       direction);

extern void plbst_swap(struct plbst_node *node, struct plbst_node *child);

#endif
