#include <karn/avl.h>
#include <criterion/criterion.h>
#include <stdio.h>
#include <errno.h>

struct avlut_node {
	struct avl_node avl;
	unsigned int    value;
};

static struct avlut_node avlut_zero = {
	.avl   = {
		.children = {
			[AVL_LEFT_SIDE]  = NULL,
			[AVL_RIGHT_SIDE] = NULL
		},
		.balance  = 0
	},
	.value = 0
};

static struct avlut_node avlut_two = {
	.avl   = {
		.children = {
			[AVL_LEFT_SIDE]  = NULL,
			[AVL_RIGHT_SIDE] = NULL
		},
		.balance  = 0
	},
	.value = 2
};

static struct avlut_node avlut_one = {
	.avl   = {
		.children = {
			[AVL_LEFT_SIDE]  = &avlut_zero.avl,
			[AVL_RIGHT_SIDE] = &avlut_two.avl
		},
		.balance  = 0
	},
	.value = 1
};

static struct avlut_node avlut_four = {
	.avl   = {
		.children = {
			[AVL_LEFT_SIDE]  = NULL,
			[AVL_RIGHT_SIDE] = NULL
		},
		.balance  = 0
	},
	.value = 4
};

static struct avlut_node avlut_five = {
	.avl   = {
		.children = {
			[AVL_LEFT_SIDE]  = &avlut_four.avl,
			[AVL_RIGHT_SIDE] = NULL
		},
		.balance  = -1
	},
	.value = 5
};

static struct avlut_node avlut_three = {
	.avl   = {
		.children = {
			[AVL_LEFT_SIDE]  = &avlut_one.avl,
			[AVL_RIGHT_SIDE] = &avlut_five.avl
		},
		.balance  = 0
	},
	.value = 3
};

static struct avlut_node avlut_six = { .value = 6 };
static struct avlut_node avlut_seven = { .value = 7 };
static struct avlut_node avlut_eight = { .value = 8 };
static struct avlut_node avlut_nine = { .value = 9 };
static struct avlut_node avlut_ten = { .value = 10 };
static struct avlut_node avlut_eleven = { .value = 11 };
static struct avlut_node avlut_twelve = { .value = 12 };
static struct avlut_node avlut_thirteen = { .value = 13 };

static int
avlut_compare(const struct avl_node *node,
              const void            *key,
              const void            *data __unused)
{
	unsigned int k = (unsigned long)key;

	return ((struct avlut_node *)node)->value - k;
}

static int
avlut_compare_nodes(const struct avl_node *first,
                    const struct avl_node *second)
{
	return ((struct avlut_node *)first)->value -
	       ((struct avlut_node *)second)->value;
}

#if 0
static void
avlut_print_node(const struct avl_node *node)
{
	const struct avlut_node *key = (struct avlut_node *)node;

	printf("%u(%d)", key->value, (int)node->balance);
}

static void
avlut_print_tree(const struct avl_tree *tree)
{
	avl_print_tree(tree, 16U, avlut_print_node);
}
#endif

Test(avlut_iter, forward_empty)
{
	struct avl_tree  tree;
	struct avl_iter  iter;
	struct avl_node *node;
	unsigned int     count = 0;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);

	cr_expect_eq(avl_tree_count(&tree),
	             0,
	             "unexpected empty tree count: %lu != 0\n",
	             avl_tree_count(&tree));

	avl_walk_forward(&iter, &tree, node)
		count++;

	cr_expect_eq(count,
	             0,
	             "unexpected empty tree iteration count: %u != 0\n",
	             count);
}

Test(avlut_iter, backward_empty)
{
	struct avl_tree  tree;
	struct avl_iter  iter;
	struct avl_node *node;
	unsigned int     count = 0;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);

	cr_expect_eq(avl_tree_count(&tree),
	             0,
	             "unexpected empty tree count: %lu != 0\n",
	             avl_tree_count(&tree));

	avl_walk_backward(&iter, &tree, node)
		count++;

	cr_expect_eq(count,
	             0,
	             "unexpected empty tree iteration count: %u != 0\n",
	             count);
}

Test(avlut_iter, forward_single)
{
	struct avl_tree  tree;
	struct avl_iter  iter;
	struct avl_node *node;
	unsigned int     count = 0;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);
	tree.root = &avlut_zero.avl;
	tree.count = 1;

	cr_expect_eq(avl_tree_count(&tree),
	             1,
	             "unexpected single tree count: %lu != 1\n",
	             avl_tree_count(&tree));

	avl_walk_forward(&iter, &tree, node) {
		cr_expect_eq(((struct avlut_node *)node)->value,
		             count,
		             "unexpected single tree value: %u != 0\n",
		             count);
		count++;
	}

	cr_expect_eq(count,
	             1,
	             "unexpected single tree iteration count: %u != 1\n",
	             count);
}

Test(avlut_iter, backward_single)
{
	struct avl_tree  tree;
	struct avl_iter  iter;
	struct avl_node *node;
	unsigned int     count = 0;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);
	tree.root = &avlut_zero.avl;
	tree.count = 1;

	cr_expect_eq(avl_tree_count(&tree),
	             1,
	             "unexpected single tree count: %lu != 1\n",
	             avl_tree_count(&tree));

	avl_walk_backward(&iter, &tree, node) {
		cr_expect_eq(((struct avlut_node *)node)->value,
		             count,
		             "unexpected single tree value: %u != 0\n",
		             count);
		count++;
	}

	cr_expect_eq(count,
	             1,
	             "unexpected single tree iteration count: %u != 1\n",
	             count);
}

Test(avlut_iter, forward_fixed)
{
	struct avl_tree  tree;
	struct avl_iter  iter;
	struct avl_node *node;
	unsigned int     count = 0;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);
	tree.root = &avlut_three.avl;
	tree.count = 6;

	cr_expect_eq(avl_tree_count(&tree),
	             6,
	             "unexpected fixed tree count: %lu != 6\n",
	             avl_tree_count(&tree));

	avl_walk_forward(&iter, &tree, node) {
		cr_expect_eq(((struct avlut_node *)node)->value,
		             count,
		             "unexpected fixed tree value: %u != %u\n",
		             ((struct avlut_node *)node)->value,
		             count);
		count++;
	}

	cr_expect_eq(count,
	             6,
	             "unexpected fixed tree iteration count: %u != 6\n",
	             count);
}

Test(avlut_iter, backward_fixed)
{
	struct avl_tree  tree;
	struct avl_iter  iter;
	struct avl_node *node;
	unsigned int     count = 0;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);
	tree.root = &avlut_three.avl;
	tree.count = 6;

	cr_expect_eq(avl_tree_count(&tree),
	             6,
	             "unexpected fixed tree count: %lu != 6\n",
	             avl_tree_count(&tree));

	avl_walk_backward(&iter, &tree, node) {
		cr_expect_eq(((struct avlut_node *)node)->value,
		             5 - count,
		             "unexpected fixed tree value: %u != %u\n",
		             ((struct avlut_node *)node)->value,
		             5 - count);
		count++;
	}

	cr_expect_eq(count,
	             6,
	             "unexpected fixed tree iteration count: %u != 6\n",
	             count);
}

static void
avlut_find_and_iter_forward(struct avl_iter       *iter,
                            const struct avl_tree *tree,
                            unsigned int           key)
{
	struct avl_node *node;
	unsigned int     count = 0;

	avl_find_and_walk_forward(iter,
	                          tree,
	                          node,
	                          (void *)((unsigned long)key)) {
		cr_expect_eq(((struct avlut_node *)node)->value,
		             count + key,
		             "unexpected fixed tree value: %u != %u\n",
		             ((struct avlut_node *)node)->value,
		             count + key);
		count++;
	}

	cr_expect_eq(count,
	             6 - key,
	             "unexpected fixed tree iteration count: %u != %u\n",
	             count,
	             6 - key);
}

Test(avlut_iter, find_forward_fixed)
{
	struct avl_tree tree;
	struct avl_iter iter;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);
	tree.root = &avlut_three.avl;
	tree.count = 6;

	cr_expect_eq(avl_tree_count(&tree),
	             6,
	             "unexpected fixed tree count: %lu != 6\n",
	             avl_tree_count(&tree));

	avlut_find_and_iter_forward(&iter, &tree, 0);
	avlut_find_and_iter_forward(&iter, &tree, 1);
	avlut_find_and_iter_forward(&iter, &tree, 2);
	avlut_find_and_iter_forward(&iter, &tree, 3);
	avlut_find_and_iter_forward(&iter, &tree, 4);
	avlut_find_and_iter_forward(&iter, &tree, 5);
}

static void
avlut_find_and_iter_backward(struct avl_iter       *iter,
                             const struct avl_tree *tree,
                             unsigned int           key)
{
	struct avl_node *node;
	unsigned int     count = 0;

	avl_find_and_walk_backward(iter,
	                           tree,
	                           node,
	                           (void *)((unsigned long)key)) {
		cr_expect_eq(((struct avlut_node *)node)->value,
		             key - count,
		             "unexpected fixed tree value: %u != %u\n",
		             ((struct avlut_node *)node)->value,
		             key - count);
		count++;
	}

	cr_expect_eq(count,
	             key + 1,
	             "unexpected fixed tree iteration count: %u != %u\n",
	             count,
	             key + 1);
}

Test(avlut_iter, find_backward_fixed)
{
	struct avl_tree tree;
	struct avl_iter iter;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);
	tree.root = &avlut_three.avl;
	tree.count = 6;

	cr_expect_eq(avl_tree_count(&tree),
	             6,
	             "unexpected fixed tree count: %lu != 6\n",
	             avl_tree_count(&tree));

	avlut_find_and_iter_backward(&iter, &tree, 0);
	avlut_find_and_iter_backward(&iter, &tree, 1);
	avlut_find_and_iter_backward(&iter, &tree, 2);
	avlut_find_and_iter_backward(&iter, &tree, 3);
	avlut_find_and_iter_backward(&iter, &tree, 4);
	avlut_find_and_iter_backward(&iter, &tree, 5);
}

Test(avlut_search, search_empty)
{
	struct avl_tree  tree;
	struct avl_node *node;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);

	cr_expect_eq(avl_tree_count(&tree),
	             0,
	             "unexpected empty tree count: %lu != 0\n",
	             avl_tree_count(&tree));

	node = avl_find_node(&tree, 0);

	cr_expect_null(node, "unexpected empty tree node found\n");
}

Test(avlut_search, search_single)
{
	struct avl_tree  tree;
	struct avl_node *node;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);
	tree.root = &avlut_zero.avl;
	tree.count = 1;

	cr_expect_eq(avl_tree_count(&tree),
	             1,
	             "unexpected empty tree count: %lu != 1\n",
	             avl_tree_count(&tree));

	node = avl_find_node(&tree, (void *)2);
	cr_expect_null(node, "unexpected single tree node found\n");

	node = avl_find_node(&tree, (void *)0);
	cr_assert_not_null(node, "single tree node not found\n");
	cr_expect_eq(node,
	             &avlut_zero.avl,
	             "wrong single tree node found: %u != 0\n",
	             ((struct avlut_node *)node)->value);
}

static void
avlut_find_present_node(const struct avl_tree   *tree,
                        const struct avlut_node *node)
{
	const struct avl_node *found;

	found = avl_find_node(tree, (void *)((unsigned long)node->value));
	cr_assert_not_null(found, "tree node not found\n");
	cr_expect_eq(found,
	             &node->avl,
	             "wrong tree node found: %u != %u\n",
	             ((struct avlut_node *)found)->value,
	             node->value);
}

static void
avlut_find_absent_key(const struct avl_tree *tree, unsigned int key)

{
	const struct avl_node *found;

	found = avl_find_node(tree, (void *)((unsigned long)key));
	cr_expect_null(found, "unexpected tree node found\n");
}

Test(avlut_search, search_fixed)
{
	struct avl_tree tree;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);
	tree.root = &avlut_three.avl;
	tree.count = 6;

	cr_expect_eq(avl_tree_count(&tree),
	             6,
	             "unexpected fixed tree count: %lu != 1\n",
	             avl_tree_count(&tree));

	avlut_find_present_node(&tree, &avlut_zero);
	avlut_find_present_node(&tree, &avlut_one);
	avlut_find_present_node(&tree, &avlut_two);
	avlut_find_present_node(&tree, &avlut_three);
	avlut_find_present_node(&tree, &avlut_four);
	avlut_find_present_node(&tree, &avlut_five);
	avlut_find_absent_key(&tree, -1);
	avlut_find_absent_key(&tree, 6);
}

Test(avlut_append, single_append)
{
	struct avl_tree  tree;
	struct avl_iter  iter;
	struct avl_node *node;
	unsigned int     count = 0;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);

	avl_append_node(&tree, &avlut_three.avl,
	                (void *)((unsigned long)avlut_three.value));

	cr_expect_eq(avl_tree_count(&tree),
	             1,
	             "unexpected tree count: %lu != 1\n",
	             avl_tree_count(&tree));

	avlut_find_present_node(&tree, &avlut_three);
	avlut_find_absent_key(&tree, 6);

	avl_walk_forward(&iter, &tree, node)
		count++;

	cr_expect_eq(count,
	             avl_tree_count(&tree),
	             "unexpected tree iteration count: %u != %lu\n",
	             count,
	             avl_tree_count(&tree));
}

static bool
avlut_check_tree(const struct avl_tree *tree, unsigned int count)
{
	return avl_check_tree(tree, count, avlut_compare_nodes);
}

static void
avlut_append(struct avl_tree   *tree,
             struct avlut_node *node,
             unsigned int       count)
{
	int              ret;
	struct avl_iter  iter;
	unsigned int     cnt = 0;
	struct avl_node *curr = NULL;
	struct avl_node  dup;

	assert(count);

	ret = avl_append_node(tree,
	                      &node->avl,
	                      (void *)((unsigned long)node->value));

	cr_assert_eq(ret, 0, "append failed: %d\n", ret);

	cr_expect_eq(avl_tree_count(tree),
	             count,
	             "unexpected tree count: %lu != %u\n",
	             avl_tree_count(tree),
	             count);

	avlut_find_present_node(tree, node);
	avlut_find_absent_key(tree, 27);

	avl_walk_forward(&iter, tree, curr)
		cnt++;
	cr_expect_eq(cnt,
	             count,
	             "unexpected tree iteration count: %u != %u\n",
	             cnt,
	             count);

	ret = avl_append_node(tree,
	                      &dup,
	                      (void *)((unsigned long)node->value));
	cr_assert_eq(ret, -EEXIST, "append duplicate returned: %d\n", ret);

	cr_expect(avlut_check_tree(tree, count), "tree property violation\n");
}

Test(avlut_append, double_append_right)
{
	struct avl_tree tree;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);

	avlut_append(&tree, &avlut_three, 1);
	avlut_append(&tree, &avlut_five, 2);
}

Test(avlut_append, double_append_left)
{
	struct avl_tree tree;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);

	avlut_append(&tree, &avlut_three, 1);
	avlut_append(&tree, &avlut_zero, 2);
}

Test(avlut_append, triple_append_with_right_rebalance)
{
	struct avl_tree tree;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);

	avlut_append(&tree, &avlut_three, 1);
	avlut_append(&tree, &avlut_one, 2);
	avlut_append(&tree, &avlut_zero, 3);
}

Test(avlut_append, triple_append_with_left_rebalance)
{
	struct avl_tree tree;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);

	avlut_append(&tree, &avlut_zero, 1);
	avlut_append(&tree, &avlut_one, 2);
	avlut_append(&tree, &avlut_two, 3);
}

Test(avlut_append, triple_append_with_left_right_rebalance)
{
	struct avl_tree tree;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);

	avlut_append(&tree, &avlut_three, 1);
	avlut_append(&tree, &avlut_one, 2);
	avlut_append(&tree, &avlut_two, 3);
}

Test(avlut_append, triple_append_with_right_left_rebalance)
{
	struct avl_tree tree;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);

	avlut_append(&tree, &avlut_three, 1);
	avlut_append(&tree, &avlut_five, 2);
	avlut_append(&tree, &avlut_four, 3);
}

Test(avlut_append, append_depth_3)
{
	struct avl_tree tree;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);

	avlut_append(&tree, &avlut_three, 1);
	avlut_append(&tree, &avlut_one, 2);
	avlut_append(&tree, &avlut_two, 3);
	avlut_append(&tree, &avlut_zero, 4);
	avlut_append(&tree, &avlut_five, 5);
	avlut_append(&tree, &avlut_four, 6);
}

Test(avlut_append, append_depth_all)
{
	struct avl_tree tree;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);

	avlut_append(&tree, &avlut_three, 1);
	avlut_append(&tree, &avlut_one, 2);
	avlut_append(&tree, &avlut_two, 3);
	avlut_append(&tree, &avlut_zero, 4);
	avlut_append(&tree, &avlut_five, 5);
	avlut_append(&tree, &avlut_four, 6);
	avlut_append(&tree, &avlut_six, 7);
	avlut_append(&tree, &avlut_seven, 8);
	avlut_append(&tree, &avlut_eight, 9);
	avlut_append(&tree, &avlut_nine, 10);
	avlut_append(&tree, &avlut_ten, 11);
	avlut_append(&tree, &avlut_eleven, 12);
	avlut_append(&tree, &avlut_twelve, 13);
}

Test(avlut_delete, delete_empty)
{
	struct avl_tree tree;
	struct avl_node *node;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);

	cr_expect_eq(avl_tree_count(&tree),
	             0,
	             "unexpected tree count: %lu != 0\n",
	             avl_tree_count(&tree));

	node = avl_delete_node(&tree, 0);

	cr_expect_null(node, "unexpected tree deleted node\n");
}

Test(avlut_delete, delete_single)
{
	struct avl_tree tree;
	struct avl_node *node;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);
	tree.root = &avlut_zero.avl;
	tree.count = 1;

	cr_expect_eq(avl_tree_count(&tree),
	             1,
	             "unexpected tree count: %lu != 1\n",
	             avl_tree_count(&tree));

	node = avl_delete_node(&tree, (void *)27);
	cr_expect_null(node, "deleted absent node\n");

	node = avl_delete_node(&tree, 0);

	cr_assert_not_null(node, "node deletion failed\n");
	cr_expect(node == &avlut_zero.avl,
	          "unexpected deleted node: %p != %p\n",
	          node,
	          &avlut_zero.avl);

	cr_expect_null(tree.root,
	               "invalid post delete tree root: %p\n",
	               tree.root);

	cr_expect_eq(avl_tree_count(&tree),
	             0,
	             "unexpected tree count: %lu != 0\n",
	             avl_tree_count(&tree));
}

Test(avlut_delete, delete_left_leaf_from_double)
{
	struct avl_tree tree;
	struct avl_node *node;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);
	tree.root = &avlut_five.avl;
	tree.count = 2;

	node = avl_delete_node(&tree, (void *)27);
	cr_expect_null(node, "deleted absent node\n");

	node = avl_delete_node(&tree, (void *)4);

	cr_assert_not_null(node, "node deletion failed\n");
	cr_expect(node == &avlut_four.avl, "unexpected deleted node\n");

	cr_assert_not_null(tree.root, "invalid post delete tree root\n");
	cr_expect(tree.root == &avlut_five.avl,
	          "invalid post delete tree root\n");

	cr_expect_null(avlut_five.avl.children[AVL_LEFT_SIDE],
	               "invalid post delete root left child\n");
	cr_expect_null(avlut_five.avl.children[AVL_RIGHT_SIDE],
	               "invalid post delete root right_child\n");

	cr_expect_eq(avl_tree_count(&tree),
	             1,
	             "unexpected tree count: %lu != 1\n",
	             avl_tree_count(&tree));
}

Test(avlut_delete, delete_root_from_double_with_left_leaf)
{
	struct avl_tree tree;
	struct avl_node *node;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);
	tree.root = &avlut_five.avl;
	tree.count = 2;

	node = avl_delete_node(&tree, (void *)27);
	cr_expect_null(node, "deleted absent node\n");

	node = avl_delete_node(&tree, (void *)5);

	cr_assert_not_null(node, "node deletion failed\n");
	cr_expect(node == &avlut_five.avl, "unexpected deleted node\n");

	cr_assert_not_null(tree.root, "invalid post delete tree root\n");
	cr_expect(tree.root == &avlut_four.avl,
	          "invalid post delete tree root\n");

	cr_expect_null(avlut_four.avl.children[AVL_LEFT_SIDE],
	               "invalid post delete root left child\n");
	cr_expect_null(avlut_four.avl.children[AVL_RIGHT_SIDE],
	               "invalid post delete root right_child\n");

	cr_expect_eq(avl_tree_count(&tree),
	             1,
	             "unexpected tree count: %lu != 1\n",
	             avl_tree_count(&tree));
}

Test(avlut_delete, delete_right_leaf_from_double)
{
	struct avl_tree tree;
	struct avl_node *node;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);
	tree.root = &avlut_one.avl;
	avlut_one.avl.children[AVL_LEFT_SIDE] = NULL;
	tree.count = 2;

	node = avl_delete_node(&tree, (void *)27);
	cr_expect_null(node, "deleted absent node\n");

	node = avl_delete_node(&tree, (void *)2);

	cr_assert_not_null(node, "node deletion failed\n");
	cr_expect(node == &avlut_two.avl, "unexpected deleted node\n");

	cr_assert_not_null(tree.root, "invalid post delete tree root\n");
	cr_expect(tree.root == &avlut_one.avl,
	          "invalid post delete tree root\n");

	cr_expect_null(avlut_one.avl.children[AVL_LEFT_SIDE],
	               "invalid post delete root left child\n");
	cr_expect_null(avlut_one.avl.children[AVL_RIGHT_SIDE],
	               "invalid post delete root right_child\n");

	cr_expect_eq(avl_tree_count(&tree),
	             1,
	             "unexpected tree count: %lu != 1\n",
	             avl_tree_count(&tree));
}

Test(avlut_delete, delete_root_from_double_with_right_leaf)
{
	struct avl_tree tree;
	struct avl_node *node;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);
	tree.root = &avlut_one.avl;
	avlut_one.avl.children[AVL_LEFT_SIDE] = NULL;
	tree.count = 2;

	node = avl_delete_node(&tree, (void *)27);
	cr_expect_null(node, "deleted absent node\n");

	node = avl_delete_node(&tree, (void *)1);

	cr_assert_not_null(node, "node deletion failed\n");
	cr_expect(node == &avlut_one.avl, "unexpected deleted node\n");

	cr_assert_not_null(tree.root, "invalid post delete tree root\n");
	cr_expect(tree.root == &avlut_two.avl,
	          "invalid post delete tree root\n");

	cr_expect_null(avlut_two.avl.children[AVL_LEFT_SIDE],
	               "invalid post delete root left child\n");
	cr_expect_null(avlut_two.avl.children[AVL_RIGHT_SIDE],
	               "invalid post delete root right_child\n");

	cr_expect_eq(avl_tree_count(&tree),
	             1,
	             "unexpected tree count: %lu != 1\n",
	             avl_tree_count(&tree));
}

static void
avlut_delete_one(struct avl_tree                *tree,
                 struct avlut_node              *node,
                 const struct avlut_node *const *remain,
                 unsigned int                    count)
{
	struct avl_node *avl;
	struct avl_iter  iter;
	struct avl_node *curr;
	unsigned int     cnt;

	cr_expect_eq(avl_tree_count(tree),
	             count + 1,
	             "unexpected tree count: %lu != %u\n",
	             avl_tree_count(tree),
	             count + 1);

	avl = avl_delete_node(tree, (void *)27);
	cr_expect_null(avl, "deleted absent node\n");

	avl = avl_delete_node(tree, (void *)((unsigned long)node->value));
	cr_assert_not_null(avl, "node deletion failed\n");
	cr_expect(avl == &node->avl, "unexpected deleted node\n");

	cr_expect_eq(avl_tree_count(tree),
	             count,
	             "unexpected post deletion tree count: %lu != %u\n",
	             avl_tree_count(tree),
	             count);

	avlut_find_absent_key(tree, node->value);

	cnt = 0;
	avl_walk_forward(&iter, tree, curr) {
		cr_assert_lt(cnt,
		             count,
		             "unexpected iteration count: %u != %u\n",
		             cnt,
		             count);
		cr_assert_eq(curr,
		             &remain[cnt]->avl,
		             "unexpected node %p != %p (value=%u)\n",
		             curr,
		             &remain[cnt]->avl,
		             remain[cnt]->value);
		cnt++;
	}

	cr_expect(avlut_check_tree(tree, count), "tree property violation\n");
}

Test(avlut_delete, delete_right_leaf)
{
	struct avl_tree                tree;
	const struct avlut_node *const remain[] = {
		&avlut_zero,
		&avlut_two,
		&avlut_three,
	};

	avl_init_tree(&tree, avlut_compare, NULL, NULL);

	avlut_append(&tree, &avlut_two, 1);
	avlut_append(&tree, &avlut_zero, 2);
	avlut_append(&tree, &avlut_three, 3);
	avlut_append(&tree, &avlut_one, 4);

	avlut_delete_one(&tree, &avlut_one, remain, array_nr(remain));
}

Test(avlut_delete, delete_three_from_full)
{
	struct avl_tree                tree;
	const struct avlut_node *const remain[] = {
		&avlut_zero,
		&avlut_one,
		&avlut_two,
		&avlut_four,
		&avlut_five
	};

	avl_init_tree(&tree, avlut_compare, NULL, NULL);
	tree.root = &avlut_three.avl;
	tree.count = 6;

	avlut_delete_one(&tree,
	                 &avlut_three,
	                 remain,
	                 array_nr(remain));
}

Test(avlut_delete, delete_five_from_full)
{
	struct avl_tree                tree;
	const struct avlut_node *const remain[] = {
		&avlut_zero,
		&avlut_one,
		&avlut_two,
		&avlut_three,
		&avlut_four,
	};

	avl_init_tree(&tree, avlut_compare, NULL, NULL);
	tree.root = &avlut_three.avl;
	tree.count = 6;

	avlut_delete_one(&tree,
	                 &avlut_five,
	                 remain,
	                 array_nr(remain));
}

Test(avlut_delete, delete_four_from_full)
{
	struct avl_tree                tree;
	const struct avlut_node *const remain[] = {
		&avlut_zero,
		&avlut_one,
		&avlut_two,
		&avlut_three,
		&avlut_five,
	};

	avl_init_tree(&tree, avlut_compare, NULL, NULL);
	tree.root = &avlut_three.avl;
	tree.count = 6;

	avlut_delete_one(&tree,
	                 &avlut_four,
	                 remain,
	                 array_nr(remain));
}

Test(avlut_delete, delete_one_from_full)
{
	struct avl_tree                tree;
	const struct avlut_node *const remain[] = {
		&avlut_zero,
		&avlut_two,
		&avlut_three,
		&avlut_four,
		&avlut_five,
	};

	avl_init_tree(&tree, avlut_compare, NULL, NULL);
	tree.root = &avlut_three.avl;
	tree.count = 6;

	avlut_delete_one(&tree,
	                 &avlut_one,
	                 remain,
	                 array_nr(remain));
}

Test(avlut_delete, delete_two_from_full)
{
	struct avl_tree                tree;
	const struct avlut_node *const remain[] = {
		&avlut_zero,
		&avlut_one,
		&avlut_three,
		&avlut_four,
		&avlut_five,
	};

	avl_init_tree(&tree, avlut_compare, NULL, NULL);
	tree.root = &avlut_three.avl;
	tree.count = 6;

	avlut_delete_one(&tree,
	                 &avlut_two,
	                 remain,
	                 array_nr(remain));
}

Test(avlut_delete, delete_sequence_from_full)
{
	struct avl_tree          tree;
	const struct avlut_node *remain0[] = {
		&avlut_zero,
		&avlut_one,
		&avlut_two,
		&avlut_three,
		&avlut_four
	};
	const struct avlut_node *remain1[] = {
		&avlut_zero,
		&avlut_one,
		&avlut_two,
		&avlut_three,
	};
	const struct avlut_node *remain2[] = {
		&avlut_zero,
		&avlut_two,
		&avlut_three,
	};
	const struct avlut_node *remain3[] = {
		&avlut_zero,
		&avlut_two,
	};

	avl_init_tree(&tree, avlut_compare, NULL, NULL);
	tree.root = &avlut_three.avl;
	tree.count = 6;

	avlut_delete_one(&tree, &avlut_five, remain0, array_nr(remain0));
	avlut_delete_one(&tree, &avlut_four, remain1, array_nr(remain1));
	avlut_delete_one(&tree, &avlut_one, remain2, array_nr(remain2));
	avlut_delete_one(&tree, &avlut_three, remain3, array_nr(remain3));
}

Test(avlut_delete, delete_all)
{
	struct avl_tree tree;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);

	avlut_append(&tree, &avlut_three, 1);
	avlut_append(&tree, &avlut_one, 2);
	avlut_append(&tree, &avlut_two, 3);
	avlut_append(&tree, &avlut_zero, 4);
	avlut_append(&tree, &avlut_five, 5);
	avlut_append(&tree, &avlut_four, 6);
	avlut_append(&tree, &avlut_six, 7);
	avlut_append(&tree, &avlut_seven, 8);
	avlut_append(&tree, &avlut_eight, 9);
	avlut_append(&tree, &avlut_nine, 10);
	avlut_append(&tree, &avlut_ten, 11);
	avlut_append(&tree, &avlut_eleven, 12);
	avlut_append(&tree, &avlut_twelve, 13);
	
	const struct avlut_node *remain0[] = {
		&avlut_zero,
		&avlut_one,
		&avlut_two,
		&avlut_three,
		&avlut_four,
		&avlut_five,
		&avlut_six,
		&avlut_seven,
		&avlut_eight,
		&avlut_nine,
		&avlut_ten,
		&avlut_eleven,
	};
	const struct avlut_node *remain1[] = {
		&avlut_zero,
		&avlut_one,
		&avlut_two,
		&avlut_three,
		&avlut_four,
		&avlut_five,
		&avlut_six,
		&avlut_seven,
		&avlut_eight,
		&avlut_nine,
		&avlut_ten,
	};
	const struct avlut_node *remain2[] = {
		&avlut_zero,
		&avlut_one,
		&avlut_two,
		&avlut_three,
		&avlut_four,
		&avlut_five,
		&avlut_six,
		&avlut_seven,
		&avlut_nine,
		&avlut_ten,
	};
	const struct avlut_node *remain3[] = {
		&avlut_zero,
		&avlut_one,
		&avlut_two,
		&avlut_three,
		&avlut_four,
		&avlut_five,
		&avlut_six,
		&avlut_seven,
		&avlut_nine,
	};
	const struct avlut_node *remain4[] = {
		&avlut_zero,
		&avlut_one,
		&avlut_two,
		&avlut_three,
		&avlut_four,
		&avlut_five,
		&avlut_six,
		&avlut_seven,
	};
	const struct avlut_node *remain5[] = {
		&avlut_zero,
		&avlut_one,
		&avlut_two,
		&avlut_three,
		&avlut_four,
		&avlut_five,
		&avlut_six,
	};
	const struct avlut_node *remain6[] = {
		&avlut_zero,
		&avlut_one,
		&avlut_two,
		&avlut_four,
		&avlut_five,
		&avlut_six,
	};
	const struct avlut_node *remain7[] = {
		&avlut_zero,
		&avlut_two,
		&avlut_four,
		&avlut_five,
		&avlut_six,
	};
	const struct avlut_node *remain8[] = {
		&avlut_zero,
		&avlut_two,
		&avlut_five,
		&avlut_six,
	};
	const struct avlut_node *remain9[] = {
		&avlut_zero,
		&avlut_five,
		&avlut_six,
	};
	const struct avlut_node *remain10[] = {
		&avlut_zero,
		&avlut_six,
	};
	const struct avlut_node *remain11[] = {
		&avlut_zero,
	};

	avlut_delete_one(&tree, &avlut_twelve, remain0, array_nr(remain0));
	avlut_delete_one(&tree, &avlut_eleven, remain1, array_nr(remain1));
	avlut_delete_one(&tree, &avlut_eight, remain2, array_nr(remain2));
	avlut_delete_one(&tree, &avlut_ten, remain3, array_nr(remain3));
	avlut_delete_one(&tree, &avlut_nine, remain4, array_nr(remain4));
	avlut_delete_one(&tree, &avlut_seven, remain5, array_nr(remain5));
	avlut_delete_one(&tree, &avlut_three, remain6, array_nr(remain6));
	avlut_delete_one(&tree, &avlut_one, remain7, array_nr(remain7));
	avlut_delete_one(&tree, &avlut_four, remain8, array_nr(remain8));
	avlut_delete_one(&tree, &avlut_two, remain9, array_nr(remain9));
	avlut_delete_one(&tree, &avlut_five, remain10, array_nr(remain10));
	avlut_delete_one(&tree, &avlut_six, remain11, array_nr(remain11));
}

Test(avlut_delete, delete_right_son)
{
	struct avl_tree tree;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);

	avlut_append(&tree, &avlut_seven, 1);
	avlut_append(&tree, &avlut_four, 2);
	avlut_append(&tree, &avlut_ten, 3);
	avlut_append(&tree, &avlut_two, 4);
	avlut_append(&tree, &avlut_five, 5);
	avlut_append(&tree, &avlut_nine, 6);
	avlut_append(&tree, &avlut_eleven, 7);
	avlut_append(&tree, &avlut_one, 8);
	avlut_append(&tree, &avlut_three, 9);
	avlut_append(&tree, &avlut_six, 10);
	avlut_append(&tree, &avlut_eight, 11);
	
	const struct avlut_node *remain0[] = {
		&avlut_one,
		&avlut_two,
		&avlut_three,
		&avlut_four,
		&avlut_five,
		&avlut_six,
		&avlut_seven,
		&avlut_eight,
		&avlut_nine,
		&avlut_eleven,
	};

	avlut_delete_one(&tree, &avlut_ten, remain0, array_nr(remain0));
}

Test(avlut_delete, delete_root)
{
	struct avl_tree tree;

	avl_init_tree(&tree, avlut_compare, NULL, NULL);

	avlut_append(&tree, &avlut_five, 1);
	avlut_append(&tree, &avlut_two, 2);
	avlut_append(&tree, &avlut_eight, 3);
	avlut_append(&tree, &avlut_zero, 4);
	avlut_append(&tree, &avlut_four, 5);
	avlut_append(&tree, &avlut_six, 6);
	avlut_append(&tree, &avlut_twelve, 7);
	avlut_append(&tree, &avlut_one, 8);
	avlut_append(&tree, &avlut_three, 9);
	avlut_append(&tree, &avlut_seven, 10);
	avlut_append(&tree, &avlut_ten, 11);
	avlut_append(&tree, &avlut_thirteen, 12);
	avlut_append(&tree, &avlut_nine, 13);
	avlut_append(&tree, &avlut_eleven, 14);
	
	const struct avlut_node *remain0[] = {
		&avlut_zero,
		&avlut_one,
		&avlut_two,
		&avlut_three,
		&avlut_four,
		&avlut_six,
		&avlut_seven,
		&avlut_eight,
		&avlut_nine,
		&avlut_ten,
		&avlut_eleven,
		&avlut_twelve,
		&avlut_thirteen,
	};

	avlut_delete_one(&tree, &avlut_five, remain0, array_nr(remain0));
}
