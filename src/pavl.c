#include <karn/pavl.h>
#include <utils/cdefs.h>
#include <utils/pow2.h>
#include <stdbool.h>
#include <errno.h>

/******************************************************************************
 * Utils
 ******************************************************************************/

#define pavl_assert(_tree) \
	karn_assert(_tree); \
	karn_assert((_tree)->compare)

static enum pavl_side
pavl_shallow_node_child(const struct pavl_node *node)
{
	karn_assert(node);

	return (node->balance > 0) ? PAVL_LEFT_SIDE : PAVL_RIGHT_SIDE;
}

static enum pavl_side
pavl_node_child_side(const struct pavl_node *node,
                     const struct pavl_node *child)
{
	karn_assert(node);
	karn_assert(child);

	karn_assert((node->children[PAVL_LEFT_SIDE] == child) ||
	            (node->children[PAVL_RIGHT_SIDE] == child));

	return (node->children[PAVL_LEFT_SIDE] != child);
}

static struct pavl_node **
pavl_parent_slot(struct pavl_tree *tree, const struct pavl_node *node)
{
	pavl_assert(tree);
	karn_assert(node);

	if (node->parent) {
		enum pavl_side side = pavl_node_child_side(node->parent, node);

		return &node->parent->children[side];
	}

	return &tree->root;
}

static struct pavl_node *
pavl_single_rotate(struct pavl_node **slot,
                   struct pavl_node  *child,
                   enum pavl_side     side)
{
	karn_assert(slot);
	karn_assert(*slot);
	karn_assert((*slot)->children[!side] == child);
	karn_assert(child);
	karn_assert(child->parent == (*slot));

	struct pavl_node *node = *slot;
	struct pavl_node *grand = child->children[side];

	if (grand) {
		karn_assert(grand->parent == child);
		grand->parent = node;
	}
	child->children[side] = node;
	child->parent = node->parent;
	node->children[!side] = grand;
	node->parent = child;

	/* Set new top-level node. */
	*slot = child;

	return child;
}

static struct pavl_node *
pavl_double_rotate(struct pavl_node **slot,
                   struct pavl_node  *child,
                   enum pavl_side     side)
{
	karn_assert(slot);
	karn_assert(*slot);
	karn_assert((*slot)->children[!side] == child);
	karn_assert(child);
	karn_assert(child->parent == (*slot));
	karn_assert(child->children[side]);
	karn_assert(child->children[side]->parent == child);

	struct pavl_node  *node = *slot;
	struct pavl_node  *grand = child->children[side];
	struct pavl_node **siblings = grand->children;
	struct pavl_node  *great;

	/* pavl_single_rotate(&node->children[!side], grand, !side); */
	great = siblings[!side];
	if (great) {
		karn_assert(great->parent == grand);
		great->parent = child;
	}
	siblings[!side] = child;
	child->children[side] = great;
	child->parent = grand;

	/* pavl_single_rotate(slot, grand, side); */
	great = siblings[side];
	if (great) {
		karn_assert(great->parent == grand);
		great->parent = node;
	}
	siblings[side] = node;
	grand->parent = node->parent;
	node->children[!side] = great;
	node->parent = grand;

	*slot = grand;

	/* Update balance factor. */
	switch (grand->balance) {
	case -1:
		node->balance = (char)side;
		child->balance = (char)(!side);
		break;

	case 0:
		node->balance = 0;
		child->balance = 0;
		break;

	case 1:
		node->balance = 0 - (char)(!side);
		child->balance = 0 - (char)side;
		break;

	default:
		karn_assert(0);
	}

	grand->balance = 0;

	return grand;
}

static void
pavl_init_leaf_node(struct pavl_node *node, const struct pavl_node *parent)
{
	node->children[PAVL_LEFT_SIDE] = NULL;
	node->children[PAVL_RIGHT_SIDE] = NULL;
	node->balance =  0;
	node->parent = (struct pavl_node *)parent;
}

/******************************************************************************
 * Insertion handling
 ******************************************************************************/

struct pavl_scan {
	struct pavl_node *parent;
	struct pavl_node *top;
	enum pavl_side    side;
};

static struct pavl_node *
pavl_scan_key(const struct pavl_tree *tree,
              const void             *key,
              struct pavl_scan       *scan)
{
	struct pavl_node *node = tree->root;
	int               result = 0;

	scan->parent = NULL;
	scan->top = node;

	while (node) {
		result = tree->compare(node, key, tree->data);
		if (!result)
			return node;

		if (node->balance)
			scan->top = node;

		scan->parent = node;
		node = node->children[result < 0];
	}

	scan->side = result < 0;

	return NULL;
}

static void
pavl_post_append_rebalance(struct pavl_tree       *tree,
                           struct pavl_node       *node,
                           const struct pavl_node *top)
{
	enum pavl_side side;

	do {
		side = pavl_node_child_side(node->parent, node);
		node->parent->balance += (side == PAVL_LEFT_SIDE) ? -1 : 1;

		node = node->parent;
	} while (node != top);

	if (uabs(node->balance) == 2) {
		/* Rebalancing required. */
		struct pavl_node **slot = pavl_parent_slot(tree, node);
		struct pavl_node  *child;

		side = pavl_shallow_node_child(node);
		child = node->children[!side];

		if (pavl_shallow_node_child(child) == side) {
			karn_assert(node->balance ==
			            ((side == PAVL_LEFT_SIDE) ? 2 : -2));
			karn_assert(child->balance ==
			            ((side == PAVL_LEFT_SIDE) ? 1 : -1));

			node->balance = 0;
			child->balance = 0;
			pavl_single_rotate(slot, child, side);
		}
		else
			pavl_double_rotate(slot, child, side);
	}
}

static void
pavl_post_scan_append_node(struct pavl_tree       *tree,
                           struct pavl_node       *node,
                           const struct pavl_scan *scan)
{
	karn_assert(scan);

	pavl_init_leaf_node(node, scan->parent);

	if (scan->parent) {
		karn_assert(scan->top);

		scan->parent->children[scan->side] = node;
		pavl_post_append_rebalance(tree, node, scan->top);
	}
	else
		tree->root = node;

	tree->count++;
}

int
pavl_append_node(struct pavl_tree *tree,
                 struct pavl_node *node,
                 const void       *key)
{
	pavl_assert(tree);
	karn_assert(node);

	struct pavl_scan scan;

	if (pavl_scan_key(tree, key, &scan))
		return -EEXIST;

	pavl_post_scan_append_node(tree, node, &scan);

	return 0;
}

void
pavl_replace_node(struct pavl_tree       *tree,
                  const struct pavl_node *old,
                  struct pavl_node       *new)
{
	pavl_assert(tree);
	karn_assert(tree->count);
	karn_assert(old);
	karn_assert(new);

	if (old->parent) {
		enum pavl_side side = pavl_node_child_side(old->parent, old);

		old->parent->children[side] = new;
	}
	else
		tree->root = new;

	*new = *old;

	if (old->children[PAVL_LEFT_SIDE])
		old->children[PAVL_LEFT_SIDE]->parent = new;

	if (old->children[PAVL_RIGHT_SIDE])
		old->children[PAVL_RIGHT_SIDE]->parent = new;
}

struct pavl_node *
pavl_insert_node(struct pavl_tree *tree,
                 struct pavl_node *node,
                 const void       *key)
{
	pavl_assert(tree);
	karn_assert(node);

	struct pavl_scan  scan;
	struct pavl_node *old;

	old = pavl_scan_key(tree, key, &scan);
	if (!old) {
		pavl_post_scan_append_node(tree, node, &scan);
		return NULL;
	}

	pavl_replace_node(tree, old, node);

	return old;
}

/******************************************************************************
 * Deletion handling
 ******************************************************************************/

static struct pavl_node *
pavl_remove_node(struct pavl_tree *tree,
                 struct pavl_node *node,
                 enum pavl_side   *from)
{
	struct pavl_node  *parent = node->parent;
	enum pavl_side     side;
	struct pavl_node **slot;
	struct pavl_node  *tmp;

	if (parent) {
		side = pavl_node_child_side(parent, node);
		slot = &parent->children[side];
	}
	else {
		side = PAVL_LEFT_SIDE;
		slot = &tree->root;
	}

	tmp = node->children[PAVL_RIGHT_SIDE];
	if (!tmp) {
		/*
		 * "node" has no right child: simply replace it with "node"'s
		 * left child.
		 */
		tmp = node->children[PAVL_LEFT_SIDE];
		if (tmp)
			tmp->parent = parent;
	}
	else if (!tmp->children[PAVL_LEFT_SIDE]) {
		/*
		 * "node"'s right child, "tmp", has no left child:
		 * replace "node" by "tmp", attaching "node"’s left
		 * child to "tmp".
		 */
		tmp->parent = parent;

		/*
		 * In addition, "tmp" acquires "node"'s balance factor
		 * and ...
		 */
		tmp->balance = node->balance;

		tmp->children[PAVL_LEFT_SIDE] = node->children[PAVL_LEFT_SIDE];

		if (node->children[PAVL_LEFT_SIDE])
			node->children[PAVL_LEFT_SIDE]->parent = tmp;

		/*
		 * ... "tmp" must be added to the stack of nodes above
		 * the deleted node "node".
		 */
		parent = tmp;
		side = PAVL_RIGHT_SIDE;
	}
	else {
		/*
		 * "node"'s right child, "tmp", has a left child:
		 * delete the inorder successor of "node", so we push the nodes
		 * above it onto the stack.
		 * The only trickery is that we do not know in advance the node
		 * that will replace "node", so we reserve a slot on the stack
		 * for it and fill it later.
		 */
		karn_assert(tmp);
		karn_assert(tmp->children[PAVL_LEFT_SIDE]);

		do {
			tmp = tmp->children[PAVL_LEFT_SIDE];
		} while (tmp->children[PAVL_LEFT_SIDE]);

		parent = tmp->parent;
		parent->children[PAVL_LEFT_SIDE] =
			tmp->children[PAVL_RIGHT_SIDE];

		if (tmp->children[PAVL_RIGHT_SIDE])
			tmp->children[PAVL_RIGHT_SIDE]->parent = parent;

		*tmp = *node;
		node->children[PAVL_LEFT_SIDE]->parent = tmp;
		node->children[PAVL_RIGHT_SIDE]->parent = tmp;

		side = PAVL_LEFT_SIDE;
	}

	*slot = tmp;
	*from = side;

	return parent;
}

static struct pavl_node *
pavl_post_remove_rebalance_node(struct pavl_tree  *tree,
                                struct pavl_node  *node,
                                enum pavl_side     from)
{
	karn_assert(node);
	karn_assert(node->balance >= -1);
	karn_assert(node->balance <= 1);

	char adjust = (from == PAVL_LEFT_SIDE) ? 1 : -1;

	node->balance += adjust;

	if (uabs(node->balance) == 2) {
		struct pavl_node  *child = node->children[!from];
		struct pavl_node **slot = pavl_parent_slot(tree, node);

		if (child->balance != (0 - adjust)) {
			karn_assert(node->balance == (2 * adjust));

			if (child->balance) {
				karn_assert(child->balance == adjust);

				child->balance = 0;
				node->balance = 0;
			}
			else {
				child->balance = 0 - adjust;
				node->balance = adjust;
			}

			node = pavl_single_rotate(slot, child, from);
		}
		else
			node = pavl_double_rotate(slot, child, from);
	}

	return node;
}

static void
pavl_post_remove_rebalance(struct pavl_tree *tree,
                           struct pavl_node *node,
                           enum pavl_side    from)
{
	while (true) {
		node = pavl_post_remove_rebalance_node(tree, node, from);
		if (node->balance || !node->parent)
			return;

		from = pavl_node_child_side(node->parent, node);
		node = node->parent;
	}
}

void
pavl_delete_node(struct pavl_tree *tree, struct pavl_node *node)
{
	pavl_assert(tree);
	karn_assert(tree->count);
	karn_assert(node);

	enum pavl_side from;

	node = pavl_remove_node(tree, node, &from);
	if (node)
		pavl_post_remove_rebalance(tree, node, from);

	tree->count--;
}

struct pavl_node *
pavl_delete_key(struct pavl_tree *tree, const void *key)
{
	pavl_assert(tree);
	karn_assert(tree->count);

	struct pavl_node *node;

	node = pavl_find_node(tree, key);
	if (!node)
		return NULL;

	pavl_delete_node(tree, node);

	return node;
}

/******************************************************************************
 * Finder
 ******************************************************************************/

struct pavl_node *
pavl_find_node(const struct pavl_tree *tree, const void *key)
{
	pavl_assert(tree);

	struct pavl_node *node = tree->root;

	while (node) {
		int result = tree->compare(node, key, tree->data);

		if (!result)
			return node;

		node = node->children[result < 0];
	}

	return NULL;
}

/******************************************************************************
 * Bulk loading a tree
 ******************************************************************************/

static struct pavl_node *
pavl_load_leaf_node(const void            *keys,
                    unsigned int           first,
                    pavl_get_node_byid_fn *get)
{
	struct pavl_node *leaf = get(first, keys);

	leaf->children[PAVL_LEFT_SIDE] = NULL;
	leaf->children[PAVL_RIGHT_SIDE] = NULL;
	leaf->balance =  0;

	return leaf;
}

static struct pavl_node *
pavl_load_partial_terminal_subtree(const void            *keys,
                                   unsigned int           first,
                                   pavl_get_node_byid_fn *get)
{
	struct pavl_node *left = get(first, keys);
	struct pavl_node *root = get(first + 1, keys);

	pavl_init_leaf_node(left, root);

	root->children[PAVL_LEFT_SIDE] = left;
	root->children[PAVL_RIGHT_SIDE] = NULL;
	root->balance =  -1;

	return root;
}

static struct pavl_node *
pavl_load_full_terminal_subtree(const void            *keys,
                                unsigned int           first,
                                pavl_get_node_byid_fn *get)
{
	struct pavl_node *left = get(first, keys);
	struct pavl_node *root = get(first + 1, keys);
	struct pavl_node *right = get(first + 2, keys);

	pavl_init_leaf_node(left, root);

	root->children[PAVL_LEFT_SIDE] = left;
	root->children[PAVL_RIGHT_SIDE] = right;
	root->balance =  0;

	*right = *left;

	return root;
}

struct pavl_load_part {
	struct pavl_node *parent;
	unsigned int      begin;
	unsigned int      count;
};

static unsigned int
pavl_load_stack_depth(unsigned int nr)
{
	return pow2_lower(umax((nr + 1) / 2, 1U));
}

static void
pavl_load_subtree(struct pavl_tree      *tree,
                  const void            *keys,
                  unsigned int           nr,
                  pavl_get_node_byid_fn *get)

{
	struct pavl_node      *parent = NULL;
	enum pavl_side         side = PAVL_LEFT_SIDE;
	unsigned int           begin = 0;
	unsigned int           cnt = nr;
	unsigned int           ptop = 0;
	struct pavl_load_part  parts[pavl_load_stack_depth(nr)];

	while (true) {
		struct pavl_node **slot;
		struct pavl_node  *root;

		if (parent)
			slot = &parent->children[side];
		else
			slot = &tree->root;

		if (cnt > 3) {
			karn_assert(ptop < pavl_load_stack_depth(nr));

			unsigned int left_cnt = (cnt - 1) / 2;
			unsigned int right_cnt = cnt - left_cnt - 1;

			root = get(begin + left_cnt, keys);
			root->balance = pow2_upper(right_cnt + 1) -
			                pow2_upper(left_cnt + 1);
			root->parent = parent;
			*slot = root;

			parts[ptop].parent = root;
			parts[ptop].begin = begin + left_cnt + 1;
			parts[ptop++].count = right_cnt;

			parent = root;
			side = PAVL_LEFT_SIDE;
			cnt = left_cnt;

			continue;
		}

		switch (cnt) {
		case 1:
			root = pavl_load_leaf_node(keys, begin, get);
			break;

		case 2:
			root = pavl_load_partial_terminal_subtree(keys,
			                                          begin,
			                                          get);
			break;

		case 3:
			root = pavl_load_full_terminal_subtree(keys,
			                                       begin,
			                                       get);
			break;

		default:
			karn_assert(0);
			unreachable();
		}

		root->parent = parent;
		*slot = root;

		if (!ptop--)
			return;

		parent = parts[ptop].parent;
		side = PAVL_RIGHT_SIDE;
		begin = parts[ptop].begin;
		cnt = parts[ptop].count;
	}
}

void
pavl_load_tree_from_sorted(struct pavl_tree      *tree,
                           const void            *keys,
                           unsigned int           nr,
                           pavl_get_node_byid_fn *get)

{
	pavl_assert(tree);
	karn_assert(!tree->count);
	karn_assert(!tree->root);
	karn_assert(!nr || keys);
	karn_assert(get);

	if (!nr)
		return;

	pavl_load_subtree(tree, keys, nr, get);

	tree->count = nr;
}

/******************************************************************************
 * Clearing tree content
 ******************************************************************************/

void
pavl_clear_tree(struct pavl_tree *tree)
{
	pavl_assert(tree);

	if (tree->release) {
		struct pavl_node *node = tree->root;

		while (node) {
			struct pavl_node *left = node->children[PAVL_LEFT_SIDE];

			if (!left) {
				left = node->children[PAVL_RIGHT_SIDE];
				tree->release(node, tree->data);
			}
			else {
				node->children[PAVL_LEFT_SIDE] =
					left->children[PAVL_RIGHT_SIDE];
				left->children[PAVL_RIGHT_SIDE] = node;
			}

			node = left;
		}
	}

	tree->count = 0;
	tree->root = NULL;
}

/******************************************************************************
 * Copying / cloning a tree
 ******************************************************************************/

static struct pavl_node *
pavl_clone_node(const struct pavl_node *orig,
                struct pavl_node       *parent,
                pavl_clone_node_fn     *clone,
                void                   *data)
{
	karn_assert(orig);

	struct pavl_node *node;

	node = clone(orig, data);
	if (!node)
		return NULL;

	node->children[PAVL_LEFT_SIDE] = NULL;
	node->children[PAVL_RIGHT_SIDE] = NULL;
	node->parent = parent;
	node->balance = orig->balance;

	return node;
}

static const struct pavl_node *
pavl_next_preorder_clone_node(const struct pavl_node  *orig,
                              struct pavl_node       **node)
{
	karn_assert(orig);
	karn_assert(node);

	enum pavl_side from;

	do {
		karn_assert(*node);

		if (!orig->parent)
			return NULL;

		from = pavl_node_child_side(orig->parent, orig);

		orig = orig->parent;
		*node = (*node)->parent;
	} while ((from == PAVL_RIGHT_SIDE) || !orig->children[PAVL_RIGHT_SIDE]);

	return orig->children[PAVL_RIGHT_SIDE];
}

int
pavl_clone_tree(struct pavl_tree       *tree,
                const struct pavl_tree *orig,
                pavl_clone_node_fn     *clone,
                void                   *data)
{
	pavl_assert(tree);
	karn_assert(!tree->root);
	karn_assert(!tree->count);
	pavl_assert(orig);
	karn_assert(clone);

	const struct pavl_node *src;
	struct pavl_node       *dst;
	int                     err;

	if (!orig->root)
		return 0;

	src = orig->root;
	dst = pavl_clone_node(src, NULL, clone, data);
	if (!dst)
		return -errno;

	tree->count = orig->count;
	tree->root = dst;

	while (src) {
		karn_assert(dst);

		struct pavl_node *child;

		if (src->children[PAVL_LEFT_SIDE]) {
			src = src->children[PAVL_LEFT_SIDE];
			child = pavl_clone_node(src, dst, clone, data);
			dst->children[PAVL_LEFT_SIDE] = child;
		}
		else if (src->children[PAVL_RIGHT_SIDE]) {
			src = src->children[PAVL_RIGHT_SIDE];
			child = pavl_clone_node(src, dst, clone, data);
			dst->children[PAVL_RIGHT_SIDE] = child;
		}
		else {
			src = pavl_next_preorder_clone_node(src, &dst);
			if (!src)
				break;

			child = pavl_clone_node(src, dst, clone, data);
			dst->children[PAVL_RIGHT_SIDE] = child;
		}

		if (!child)
			goto fail;

		dst = child;
	}

	return 0;

fail:
	err = -errno;
	pavl_clear_tree(tree);

	return err;
}

/******************************************************************************
 * Setup / teardown
 ******************************************************************************/

void
pavl_init_tree(struct pavl_tree         *tree,
               pavl_compare_node_key_fn *compare,
               pavl_release_node_fn     *release,
               void                     *data)
{
	karn_assert(tree);
	karn_assert(compare);

	tree->count = 0;
	tree->root = NULL;
	tree->data = data;
	tree->compare = compare;
	tree->release = release;
}

void
pavl_fini_tree(struct pavl_tree *tree)
{
	pavl_clear_tree(tree);
}

/******************************************************************************
 * Preorder iterator / traversal
 ******************************************************************************/

static const struct pavl_node *
pavl_preorder_iter_up(const struct pavl_node *node, enum pavl_side side)
{
	karn_assert(node);

	enum pavl_side from;

	do {
		if (!node->parent)
			return NULL;

		from = pavl_node_child_side(node->parent, node);
		node = node->parent;
	} while ((from == side) || !node->children[side]);

	return node->children[side];
}

static const struct pavl_node *
pavl_step_preorder_iter(const struct pavl_node *node, enum pavl_side order)
{
	karn_assert(node);

	if (node->children[order])
		return node->children[order];

	if (node->children[!order])
		return node->children[!order];

	return pavl_preorder_iter_up(node, !order);
}

struct pavl_node *
pavl_iter_next_preorder(const struct pavl_node *node)
{
	return (struct pavl_node *)pavl_step_preorder_iter(node,
	                                                   PAVL_LEFT_SIDE);
}

struct pavl_node *
pavl_iter_prev_preorder(const struct pavl_node *node)
{
	return (struct pavl_node *)pavl_step_preorder_iter(node,
	                                                   PAVL_RIGHT_SIDE);
}

struct pavl_node *
pavl_iter_first_preorder(const struct pavl_tree *tree)
{
	pavl_assert(tree);

	return tree->root;
}

/******************************************************************************
 * Inorder iterator / traversal
 ******************************************************************************/

static const struct pavl_node *
pavl_inorder_iter_down(const struct pavl_node *node, enum pavl_side side)
{
	karn_assert(node);

	while (node->children[side])
		node = node->children[side];

	return node;
}

static const struct pavl_node *
pavl_inorder_iter_up(const struct pavl_node *node, enum pavl_side side)
{
	karn_assert(node);

	enum pavl_side from;

	do {
		if (!node->parent)
			return NULL;

		from = pavl_node_child_side(node->parent, node);
		node = node->parent;
	} while (from == side);

	return node;
}

static const struct pavl_node *
pavl_step_inorder_iter(const struct pavl_node *node, enum pavl_side order)
{
	karn_assert(node);

	if (node->children[!order])
		return pavl_inorder_iter_down(node->children[!order], order);

	return pavl_inorder_iter_up(node, !order);
}

static struct pavl_node *
pavl_begin_inorder_iter(const struct pavl_tree *tree, enum pavl_side order)
{
	pavl_assert(tree);

	struct pavl_node *node = tree->root;

	if (node)
		node = (struct pavl_node *)pavl_inorder_iter_down(node, order);

	return node;
}

struct pavl_node *
pavl_iter_first_inorder(const struct pavl_tree *tree)
{
	return pavl_begin_inorder_iter(tree, PAVL_LEFT_SIDE);
}

struct pavl_node *
pavl_iter_next_inorder(const struct pavl_node *node)
{
	return (struct pavl_node *)pavl_step_inorder_iter(node, PAVL_LEFT_SIDE);
}

struct pavl_node *
pavl_iter_last_inorder(const struct pavl_tree *tree)
{
	return pavl_begin_inorder_iter(tree, PAVL_RIGHT_SIDE);
}

struct pavl_node *
pavl_iter_prev_inorder(const struct pavl_node *node)
{
	return (struct pavl_node *)pavl_step_inorder_iter(node,
	                                                  PAVL_RIGHT_SIDE);
}

/******************************************************************************
 * PAVL tree printer
 ******************************************************************************/

#if defined(CONFIG_KARN_PAVL_TEST)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * Height of an PAVL tree containing n nodes cannot exceed:
 *   1.44 * log2(n)
 *
 * Minimum number of nodes in an PAVL tree with height h is:
 *   N(h) = N(h-1) + N(h-2) + 1 for n>2 where N(0) = 1 and N(1) = 2
 */
static unsigned long
pavl_min_count(unsigned int depth)
{
	if (depth >= 3) {
		unsigned long cnt;       /* count for depth */
		unsigned long cnt_1 = 2; /* count for depth - 1 */
		unsigned long cnt_2 = 1; /* count for depth - 2 */
		unsigned int  d;

		for (d = 2; d < depth; d++) {
			cnt = 1 + cnt_1 + cnt_2;
			cnt_2 = cnt_1;
			cnt_1 = cnt;
		}

		return cnt;
	}

	return depth;
}

/*
 * An PAVL tree cannot hold more than 2^^depth - 1 nodes when perfectly balanced.
 */
static unsigned long
pavl_max_count(unsigned int depth)
{
	return (1UL << depth) - 1;
}

#define PAVL_PRINT_PREFIX_SIZE (256U)

struct pavl_printer {
	char                  prefix[PAVL_PRINT_PREFIX_SIZE];
	int                   colno;
	unsigned int          depth;
	unsigned int          max_depth;
	pavl_display_node_fn *display;
};

static void
pavl_print_subtree(const struct pavl_node *node, struct pavl_printer *printer);

static void
pavl_print_subtree_node(const struct pavl_node *node,
                        const struct pavl_node *sibling,
                        const char             *prefix,
                        size_t                  size,
                        struct pavl_printer    *printer)
{
	if (node) {
		printf("%s +-", printer->prefix);

		if ((size_t)printer->colno < (sizeof(printer->prefix) - size)) {
			memcpy(&printer->prefix[printer->colno], prefix, size);
			printer->colno += size - 1;

			printer->depth++;
			pavl_print_subtree(node, printer);
			printer->depth--;

			printer->colno -= size - 1;
			printer->prefix[printer->colno] = '\0';
		}
	}
	else if (sibling)
		printf("%s +-{null}\n", printer->prefix);
}

static void
pavl_print_subtree(const struct pavl_node *node, struct pavl_printer *printer)
{
	if (printer->depth < printer->max_depth) {
		const struct pavl_node *left = node->children[PAVL_LEFT_SIDE];
		const struct pavl_node *right = node->children[PAVL_RIGHT_SIDE];

		printer->display(node);
		putchar('\n');

		pavl_print_subtree_node(right,
		                       left,
		                       " |  ",
		                       sizeof(" |  "),
		                       printer);
		pavl_print_subtree_node(left,
		                       right,
		                       "    ",
		                       sizeof("    "),
		                       printer);
	}
}

int
pavl_print_tree(const struct pavl_tree *tree,
                unsigned int            max_depth,
                pavl_display_node_fn   *display)
{
	pavl_assert(tree);
	karn_assert(max_depth);

	if (tree->root) {
		struct pavl_printer *printer;

		printer = malloc(sizeof(*printer));
		if (!printer)
			return -ENOMEM;

		printer->prefix[0] = '\0';
		printer->colno = 0;
		printer->depth = 0;
		printer->max_depth = max_depth;
		printer->display = display;

		pavl_print_subtree(tree->root, printer);

		free(printer);
	}

	return 0;
}

static bool
pavl_recurs_check_subtree(const struct pavl_node *node,
                          int                    *height,
                          pavl_compare_nodes_fn  *compare)
{
	const struct pavl_node *left;
	int                     left_height;
	const struct pavl_node *right;
	int                     right_height;
	int                     balance;

	if (!node) {
		*height = 0;
		return true;
	}

	left = node->children[PAVL_LEFT_SIDE];
	if (left) {
		if (left->parent != node) {
			fprintf(stderr,
			        "karn: pavl: "
			        "invalid parent / left child pointers\n");
			return false;
		}
		if (compare(left, node) >= 0) {
			fprintf(stderr,
			        "karn: pavl: wrong tree node ordering: "
			        "left >= parent\n");
			return false;
		}
	}

	right = node->children[PAVL_RIGHT_SIDE];
	if (right) {
		if (right->parent != node) {
			fprintf(stderr,
			        "karn: pavl: "
			        "invalid parent / right child pointers\n");
			return false;
		}
		if (compare(right, node) <= 0) {
			fprintf(stderr,
			        "karn: pavl: wrong tree node ordering: "
			        "right <= parent\n");
			return false;
		}
	}

	if (!pavl_recurs_check_subtree(left, &left_height, compare))
		return false;
	if (!pavl_recurs_check_subtree(right, &right_height, compare))
		return false;

	balance = right_height - left_height;
	if ((balance < -1) || (balance > 1)) {
		fprintf(stderr, "karn: pavl: invalid node balance factor\n");
		return false;
	}
	if (balance != (int)node->balance) {
		fprintf(stderr, "karn: pavl: unexpected node balance factor\n");
		return false;
	}

	*height = 1 + ((left_height > right_height) ?
	               left_height : right_height);

	return true;
}

bool
pavl_check_tree(const struct pavl_tree *tree,
                unsigned long           count,
                pavl_compare_nodes_fn  *compare)
{
	int height = -1;

	if (pavl_tree_count(tree) != count) {
		fprintf(stderr,
		        "karn: pavl: unexpected tree node count: "
		        "%lu != %lu\n",
		        pavl_tree_count(tree),
		        count);
		return false;
	}

	if (!count) {
		if (tree->root) {
			fprintf(stderr,
			        "karn: pavl: invalid empty tree: "
			        "root node not NULL\n");
			return false;
		}

		return true;
	}
	else {
		if (tree->root->parent) {
			fprintf(stderr,
			        "karn: pavl: "
			        "invalid root node parent: not NULL\n");
			return false;
		}
	}

	if (!pavl_recurs_check_subtree(tree->root, &height, compare))
		return false;

	if (count > pavl_max_count(height)) {
		/*
		 * A true PAVL tree cannot hold so many nodes with such a
		 * small height.
		 */
		fprintf(stderr,
		        "karn: pavl: unexpectedly small tree height: "
		        "#nodes=%lu height=%d\n",
		        count,
		        height);
		return false;
	}
	if (count < pavl_min_count(height)) {
		/*
		 * A true PAVL tree shall not be so deep to hold the given number
		 * of nodes.
		 */
		fprintf(stderr,
		        "karn: pavl: unexpectedly large tree height: "
		        "#nodes=%lu height=%d %lu\n",
		        count,
		        height,
		        pavl_min_count(height));
		return false;
	}

	return true;
}

#endif /* defined(CONFIG_KARN_PAVL_TEST) */
