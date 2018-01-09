#include "plbst.h"

/*
 * Rotate a node around its "pivot" child.
 *
 * When rotating right, "pivot" is left child of "node".
 * When rotating left, "pivot" is right child of "node".
 *
 *       parent                              parent
 *         /                                   /
 *       node                                pivot
 *       /  \    direction == PLBST_RIGHT    /   \
 *   pivot   c  ------------------------->  a   node
 *   /   \                                      /  \
 *  a     b                                    b    c
 *
 *
 *       parent                             parent
 *         /                                  /
 *      pivot                               node
 *      /   \    direction == PLBST_LEFT    /  \
 *   node    c  <------------------------  a   pivot
 *   /  \                                      /   \
 *  a    b                                    b     c
 */
void
plbst_rotate(struct plbst_node *node,
             struct plbst_node *pivot,
             unsigned int       direction)
{
	assert(pivot);
	assert(direction < PLBST_NR);

	struct plbst_node *tmp = pivot->plbst_children[direction];
	unsigned int       dir = (direction + 1) % PLBST_NR;

	assert(plbst_child(node, dir) == pivot);
	assert(plbst_parent(pivot) == node);
	assert(plbst_child_from_link(pivot->plbst_parent) == dir);

	if (tmp)
		tmp->plbst_parent = plbst_link(node, dir);
	node->plbst_children[dir] = tmp;

	tmp = (struct plbst_node *)node->plbst_parent;

	node->plbst_parent = plbst_link(pivot, direction);
	pivot->plbst_children[direction] = node;

	pivot->plbst_parent = (uintptr_t)tmp;
	if (tmp)
		tmp->plbst_children[plbst_child_from_link((uintptr_t)tmp)] =
			pivot;
}

/*
 * Swap node with its "child" child.
 *
 *       parent              parent
 *         /                   /
 *       node      swap      child
 *       /  \    ------->    /  \
 *    child  c             node  c
 *     /  \                /  \
 *    a    b              a    b
 *
 */
void
plbst_swap(struct plbst_node *node, struct plbst_node *child)
{
	struct plbst_node *tmp;
	unsigned int       dir;

	tmp = child->plbst_children[PLBST_LEFT];
	if (tmp)
		tmp->plbst_parent = plbst_link(node, PLBST_LEFT);
	node->plbst_children[PLBST_LEFT] = tmp;

	tmp = child->plbst_children[PLBST_RIGHT];
	if (tmp)
		tmp->plbst_parent = plbst_link(node, PLBST_RIGHT);
	node->plbst_children[PLBST_RIGHT] = tmp;

	dir = plbst_child_from_link(child->plbst_parent);
	child->plbst_children[dir] = node;

	dir = (dir + 1) % PLBST_NR;
	child->plbst_children[dir] = node->plbst_children[dir];

	dir = plbst_child_from_link(node->plbst_parent);
	tmp = plbst_parent(node);

	node->plbst_parent = plbst_link(child, dir);

	child->plbst_parent = plbst_link(tmp, dir);
	if (tmp)
		tmp->plbst_children[dir] = child;
}
