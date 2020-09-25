#include <karn/avl.h>
#include <errno.h>

#define avl_assert(_tree) \
	karn_assert(_tree); \
	karn_assert(!avl_tree_full(_tree)); \
	karn_assert((_tree)->compare)

/******************************************************************************
 * Tree path node / slot tracker
 ******************************************************************************/

static unsigned int
avl_reserve_path(struct avl_path *path)
{
	karn_assert(path->depth < AVL_MAX_DEPTH);

	return path->depth++;
}

static void
avl_fill_path(struct avl_path *path,
              unsigned int     id,
              uintptr_t        point,
              enum avl_side    side)
{
	karn_assert(path->depth <= AVL_MAX_DEPTH);
	karn_assert(id < path->depth);
	karn_assert(point);
	karn_assert(!(point & 0x1UL));

	path->stack[id] = point | (uintptr_t)side;
}

static void
avl_fill_path_slot(struct avl_path *path,
                   unsigned int     id,
                   struct avl_node **slot,
                   enum avl_side     side)
{
	avl_fill_path(path, id, (uintptr_t)slot, side);
}

static void
avl_push_path(struct avl_path *path,
              uintptr_t        point,
              enum avl_side    side)
{
	karn_assert(path->depth < AVL_MAX_DEPTH);
	karn_assert(point);
	karn_assert(!(point & 0x1UL));

	path->stack[path->depth++] = point | (uintptr_t)side;
}

static void
avl_push_path_node(struct avl_path       *path,
                   const struct avl_node *node,
                   enum avl_side          side)
{
	avl_push_path(path, (uintptr_t)node, side);
}

static void
avl_push_path_slot(struct avl_path               *path,
                   const struct avl_node * const *slot,
                   enum avl_side                  side)
{
	avl_push_path(path, (uintptr_t)slot, side);
}

static uintptr_t
avl_pop_path(struct avl_path *path, enum avl_side *from)
{
	karn_assert(path->depth <= AVL_MAX_DEPTH);

	if (path->depth) {
		uintptr_t node = path->stack[--path->depth];

		*from = (enum avl_side)(node & (uintptr_t)1);

		return node & ~((uintptr_t)1);
	}

	return 0;
}

static const struct avl_node *
avl_pop_path_node(struct avl_path *path, enum avl_side *from)
{
	return (struct avl_node *)avl_pop_path(path, from);
}

static struct avl_node **
avl_pop_path_slot(struct avl_path *path, enum avl_side *from)
{
	return (struct avl_node **)avl_pop_path(path, from);
}

/******************************************************************************
 * Tree node rotations
 ******************************************************************************/

static struct avl_node *
avl_single_rotate(struct avl_node *node,
                  struct avl_node *child,
                  enum avl_side    side)
{
	karn_assert(node);
	karn_assert(child);
	karn_assert(node->children[!side] == child);

	node->children[!side] = child->children[side];
	child->children[side] = node;

	/* Return new top-level node. */
	return child;
}

static struct avl_node *
avl_double_rotate(struct avl_node *node,
                  struct avl_node *child,
                  enum avl_side    side)
{
	karn_assert(node);
	karn_assert(node->balance == ((side == AVL_RIGHT_SIDE) ? -2 : 2));
	karn_assert(node->children[!side] == child);
	karn_assert(child);
	karn_assert(child->balance == ((side == AVL_RIGHT_SIDE) ? 1 : -1));
	karn_assert(child->children[side]);

	struct avl_node  *grand = child->children[side];
	struct avl_node **siblings = grand->children;

	/* avl_single_rotate(child, grand, !side); */
	child->children[side] = siblings[!side];
	siblings[!side] = child;

	/* avl_single_rotate(node, grand, side); */
	node->children[!side] = siblings[side];
	siblings[side] = node;

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

	/* Finally, return new top-level node. */
	return grand;
}

/******************************************************************************
 * Tree node deletion
 ******************************************************************************/

static struct avl_node **
avl_find_delete_slot(const struct avl_tree *tree,
                     const void            *key,
                     struct avl_path       *path)
{
	const struct avl_node **slot = (const struct avl_node **)&tree->root;
	const struct avl_node  *node = *slot;

	path->depth = 0;

	while (node) {
		int result = tree->compare(node, key, tree->data);

		if (!result)
			return (struct avl_node **)slot;

		avl_push_path_slot(path, slot, result < 0);

		slot = (const struct avl_node **)&node->children[result < 0];
		node = *slot;
	}

	return NULL;
}

static void
avl_delete_slot_node(struct avl_node **slot, struct avl_path *path)
{
	karn_assert(*slot);

	struct avl_node *node = *slot;
	struct avl_node *tmp = node->children[AVL_RIGHT_SIDE];

	if (!tmp) {
		/*
		 * "node" has no right child: simply replace it with "node"'s
		 * left child.
		 */
		*slot = node->children[AVL_LEFT_SIDE];
	}
	else if (!tmp->children[AVL_LEFT_SIDE]) {
		/*
		 * "node"'s right child, "tmp", has no left child:
		 * replace "node" by "tmp", attaching "node"’s left
		 * child to "tmp".
		 */
		tmp->children[AVL_LEFT_SIDE] = node->children[AVL_LEFT_SIDE];

		/*
		 * In addition, "tmp" acquires "node"'s balance factor
		 * and ...
		 */
		tmp->balance = node->balance;

		/*
		 * ... "tmp" must be added to the stack of nodes above
		 * the deleted node "node".
		 */
		*slot = tmp;
		avl_push_path_slot(path,
		                   (const struct avl_node **)slot,
		                   AVL_RIGHT_SIDE);
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
		unsigned int      id;
		struct avl_node **next = &node->children[AVL_RIGHT_SIDE];

		karn_assert(tmp);
		karn_assert(tmp->children[AVL_LEFT_SIDE]);

		id = avl_reserve_path(path);

		/*
		 * Find the inorder successor of "node", "tmp", i.e. the
		 * smallest valued node located into the right subtree of
		 * "node".
		 */
		do {
			avl_push_path_slot(path,
			                   (const struct avl_node **)next,
			                   AVL_LEFT_SIDE);
			next = &tmp->children[AVL_LEFT_SIDE];
			tmp = *next;
		} while (tmp->children[AVL_LEFT_SIDE]);

		*next = tmp->children[AVL_RIGHT_SIDE];
		*tmp = *node;
		
		*slot = tmp;
		avl_fill_path_slot(path, id, slot, AVL_RIGHT_SIDE);
		avl_fill_path_slot(path,
		                   id + 1,
		                   &tmp->children[AVL_RIGHT_SIDE],
		                   AVL_LEFT_SIDE);
	}
}

static void
avl_bottomup_rebalance(struct avl_path *path)
{
	while (true) {
		struct avl_node **slot;
		enum avl_side     from;
		struct avl_node  *node;
		char              adjust;

		slot = avl_pop_path_slot(path, &from);
		if (!slot)
			return;

		node = *slot;
		karn_assert(node->balance >= -1);
		karn_assert(node->balance <= 1);

		adjust = (from == AVL_LEFT_SIDE) ? 1 : -1;
		node->balance += adjust;

		if (uabs(node->balance) == 2) {
			struct avl_node *child = node->children[!from];

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

				node = avl_single_rotate(node, child, from);
			}
			else
				node = avl_double_rotate(node, child, from);

			*slot = node;
		}

		if (node->balance)
			return;
	}
}

struct avl_node *
avl_delete_node(struct avl_tree *tree, const void *key)
{
	avl_assert(tree);

	struct avl_path   path;
	struct avl_node **slot;

	slot = avl_find_delete_slot(tree, key, &path);
	if (slot) {
		karn_assert(*slot);

		struct avl_node *node = *slot;

		avl_delete_slot_node(slot, &path);
		avl_bottomup_rebalance(&path);

		tree->count--;

		return node;
	}

	return NULL;
}

/******************************************************************************
 * Tree node insertion
 ******************************************************************************/

struct avl_node *
avl_scan_node(const struct avl_tree *tree,
              const void            *key,
              struct avl_scan       *scan)
{
	avl_assert(tree);
	karn_assert(scan);

	struct avl_node * const *slot = &tree->root;
	struct avl_node         *node = *slot;

	scan->height = 0;
	scan->top_slot = (struct avl_node **)slot;
	scan->children = 0;
	scan->found_slot = NULL;

	while (node) {
		int result = tree->compare(node, key, tree->data);

		if (!result)
			break;

		if (node->balance) {
			scan->height = 0;
			scan->top_slot = (struct avl_node **)slot;
			scan->children = 0;
		}

		scan->children |= (unsigned long)(result < 0) << scan->height;
		scan->height++;
		karn_assert(scan->height <=
		            (sizeof(scan->children) * CHAR_BIT));

		slot = &node->children[result < 0];
		node = *slot;
	}

	scan->found_slot = (struct avl_node **)slot;

	return node;
}

static inline enum avl_side
avl_shallow_node_child(const struct avl_node *node)
{
	return (node->balance > 0) ? AVL_LEFT_SIDE : AVL_RIGHT_SIDE;
}

static inline void
avl_topdown_rebalance(const struct avl_scan *scan)
{
	karn_assert(scan->height <= (sizeof(scan->children) * CHAR_BIT));
	karn_assert(!scan->height || (scan->top_slot && *(scan->top_slot)));

	unsigned long    h = 0;
	struct avl_node *node = *(scan->top_slot);

	do {
		karn_assert(node);

		unsigned int child = !!(scan->children & (1UL << h));

		node->balance += (child ? 1 : -1);

		node = node->children[child];
		h++;
	} while (h < scan->height);

	node = *(scan->top_slot);
	if (uabs(node->balance) == 2) {
		enum avl_side    side = avl_shallow_node_child(node);
		struct avl_node *child = node->children[!side];

		if (avl_shallow_node_child(child) == side) {
			karn_assert(node->balance ==
			            ((side == AVL_LEFT_SIDE) ? 2 : -2));
			karn_assert(child->balance ==
			            ((side == AVL_LEFT_SIDE) ? 1 : -1));

			child->balance = 0;
			node->balance = 0;
			node = avl_single_rotate(node, child, side);
		}
		else
			node = avl_double_rotate(node, child, side);
	}
	else
		/* No rebalancing required. */
		return;

	*(scan->top_slot) = node;
}

void
avl_post_scan_append_node(struct avl_tree       *tree,
                          struct avl_node       *node,
                          const struct avl_scan *scan)
{
	avl_assert(tree);
	karn_assert(!avl_tree_full(tree));
	karn_assert(node);
	karn_assert(scan);
	karn_assert(scan->top_slot);
	karn_assert(scan->found_slot);
	karn_assert(!*scan->found_slot);

	/* Setup new node content. */
	node->children[AVL_LEFT_SIDE] = NULL;
	node->children[AVL_RIGHT_SIDE] = NULL;
	node->balance = 0;

	/* Make parent point to new node. */
	*(scan->found_slot) = node;

	/* Rebalance to restore AVL tree depth property. */
	if (scan->height)
		avl_topdown_rebalance(scan);

	tree->count++;
}

void
avl_post_scan_replace_node(struct avl_node *node, const struct avl_scan *scan)
{
	karn_assert(node);
	karn_assert(scan);
	karn_assert(scan->top_slot);
	karn_assert(scan->found_slot);
	karn_assert(*scan->found_slot);

	/* Init new node by using the old node content. */
	*node = **(scan->found_slot);

	/*
	 * As we are replacing old node by the new one, AVL tree depth property
	 * has not been altered.
	 * Simply make old node parent point to new node.
	 */
	*(scan->found_slot) = node;
}

int
avl_append_node(struct avl_tree *tree, struct avl_node *node, const void *key)
{
	struct avl_scan scan;

	if (avl_scan_node(tree, key, &scan))
		return -EEXIST;

	avl_post_scan_append_node(tree, node, &scan);

	return 0;
}

struct avl_node *
avl_replace_node(struct avl_tree *tree, struct avl_node *node, const void *key)
{
	struct avl_scan  scan;
	struct avl_node *old;

	old = avl_scan_node(tree, key, &scan);
	if (!old)
		return NULL;

	avl_post_scan_replace_node(node, &scan);

	return old;
}

struct avl_node *
avl_insert_node(struct avl_tree *tree, struct avl_node *node, const void *key)
{
	struct avl_scan  scan;
	struct avl_node *old;

	old = avl_scan_node(tree, key, &scan);
	if (old)
		avl_post_scan_replace_node(node, &scan);
	else
		avl_post_scan_append_node(tree, node, &scan);

	return old;
}

void
avl_clear_tree(struct avl_tree *tree)
{
	avl_assert(tree);

	if (tree->release) {
		struct avl_node *node = tree->root;

		while (node) {
			struct avl_node *left = node->children[AVL_LEFT_SIDE];

			if (!left) {
				left = node->children[AVL_RIGHT_SIDE];
				tree->release(node, tree->data);
			}
			else {
				node->children[AVL_LEFT_SIDE] =
					left->children[AVL_RIGHT_SIDE];
				left->children[AVL_RIGHT_SIDE] = node;
			}

			node = left;
		}
	}

	tree->count = 0;
	tree->root = NULL;
}

struct avl_node *
avl_find_node(const struct avl_tree *tree, const void *key)
{
	avl_assert(tree);

	struct avl_node *node = tree->root;

	while (node) {
		int result = tree->compare(node, key, tree->data);

		if (!result)
			return node;

		node = node->children[result < 0];
	}

	return NULL;
}

void
avl_init_tree(struct avl_tree         *tree,
              avl_compare_node_key_fn *compare,
              avl_release_node_fn     *release,
              void                    *data)
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
avl_fini_tree(struct avl_tree *tree)
{
	avl_clear_tree(tree);
}

/******************************************************************************
 * Tree iterator / traversal
 ******************************************************************************/

static const struct avl_node *
avl_iter_down(struct avl_path       *path,
              const struct avl_node *node,
              enum avl_side          side)
{
	while (node->children[side]) {
		avl_push_path_node(path, node, side);
		node = node->children[side];
	}

	return node;
}

static const struct avl_node *
avl_iter_up(struct avl_path       *path,
            const struct avl_node *node,
            enum avl_side          side)
{
	enum avl_side from;

	do {
		node = avl_pop_path_node(path, &from);
	} while (node && (from == side));

	return node;
}

static const struct avl_node *
avl_step_iter(struct avl_iter       *iter,
              const struct avl_node *node,
              enum avl_side          order)
{
	karn_assert(iter);
	karn_assert(iter->path.depth < AVL_MAX_DEPTH);

	struct avl_path *path = &iter->path;

	if (node->children[!order]) {
		avl_push_path_node(path, node, !order);

		return avl_iter_down(path, node->children[!order], order);
	}

	return avl_iter_up(path, node, !order);
}

static struct avl_node *
avl_begin_iter(struct avl_iter       *iter,
               const struct avl_tree *tree,
               enum avl_side          order)
{
	karn_assert(iter);
	avl_assert(tree);

	const struct avl_node *node = tree->root;
	struct avl_path       *path = &iter->path;

	if (!node)
		return NULL;

	path->depth = 0;

	return (struct avl_node *)avl_iter_down(path, node, order);
}

struct avl_node *
avl_iter_find(struct avl_iter       *iter,
              const struct avl_tree *tree,
              const void            *key)
{
	karn_assert(iter);
	avl_assert(tree);

	const struct avl_node *node = tree->root;
	struct avl_path       *path = &iter->path;

	path->depth = 0;

	while (node) {
		int result = tree->compare(node, key, tree->data);

		if (!result)
			return (struct avl_node *)node;

		avl_push_path_node(path, node, result < 0);
		node = node->children[result < 0];
	}

	return NULL;
}

struct avl_node *
avl_iter_first(struct avl_iter *iter, const struct avl_tree *tree)
{
	return avl_begin_iter(iter, tree, AVL_LEFT_SIDE);
}

struct avl_node *
avl_iter_next(struct avl_iter *iter, const struct avl_node *node)
{
	return (struct avl_node *)avl_step_iter(iter, node, AVL_LEFT_SIDE);
}

struct avl_node *
avl_iter_last(struct avl_iter *iter, const struct avl_tree *tree)
{
	return avl_begin_iter(iter, tree, AVL_RIGHT_SIDE);
}

struct avl_node *
avl_iter_prev(struct avl_iter *iter, const struct avl_node *node)
{
	return (struct avl_node *)avl_step_iter(iter, node, AVL_RIGHT_SIDE);
}

/******************************************************************************
 * AVL tree printer
 ******************************************************************************/

#if defined(CONFIG_KARN_AVL_TEST)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * Height of an AVL tree containing n nodes cannot exceed:
 *   1.44 * log2(n)
 *
 * Minimum number of nodes in an AVL tree with height h is:
 *   N(h) = N(h-1) + N(h-2) + 1 for n>2 where N(0) = 1 and N(1) = 2
 */
static unsigned long
avl_min_count(unsigned int depth)
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
 * An AVL tree cannot hold more than 2^^depth - 1 nodes when perfectly balanced.
 */
static unsigned long
avl_max_count(unsigned int depth)
{
	return (1UL << depth) - 1;
}

#define AVL_PRINT_PREFIX_SIZE (256U)

struct avl_printer {
	char                 prefix[AVL_PRINT_PREFIX_SIZE];
	int                  colno;
	unsigned int         depth;
	unsigned int         max_depth;
	avl_display_node_fn *display;
};

static void
avl_print_subtree(const struct avl_node *node, struct avl_printer *printer);

static void
avl_print_subtree_node(const struct avl_node *node,
                       const struct avl_node *sibling,
                       const char            *prefix,
                       size_t                 size,
                       struct avl_printer    *printer)
{
	if (node) {
		printf("%s +-", printer->prefix);

		if ((size_t)printer->colno < (sizeof(printer->prefix) - size)) {
			memcpy(&printer->prefix[printer->colno], prefix, size);
			printer->colno += size - 1;

			printer->depth++;
			avl_print_subtree(node, printer);
			printer->depth--;

			printer->colno -= size - 1;
			printer->prefix[printer->colno] = '\0';
		}
	}
	else if (sibling)
		printf("%s +-{null}\n", printer->prefix);
}

static void
avl_print_subtree(const struct avl_node *node, struct avl_printer *printer)
{
	if (printer->depth < printer->max_depth) {
		const struct avl_node *left = node->children[AVL_LEFT_SIDE];
		const struct avl_node *right = node->children[AVL_RIGHT_SIDE];

		printer->display(node);
		putchar('\n');

		avl_print_subtree_node(right,
		                       left,
		                       " |  ",
		                       sizeof(" |  "),
		                       printer);
		avl_print_subtree_node(left,
		                       right,
		                       "    ",
		                       sizeof("    "),
		                       printer);
	}
}

int
avl_print_tree(const struct avl_tree *tree,
               unsigned int           max_depth,
               avl_display_node_fn   *display)
{
	avl_assert(tree);
	karn_assert(max_depth);

	if (tree->root) {
		struct avl_printer *printer;

		printer = malloc(sizeof(*printer));
		if (!printer)
			return -ENOMEM;

		printer->prefix[0] = '\0';
		printer->colno = 0;
		printer->depth = 0;
		printer->max_depth = max_depth;
		printer->display = display;

		avl_print_subtree(tree->root, printer);

		free(printer);
	}

	return 0;
}

static bool
avl_recurs_check_subtree(const struct avl_node *node,
                         int                   *height,
                         avl_compare_nodes_fn  *compare)
{
	const struct avl_node *left;
	int                    left_height;
	const struct avl_node *right;
	int                    right_height;
	int                    balance;

	if (!node) {
		*height = 0;
		return true;
	}

	left = node->children[AVL_LEFT_SIDE];
	if (left) {
		if (compare(left, node) >= 0) {
			fprintf(stderr,
			        "karn: avl: wrong tree node ordering: "
			        "left >= parent\n");
			return false;
		}
	}

	right = node->children[AVL_RIGHT_SIDE];
	if (right) {
		if (compare(right, node) <= 0) {
			fprintf(stderr,
			        "karn: avl: wrong tree node ordering: "
			        "right <= parent\n");
			return false;
		}
	}

	if (!avl_recurs_check_subtree(left, &left_height, compare))
		return false;
	if (!avl_recurs_check_subtree(right, &right_height, compare))
		return false;

	balance = right_height - left_height;
	if ((balance < -1) || (balance > 1)) {
		fprintf(stderr, "karn: avl: invalid node balance factor\n");
		return false;
	}
	if (balance != (int)node->balance) {
		fprintf(stderr, "karn: avl: unexpected node balance factor\n");
		return false;
	}

	*height = 1 + ((left_height > right_height) ?
	               left_height : right_height);

	return true;
}

bool
avl_check_tree(const struct avl_tree *tree,
               unsigned long          count,
               avl_compare_nodes_fn  *compare)
{
	int height = -1;

	if (avl_tree_count(tree) != count) {
		fprintf(stderr,
		        "karn: avl: unexpected tree node count: "
		        "%lu != %lu\n",
		        avl_tree_count(tree),
		        count);
		return false;
	}

	if (!count) {
		if (tree->root) {
			fprintf(stderr,
			        "karn: avl: invalid empty tree: "
			        "root node not NULL\n");
			return false;
		}

		return true;
	}

	if (!avl_recurs_check_subtree(tree->root, &height, compare))
		return false;

	if (count > avl_max_count(height)) {
		/*
		 * A true AVL tree cannot hold so many nodes with such a
		 * small height.
		 */
		fprintf(stderr,
		        "karn: avl: unexpectedly small tree height: "
		        "#nodes=%lu height=%d\n",
		        count,
		        height);
		return false;
	}
	if (count < avl_min_count(height)) {
		/*
		 * A true AVL tree shall not be so deep to hold the given number
		 * of nodes.
		 */
		fprintf(stderr,
		        "karn: avl: unexpectedly large tree height: "
		        "#nodes=%lu height=%d %lu\n",
		        count,
		        height,
		        avl_min_count(height));
		return false;
	}

	return true;
}

#endif /* defined(CONFIG_KARN_AVL_TEST) */
