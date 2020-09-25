#include <karn/pavl.h>
#include <utils/cdefs.h>
#include <criterion/criterion.h>
#include <stdio.h>
#include <errno.h>

struct pavlut_node {
	struct pavl_node avl;
	int              value;
};

static int
pavlut_compare(const struct pavl_node *node,
               const void             *key,
               const void             *data __unused)
{
	int k = (unsigned long)key;

	return ((struct pavlut_node *)node)->value - k;
}

struct pavlut_cleared_nodes {
	unsigned int               nr;
        const struct pavlut_node **nodes;
        unsigned int               excess;
};

static void
pavlut_release_node(struct pavl_node *node, void *data)
{
	const struct pavlut_node     *pavlut = (struct pavlut_node *)node;
	struct pavlut_cleared_nodes  *cleared = data;
	unsigned int                  n;

	for (n = 0; n < cleared->nr; n++) {
		if (pavlut == cleared->nodes[n]) {
			cleared->nodes[n] = NULL;
			return;
		}
	}

	cleared->excess++;
}

static int
pavlut_compare_nodes(const struct pavl_node *first,
                     const struct pavl_node *second)
{
	return ((struct pavlut_node *)first)->value -
	       ((struct pavlut_node *)second)->value;
}

static bool
pavlut_check_tree(const struct pavl_tree *tree, unsigned long count)
{
	return pavl_check_tree(tree, count, pavlut_compare_nodes);
}

static void
pavlut_print_node(const struct pavl_node *node)
{
	const struct pavlut_node *key = (struct pavlut_node *)node;

	printf("%d(%d)", key->value, (int)node->balance);
}

static void
pavlut_print_tree(const struct pavl_tree *tree)
{
	pavl_print_tree(tree, 16U, pavlut_print_node);
}

static int pavlut_absent_key = 100;

/*
 * Empty tree.
 */

static struct pavlut_cleared_nodes pavlut_empty_cleared = {
	.nr     = 0,
	.nodes  = NULL,
	.excess = 0,
};

static struct pavl_tree pavlut_empty_tree =
	PAVL_INIT_TREE(&pavlut_compare,
	               pavlut_release_node,
	               &pavlut_empty_cleared);

/*
 * Single node large tree.
 */
static struct pavlut_node pavlut_single = {
	.avl   = {
		.children = {
			[PAVL_LEFT_SIDE]  = NULL,
			[PAVL_RIGHT_SIDE] = NULL
		},
		.parent   = NULL,
		.balance  = 0
	},
	.value = 1
};

static const struct pavlut_node * pavlut_single_cleared_nodes[] = {
	&pavlut_single
};

static struct pavlut_cleared_nodes pavlut_single_cleared = {
	.nr     = 1,
	.nodes  = pavlut_single_cleared_nodes,
	.excess = 0,
};

static struct pavl_tree pavlut_single_tree = {
	.count   = 1,
	.root    = &pavlut_single.avl,
	.data    = &pavlut_single_cleared,
	.compare = pavlut_compare,
	.release = pavlut_release_node
};

/*
 * 2 nodes large tree with one root and one left child:
 *
 *             [1]     <pavlut_double_left_top>
 *             /
 *           [0]       <pavlut_double_left_child>
 */
static struct pavlut_node pavlut_double_left_top;
static struct pavlut_node pavlut_double_left_child;

static const struct pavlut_node * pavlut_double_left_cleared_nodes[] = {
	&pavlut_double_left_child,
	&pavlut_double_left_top
};

static struct pavlut_cleared_nodes pavlut_double_left_cleared = {
	.nr     = array_nr(pavlut_double_left_cleared_nodes),
	.nodes  = pavlut_double_left_cleared_nodes,
	.excess = 0,
};

static struct pavl_tree pavlut_double_left_tree = {
	.count   = 2,
	.root    = &pavlut_double_left_top.avl,
	.data    = &pavlut_double_left_cleared,
	.compare = pavlut_compare,
	.release = pavlut_release_node
};

static struct pavlut_node pavlut_double_left_top = {
	.avl   = {
		.children = {
			[PAVL_LEFT_SIDE]  = &pavlut_double_left_child.avl,
			[PAVL_RIGHT_SIDE] = NULL
		},
		.parent   = NULL,
		.balance  = -1
	},
	.value = 1
};

static struct pavlut_node pavlut_double_left_child = {
	.avl   = {
		.children = {
			[PAVL_LEFT_SIDE]  = NULL,
			[PAVL_RIGHT_SIDE] = NULL
		},
		.parent   = &pavlut_double_left_top.avl,
		.balance  = 0
	},
	.value = 0
};

/*
 * 2 nodes large tree with one root and one right child:
 *
 *             [1]     <pavlut_double_right_top>
 *               \
 *               [2]   <pavlut_double_right_child>
 */
static struct pavlut_node pavlut_double_right_top;
static struct pavlut_node pavlut_double_right_child;

static const struct pavlut_node * pavlut_double_right_cleared_nodes[] = {
	&pavlut_double_right_top,
	&pavlut_double_right_child
};

static struct pavlut_cleared_nodes pavlut_double_right_cleared = {
	.nr     = array_nr(pavlut_double_right_cleared_nodes),
	.nodes  = pavlut_double_right_cleared_nodes,
	.excess = 0,
};

static struct pavl_tree pavlut_double_right_tree = {
	.count   = 2,
	.root    = &pavlut_double_right_top.avl,
	.data    = &pavlut_double_right_cleared,
	.compare = pavlut_compare,
	.release = pavlut_release_node
};

static struct pavlut_node pavlut_double_right_top = {
	.avl   = {
		.children = {
			[PAVL_LEFT_SIDE]  = NULL,
			[PAVL_RIGHT_SIDE] = &pavlut_double_right_child.avl
		},
		.parent   = NULL,
		.balance  = 1
	},
	.value = 1
};

static struct pavlut_node pavlut_double_right_child = {
	.avl   = {
		.children = {
			[PAVL_LEFT_SIDE]  = NULL,
			[PAVL_RIGHT_SIDE] = NULL
		},
		.parent   = &pavlut_double_right_top.avl,
		.balance  = 0
	},
	.value = 2
};

/*
 * 3 nodes large tree with one root, one right and one left child:
 *
 *                               [1]   <pavlut_triple_top>
 *                               / \
 *  <pavlut_triple_left_child> [0] [2] <pavlut_triple_right_child>
 */
static struct pavlut_node pavlut_triple_top;
static struct pavlut_node pavlut_triple_left_child;
static struct pavlut_node pavlut_triple_right_child;
static struct pavl_tree   pavlut_triple_tree = {
	.count   = 3,
	.root    = &pavlut_triple_top.avl,
	.data    = NULL,
	.compare = pavlut_compare,
	.release = NULL
};

static struct pavlut_node pavlut_triple_top = {
	.avl   = {
		.children = {
			[PAVL_LEFT_SIDE]  = &pavlut_triple_left_child.avl,
			[PAVL_RIGHT_SIDE] = &pavlut_triple_right_child.avl
		},
		.parent   = NULL,
		.balance  = 0
	},
	.value = 1
};

static struct pavlut_node pavlut_triple_left_child = {
	.avl   = {
		.children = {
			[PAVL_LEFT_SIDE]  = NULL,
			[PAVL_RIGHT_SIDE] = NULL
		},
		.parent   = &pavlut_triple_top.avl,
		.balance  = 0
	},
	.value = 0
};

static struct pavlut_node pavlut_triple_right_child = {
	.avl   = {
		.children = {
			[PAVL_LEFT_SIDE]  = NULL,
			[PAVL_RIGHT_SIDE] = NULL
		},
		.parent   = &pavlut_triple_top.avl,
		.balance  = 0
	},
	.value = 2
};

/*
 * 13 nodes large tree used for remaining tests.
 *
 *         +--------[6]--------+
 *         |                   |
 *    +---[2]---+         +---[10]---+
 *    |         |         |          |
 *   [0]-+   +-[4]-+   +-[8]-+   +-[12]
 *       |   |     |   |     |   |
 *      [1] [3]   [5] [7]   [9] [11]
 *
 */
static struct pavlut_node pavlut_zero;
static struct pavlut_node pavlut_one;
static struct pavlut_node pavlut_two;
static struct pavlut_node pavlut_three;
static struct pavlut_node pavlut_four;
static struct pavlut_node pavlut_five;
static struct pavlut_node pavlut_six;
static struct pavlut_node pavlut_seven;
static struct pavlut_node pavlut_eight;
static struct pavlut_node pavlut_nine;
static struct pavlut_node pavlut_ten;
static struct pavlut_node pavlut_eleven;
static struct pavlut_node pavlut_twelve;

static const struct pavlut_node * pavlut_complex_cleared_nodes[] = {
	&pavlut_zero,
	&pavlut_one,
	&pavlut_two,
	&pavlut_three,
	&pavlut_four,
	&pavlut_five,
	&pavlut_six,
	&pavlut_seven,
	&pavlut_eight,
	&pavlut_nine,
	&pavlut_ten,
	&pavlut_eleven,
	&pavlut_twelve
};

static struct pavlut_cleared_nodes pavlut_complex_cleared = {
	.nr     = array_nr(pavlut_complex_cleared_nodes),
	.nodes  = pavlut_complex_cleared_nodes,
	.excess = 0,
};

static struct pavl_tree pavlut_complex_tree = {
	.count   = 13,
	.root    = &pavlut_six.avl,
	.data    = &pavlut_complex_cleared,
	.compare = pavlut_compare,
	.release = pavlut_release_node
};

static struct pavlut_node pavlut_zero = {
	.avl   = {
		.children = {
			[PAVL_LEFT_SIDE]  = NULL,
			[PAVL_RIGHT_SIDE] = &pavlut_one.avl
		},
		.parent   = &pavlut_two.avl,
		.balance  = 1
	},
	.value = 0
};

static struct pavlut_node pavlut_one = {
	.avl   = {
		.children = {
			[PAVL_LEFT_SIDE]  = NULL,
			[PAVL_RIGHT_SIDE] = NULL
		},
		.parent   = &pavlut_zero.avl,
		.balance  = 0
	},
	.value = 1
};

static struct pavlut_node pavlut_two = {
	.avl   = {
		.children = {
			[PAVL_LEFT_SIDE]  = &pavlut_zero.avl,
			[PAVL_RIGHT_SIDE] = &pavlut_four.avl
		},
		.parent   = &pavlut_six.avl,
		.balance  = 0
	},
	.value = 2
};

static struct pavlut_node pavlut_three = {
	.avl   = {
		.children = {
			[PAVL_LEFT_SIDE]  = NULL,
			[PAVL_RIGHT_SIDE] = NULL
		},
		.parent   = &pavlut_four.avl,
		.balance  = 0
	},
	.value = 3
};

static struct pavlut_node pavlut_four = {
	.avl   = {
		.children = {
			[PAVL_LEFT_SIDE]  = &pavlut_three.avl,
			[PAVL_RIGHT_SIDE] = &pavlut_five.avl
		},
		.parent   = &pavlut_two.avl,
		.balance  = 0
	},
	.value = 4
};

static struct pavlut_node pavlut_five = {
	.avl   = {
		.children = {
			[PAVL_LEFT_SIDE]  = NULL,
			[PAVL_RIGHT_SIDE] = NULL
		},
		.parent   = &pavlut_four.avl,
		.balance  = 0
	},
	.value = 5
};

static struct pavlut_node pavlut_six = {
	.avl   = {
		.children = {
			[PAVL_LEFT_SIDE]  = &pavlut_two.avl,
			[PAVL_RIGHT_SIDE] = &pavlut_ten.avl
		},
		.parent   = NULL,
		.balance  = 0
	},
	.value = 6
};

static struct pavlut_node pavlut_seven = {
	.avl   = {
		.children = {
			[PAVL_LEFT_SIDE]  = NULL,
			[PAVL_RIGHT_SIDE] = NULL
		},
		.parent   = &pavlut_eight.avl,
		.balance  = 0
	},
	.value = 7
};

static struct pavlut_node pavlut_eight = {
	.avl   = {
		.children = {
			[PAVL_LEFT_SIDE]  = &pavlut_seven.avl,
			[PAVL_RIGHT_SIDE] = &pavlut_nine.avl
		},
		.parent   = &pavlut_ten.avl,
		.balance  = 0
	},
	.value = 8
};

static struct pavlut_node pavlut_nine = {
	.avl   = {
		.children = {
			[PAVL_LEFT_SIDE]  = NULL,
			[PAVL_RIGHT_SIDE] = NULL
		},
		.parent   = &pavlut_eight.avl,
		.balance  = 0
	},
	.value = 9
};

static struct pavlut_node pavlut_ten = {
	.avl   = {
		.children = {
			[PAVL_LEFT_SIDE]  = &pavlut_eight.avl,
			[PAVL_RIGHT_SIDE] = &pavlut_twelve.avl,
		},
		.parent   = &pavlut_six.avl,
		.balance  = 0
	},
	.value = 10
};

static struct pavlut_node pavlut_eleven = {
	.avl   = {
		.children = {
			[PAVL_LEFT_SIDE]  = NULL,
			[PAVL_RIGHT_SIDE] = NULL
		},
		.parent   = &pavlut_twelve.avl,
		.balance  = 0
	},
	.value = 11
};

static struct pavlut_node pavlut_twelve = {
	.avl   = {
		.children = {
			[PAVL_LEFT_SIDE]  = &pavlut_eleven.avl,
			[PAVL_RIGHT_SIDE] = NULL
		},
		.parent   = &pavlut_ten.avl,
		.balance  = -1
	},
	.value = 12
};

static void
pavlut_check_forward_inorder(const struct pavl_tree           *tree,
                             const struct pavlut_node * const *expected,
                             unsigned long                     nr)
{
	struct pavl_node *node;
	unsigned long     cnt;

	cr_expect_eq(pavl_tree_count(tree),
	             nr,
	             "unexpected tree count: %lu != %lu\n",
	             pavl_tree_count(tree),
	             nr);
	if (pavl_tree_count(tree) != nr)
		goto fail;

	cnt = 0;
	pavl_walk_forward_inorder(tree, node) {
		const struct pavlut_node *n = (struct pavlut_node *)node;

		cr_expect_eq(n,
		             expected[cnt],
		             "unexpected tree iteration node: "
		             "%d[%p] != %d[%p]\n",
		             n->value,
		             n,
		             expected[cnt]->value,
		             expected[cnt]);
		if (n != expected[cnt])
			goto fail;

		cnt++;
	}

	cr_expect_eq(cnt,
	             nr,
	             "unexpected tree iteration count: %lu != %lu\n",
	             cnt,
	             nr);
	if (cnt != nr)
		goto fail;

	return;

fail:
	cr_assert_fail("iteration checking failed\n");
}

static void
pavlut_check_backward_inorder(const struct pavl_tree           *tree,
                              const struct pavlut_node * const *expected,
                              unsigned long                     nr)
{
	struct pavl_node *node;
	unsigned long     cnt;

	cr_expect_eq(pavl_tree_count(tree),
	             nr,
	             "unexpected tree count: %lu != %lu\n",
	             pavl_tree_count(tree),
	             nr);
	if (pavl_tree_count(tree) != nr)
		goto fail;

	cnt = 0;
	pavl_walk_backward_inorder(tree, node) {
		const struct pavlut_node *n = (struct pavlut_node *)node;

		cr_expect_eq(n,
		             expected[cnt],
		             "unexpected tree iteration node: "
		             "%d[%p] != %d[%p]\n",
		             n->value,
		             n,
		             expected[cnt]->value,
		             expected[cnt]);
		if (n != expected[cnt])
			goto fail;

		cnt++;
	}

	cr_expect_eq(cnt,
	             nr,
	             "unexpected tree iteration count: %lu != %lu\n",
	             cnt,
	             nr);
	if (cnt != nr)
		goto fail;

	return;

fail:
	cr_assert_fail("iteration checking failed\n");
}

static bool
pavlut_check_forward_inorder_from(unsigned int                      first,
                                  const struct pavlut_node * const *expected,
                                  unsigned int                      nr)
{
	const struct pavl_node *node;
	unsigned int            cnt;

	cnt = first;
	for (node = &expected[cnt]->avl;
	     node;
	     node = pavl_iter_next_inorder(node)) {
		const struct pavlut_node *n = (struct pavlut_node *)node;

		cr_expect_eq(n,
		             expected[cnt],
		             "unexpected tree iteration node: "
		             "%d[%p] != %d[%p]\n",
		             n->value,
		             n,
		             expected[cnt]->value,
		             expected[cnt]);
		if (n != expected[cnt])
			return false;

		cnt++;
	}

	cr_expect_eq(cnt - first,
	             nr,
	             "unexpected tree iteration count: %u != %u\n",
	             cnt - first,
	             nr);
	if ((cnt - first) != nr)
		return false;

	return true;
}

static void
paclut_check_init_forward_inorder(const struct pavl_tree           *tree,
                                  const struct pavlut_node * const *expected,
                                  unsigned long                     nr)
{
	unsigned int first;

	cr_expect_eq(pavl_tree_count(tree),
	             nr,
	             "unexpected tree count: %lu != %lu\n",
	             pavl_tree_count(tree),
	             nr);
	if (pavl_tree_count(tree) != nr)
		goto fail;

	for (first = 0; first < nr; first++)
		if (!pavlut_check_forward_inorder_from(first,
		                                       expected,
		                                       nr - first))
			goto fail;

	return;

fail:
	cr_assert_fail("initialized iteration checking failed\n");
}

static bool
pavlut_check_backward_inorder_from(unsigned int                      last,
                                   const struct pavlut_node * const *expected,
                                   unsigned int                      nr)
{
	const struct pavl_node *node;
	unsigned int            cnt;

	cnt = last;
	for (node = &expected[last]->avl;
	     node;
	     node = pavl_iter_prev_inorder(node)) {
		const struct pavlut_node *n = (struct pavlut_node *)node;

		cr_expect_eq(n,
		             expected[cnt],
		             "unexpected tree iteration node: "
		             "%d[%p] != %d[%p]\n",
		             n->value,
		             n,
		             expected[cnt]->value,
		             expected[cnt]);
		if (n != expected[cnt])
			return false;

		cnt++;
	}

	cr_expect_eq(cnt - last,
	             nr,
	             "unexpected tree iteration count: %u != %u\n",
	             cnt - last,
	             nr);
	if ((cnt - last) != nr)
		return false;

	return true;
}

static void
pavlut_check_init_backward_inorder(const struct pavl_tree           *tree,
                                   const struct pavlut_node * const *expected,
                                   unsigned long                     nr)
{
	unsigned int last;

	cr_expect_eq(pavl_tree_count(tree),
	             nr,
	             "unexpected tree count: %lu != %lu\n",
	             pavl_tree_count(tree),
	             nr);
	if (pavl_tree_count(tree) != nr)
		goto fail;

	for (last = 0; last < nr; last++) {
		if (!pavlut_check_backward_inorder_from(last,
		                                        expected,
		                                        nr - last))
			goto fail;
	}

	return;

fail:
	cr_assert_fail("initialized iteration checking failed\n");
}

Test(pavlut_inorder_iter, forward_empty)
{
	const struct pavlut_node * const expected[] = { };
	struct pavl_tree                 tree;

	pavl_init_tree(&tree, pavlut_compare, NULL, NULL);

	pavlut_check_forward_inorder(&tree, expected, 0);
}

Test(pavlut_inorder_iter, backward_empty)
{
	const struct pavlut_node * const expected[] = { };
	struct pavl_tree                 tree;

	pavl_init_tree(&tree, pavlut_compare, NULL, NULL);

	pavlut_check_backward_inorder(&tree, expected, 0);
}

Test(pavlut_inorder_iter, forward_single)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_single
	};

	pavlut_check_forward_inorder(&pavlut_single_tree,
	                             expected,
	                             array_nr(expected));
}

Test(pavlut_inorder_iter, forward_init_single)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_single
	};

	paclut_check_init_forward_inorder(&pavlut_single_tree,
	                                  expected,
	                                  array_nr(expected));
}

Test(pavlut_inorder_iter, backward_single)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_single
	};

	pavlut_check_backward_inorder(&pavlut_single_tree,
	                              expected,
	                              array_nr(expected));
}

Test(pavlut_inorder_iter, backward_init_single)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_single
	};

	pavlut_check_init_backward_inorder(&pavlut_single_tree,
	                                   expected,
	                                   array_nr(expected));
}

Test(pavlut_inorder_iter, forward_double_left)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_double_left_child,
		&pavlut_double_left_top
	};

	pavlut_check_forward_inorder(&pavlut_double_left_tree,
	                             expected,
	                             array_nr(expected));
}

Test(pavlut_inorder_iter, forward_init_double_left)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_double_left_child,
		&pavlut_double_left_top
	};

	paclut_check_init_forward_inorder(&pavlut_double_left_tree,
	                                  expected,
	                                  array_nr(expected));
}

Test(pavlut_inorder_iter, backward_double_left)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_double_left_top,
		&pavlut_double_left_child
	};

	pavlut_check_backward_inorder(&pavlut_double_left_tree,
	                              expected,
	                              array_nr(expected));
}

Test(pavlut_inorder_iter, backward_init_double_left)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_double_left_top,
		&pavlut_double_left_child
	};

	pavlut_check_init_backward_inorder(&pavlut_double_left_tree,
	                                   expected,
	                                   array_nr(expected));
}

Test(pavlut_inorder_iter, forward_double_right)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_double_right_top,
		&pavlut_double_right_child
	};

	pavlut_check_forward_inorder(&pavlut_double_right_tree,
	                             expected,
	                             array_nr(expected));
}

Test(pavlut_inorder_iter, forward_init_double_right)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_double_right_top,
		&pavlut_double_right_child
	};

	paclut_check_init_forward_inorder(&pavlut_double_right_tree,
	                                  expected,
	                                  array_nr(expected));
}

Test(pavlut_inorder_iter, backward_double_right)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_double_right_child,
		&pavlut_double_right_top
	};

	pavlut_check_backward_inorder(&pavlut_double_right_tree,
	                              expected,
	                              array_nr(expected));
}

Test(pavlut_inorder_iter, backward_init_double_right)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_double_right_child,
		&pavlut_double_right_top
	};

	pavlut_check_init_backward_inorder(&pavlut_double_right_tree,
	                                   expected,
	                                   array_nr(expected));
}

Test(pavlut_inorder_iter, forward_complex)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_zero,
		&pavlut_one,
		&pavlut_two,
		&pavlut_three,
		&pavlut_four,
		&pavlut_five,
		&pavlut_six,
		&pavlut_seven,
		&pavlut_eight,
		&pavlut_nine,
		&pavlut_ten,
		&pavlut_eleven,
		&pavlut_twelve
	};

	pavlut_check_forward_inorder(&pavlut_complex_tree,
	                             expected,
	                             array_nr(expected));
}

Test(pavlut_inorder_iter, forward_init_complex)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_zero,
		&pavlut_one,
		&pavlut_two,
		&pavlut_three,
		&pavlut_four,
		&pavlut_five,
		&pavlut_six,
		&pavlut_seven,
		&pavlut_eight,
		&pavlut_nine,
		&pavlut_ten,
		&pavlut_eleven,
		&pavlut_twelve
	};

	paclut_check_init_forward_inorder(&pavlut_complex_tree,
	                                  expected,
	                                  array_nr(expected));
}

Test(pavlut_inorder_iter, backward_complex)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_twelve,
		&pavlut_eleven,
		&pavlut_ten,
		&pavlut_nine,
		&pavlut_eight,
		&pavlut_seven,
		&pavlut_six,
		&pavlut_five,
		&pavlut_four,
		&pavlut_three,
		&pavlut_two,
		&pavlut_one,
		&pavlut_zero
	};

	pavlut_check_backward_inorder(&pavlut_complex_tree,
	                              expected,
	                              array_nr(expected));
}

Test(pavlut_inorder_iter, backward_init_complex)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_twelve,
		&pavlut_eleven,
		&pavlut_ten,
		&pavlut_nine,
		&pavlut_eight,
		&pavlut_seven,
		&pavlut_six,
		&pavlut_five,
		&pavlut_four,
		&pavlut_three,
		&pavlut_two,
		&pavlut_one,
		&pavlut_zero
	};

	pavlut_check_init_backward_inorder(&pavlut_complex_tree,
	                                   expected,
	                                   array_nr(expected));
}

static void
pavlut_check_forward_preorder(const struct pavl_tree           *tree,
                              const struct pavlut_node * const *expected,
                              unsigned long                     nr)
{
	struct pavl_node *node;
	unsigned long     cnt;

	cr_expect_eq(pavl_tree_count(tree),
	             nr,
	             "unexpected tree count: %lu != %lu\n",
	             pavl_tree_count(tree),
	             nr);
	if (pavl_tree_count(tree) != nr)
		goto fail;

	cnt = 0;
	pavl_walk_forward_preorder(tree, node) {
		const struct pavlut_node *n = (struct pavlut_node *)node;

		cr_expect_eq(n,
		             expected[cnt],
		             "unexpected tree iteration node: "
		             "%d[%p] != %d[%p]\n",
		             n->value,
		             n,
		             expected[cnt]->value,
		             expected[cnt]);
		if (n != expected[cnt])
			goto fail;

		cnt++;
	}

	cr_expect_eq(cnt,
	             nr,
	             "unexpected tree iteration count: %lu != %lu\n",
	             cnt,
	             nr);
	if (cnt != nr)
		goto fail;

	return;

fail:
	cr_assert_fail("iteration checking failed\n");
}

static void
pavlut_check_backward_preorder(const struct pavl_tree           *tree,
                               const struct pavlut_node * const *expected,
                               unsigned long                     nr)
{
	struct pavl_node *node;
	unsigned long     cnt;

	cr_expect_eq(pavl_tree_count(tree),
	             nr,
	             "unexpected tree count: %lu != %lu\n",
	             pavl_tree_count(tree),
	             nr);
	if (pavl_tree_count(tree) != nr)
		goto fail;

	cnt = 0;
	pavl_walk_backward_preorder(tree, node) {
		const struct pavlut_node *n = (struct pavlut_node *)node;

		cr_expect_eq(n,
		             expected[cnt],
		             "unexpected tree iteration node: "
		             "%d[%p] != %d[%p]\n",
		             n->value,
		             n,
		             expected[cnt]->value,
		             expected[cnt]);
		if (n != expected[cnt])
			goto fail;

		cnt++;
	}

	cr_expect_eq(cnt,
	             nr,
	             "unexpected tree iteration count: %lu != %lu\n",
	             cnt,
	             nr);
	if (cnt != nr)
		goto fail;

	return;

fail:
	cr_assert_fail("iteration checking failed\n");
}

static bool
pavlut_check_forward_preorder_from(unsigned int                      first,
                                   const struct pavlut_node * const *expected,
                                   unsigned int                      nr)
{
	const struct pavl_node *node;
	unsigned int            cnt;

	cnt = first;
	for (node = &expected[cnt]->avl;
	     node;
	     node = pavl_iter_next_preorder(node)) {
		const struct pavlut_node *n = (struct pavlut_node *)node;

		cr_expect_eq(n,
		             expected[cnt],
		             "unexpected tree iteration node: "
		             "%d[%p] != %d[%p]\n",
		             n->value,
		             n,
		             expected[cnt]->value,
		             expected[cnt]);
		if (n != expected[cnt])
			return false;

		cnt++;
	}

	cr_expect_eq(cnt - first,
	             nr,
	             "unexpected tree iteration count: %u != %u\n",
	             cnt - first,
	             nr);
	if ((cnt - first) != nr)
		return false;

	return true;
}

static void
pavlut_check_init_forward_preorder(const struct pavl_tree           *tree,
                                   const struct pavlut_node * const *expected,
                                   unsigned long                     nr)
{
	unsigned int first;

	cr_expect_eq(pavl_tree_count(tree),
	             nr,
	             "unexpected tree count: %lu != %lu\n",
	             pavl_tree_count(tree),
	             nr);
	if (pavl_tree_count(tree) != nr)
		goto fail;

	for (first = 0; first < nr; first++)
		if (!pavlut_check_forward_preorder_from(first,
		                                        expected,
		                                        nr - first))
			goto fail;

	return;

fail:
	cr_assert_fail("initialized iteration checking failed\n");
}

static bool
pavlut_check_backward_preorder_from(unsigned int                      last,
                                    const struct pavlut_node * const *expected,
                                    unsigned int                      nr)
{
	const struct pavl_node *node;
	unsigned int            cnt;

	cnt = last;
	for (node = &expected[last]->avl;
	     node;
	     node = pavl_iter_prev_preorder(node)) {
		const struct pavlut_node *n = (struct pavlut_node *)node;

		cr_expect_eq(n,
		             expected[cnt],
		             "unexpected tree iteration node: "
		             "%d[%p] != %d[%p]\n",
		             n->value,
		             n,
		             expected[cnt]->value,
		             expected[cnt]);
		if (n != expected[cnt])
			return false;

		cnt++;
	}

	cr_expect_eq(cnt - last,
	             nr,
	             "unexpected tree iteration count: %u != %u\n",
	             cnt - last,
	             nr);
	if ((cnt - last) != nr)
		return false;

	return true;
}

static void
pavlut_check_init_backward_preorder(const struct pavl_tree           *tree,
                                    const struct pavlut_node * const *expected,
                                    unsigned long                     nr)
{
	unsigned int last;

	cr_expect_eq(pavl_tree_count(tree),
	             nr,
	             "unexpected tree count: %lu != %lu\n",
	             pavl_tree_count(tree),
	             nr);
	if (pavl_tree_count(tree) != nr)
		goto fail;

	for (last = 0; last < nr; last++) {
		if (!pavlut_check_backward_preorder_from(last,
		                                         expected,
		                                         nr - last))
			goto fail;
	}

	return;

fail:
	cr_assert_fail("initialized iteration checking failed\n");
}

Test(pavlut_preorder_iter, forward_empty)
{
	const struct pavlut_node * const expected[] = { };
	struct pavl_tree                 tree;

	pavl_init_tree(&tree, pavlut_compare, NULL, NULL);

	pavlut_check_forward_preorder(&tree, expected, 0);
}

Test(pavlut_preorder_iter, backward_empty)
{
	const struct pavlut_node * const expected[] = { };
	struct pavl_tree                 tree;

	pavl_init_tree(&tree, pavlut_compare, NULL, NULL);

	pavlut_check_backward_preorder(&tree, expected, 0);
}

Test(pavlut_preorder_iter, forward_single)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_single
	};

	pavlut_check_forward_preorder(&pavlut_single_tree,
	                              expected,
	                              array_nr(expected));
}

Test(pavlut_preorder_iter, forward_init_single)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_single
	};

	pavlut_check_init_forward_preorder(&pavlut_single_tree,
	                                   expected,
	                                   array_nr(expected));
}

Test(pavlut_preorder_iter, backward_single)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_single
	};

	pavlut_check_backward_preorder(&pavlut_single_tree,
	                               expected,
	                               array_nr(expected));
}

Test(pavlut_preorder_iter, backward_init_single)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_single
	};

	pavlut_check_init_backward_preorder(&pavlut_single_tree,
	                                    expected,
	                                    array_nr(expected));
}

Test(pavlut_preorder_iter, forward_double_left)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_double_left_top,
		&pavlut_double_left_child
	};

	pavlut_check_forward_preorder(&pavlut_double_left_tree,
	                              expected,
	                              array_nr(expected));
}

Test(pavlut_preorder_iter, forward_init_double_left)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_double_left_top,
		&pavlut_double_left_child
	};

	pavlut_check_init_forward_preorder(&pavlut_double_left_tree,
	                                   expected,
	                                   array_nr(expected));
}

Test(pavlut_preorder_iter, backward_double_left)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_double_left_top,
		&pavlut_double_left_child
	};

	pavlut_check_backward_preorder(&pavlut_double_left_tree,
	                               expected,
	                               array_nr(expected));
}

Test(pavlut_preorder_iter, backward_init_double_left)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_double_left_top,
		&pavlut_double_left_child
	};

	pavlut_check_init_backward_preorder(&pavlut_double_left_tree,
	                                    expected,
	                                    array_nr(expected));
}

Test(pavlut_preorder_iter, forward_double_right)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_double_right_top,
		&pavlut_double_right_child
	};

	pavlut_check_forward_preorder(&pavlut_double_right_tree,
	                              expected,
	                              array_nr(expected));
}

Test(pavlut_preorder_iter, forward_init_double_right)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_double_right_top,
		&pavlut_double_right_child
	};

	pavlut_check_init_forward_preorder(&pavlut_double_right_tree,
	                                   expected,
	                                   array_nr(expected));
}

Test(pavlut_preorder_iter, backward_double_right)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_double_right_top,
		&pavlut_double_right_child
	};

	pavlut_check_backward_preorder(&pavlut_double_right_tree,
	                               expected,
	                               array_nr(expected));
}

Test(pavlut_preorder_iter, backward_init_double_right)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_double_right_top,
		&pavlut_double_right_child
	};

	pavlut_check_init_backward_preorder(&pavlut_double_right_tree,
	                                    expected,
	                                    array_nr(expected));
}

Test(pavlut_preorder_iter, forward_complex)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_six,
		&pavlut_two,
		&pavlut_zero,
		&pavlut_one,
		&pavlut_four,
		&pavlut_three,
		&pavlut_five,
		&pavlut_ten,
		&pavlut_eight,
		&pavlut_seven,
		&pavlut_nine,
		&pavlut_twelve,
		&pavlut_eleven
	};

	pavlut_check_forward_preorder(&pavlut_complex_tree,
	                              expected,
	                              array_nr(expected));
}

Test(pavlut_preorder_iter, forward_init_complex)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_six,
		&pavlut_two,
		&pavlut_zero,
		&pavlut_one,
		&pavlut_four,
		&pavlut_three,
		&pavlut_five,
		&pavlut_ten,
		&pavlut_eight,
		&pavlut_seven,
		&pavlut_nine,
		&pavlut_twelve,
		&pavlut_eleven
	};

	pavlut_check_init_forward_preorder(&pavlut_complex_tree,
	                                   expected,
	                                   array_nr(expected));
}

Test(pavlut_preorder_iter, backward_complex)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_six,
		&pavlut_ten,
		&pavlut_twelve,
		&pavlut_eleven,
		&pavlut_eight,
		&pavlut_nine,
		&pavlut_seven,
		&pavlut_two,
		&pavlut_four,
		&pavlut_five,
		&pavlut_three,
		&pavlut_zero,
		&pavlut_one
	};

	pavlut_check_backward_preorder(&pavlut_complex_tree,
	                               expected,
	                               array_nr(expected));
}

Test(pavlut_preorder_iter, backward_init_complex)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_six,
		&pavlut_ten,
		&pavlut_twelve,
		&pavlut_eleven,
		&pavlut_eight,
		&pavlut_nine,
		&pavlut_seven,
		&pavlut_two,
		&pavlut_four,
		&pavlut_five,
		&pavlut_three,
		&pavlut_zero,
		&pavlut_one
	};

	pavlut_check_init_backward_preorder(&pavlut_complex_tree,
	                                    expected,
	                                    array_nr(expected));
}

static void
pavlut_find_present_node(const struct pavl_tree   *tree,
                         const struct pavlut_node *expected)
{
	const struct pavl_node *found;

	found = pavl_find_node(tree, (void *)((long)expected->value));

	cr_expect_not_null(found, "tree node not found\n");
	if (!found)
		return;

	cr_expect_eq(found,
	             &expected->avl,
	             "wrong tree node found: %d[%p] != %d[%p]\n",
	             ((struct pavlut_node *)found)->value,
	             found,
	             expected->value,
	             expected);
}

static void
pavlut_find_absent_key(const struct pavl_tree *tree, int key)

{
	const struct pavl_node *found;

	found = pavl_find_node(tree, (void *)((unsigned long)key));

	cr_expect_null(found, "unexpected tree node found\n");
}

static void
pavlut_check_find(const struct pavl_tree           *tree,
                  const struct pavlut_node * const *expected,
                  unsigned long                     nr)
{
	unsigned int n;

	for (n = 0; n < nr; n++)
		pavlut_find_present_node(tree, expected[n]);

	pavlut_find_absent_key(tree, pavlut_absent_key);
}

Test(pavlut_find, find_present_into_complex)
{
	unsigned int                     n;
	const struct pavlut_node * const expected[] = {
		&pavlut_zero,
		&pavlut_one,
		&pavlut_two,
		&pavlut_three,
		&pavlut_four,
		&pavlut_five,
		&pavlut_six,
		&pavlut_seven,
		&pavlut_eight,
		&pavlut_nine,
		&pavlut_ten,
		&pavlut_eleven,
		&pavlut_twelve
	};

	for (n = 0; n < array_nr(expected); n++)
		pavlut_find_present_node(&pavlut_complex_tree, expected[n]);
}

Test(pavlut_find, find_absent_into_complex)
{
	pavlut_find_absent_key(&pavlut_complex_tree, pavlut_absent_key);
}

static void
pavlut_append_one_node(struct pavl_tree                 *tree,
                       struct pavlut_node               *node,
                       const struct pavlut_node * const *expected,
                       unsigned long                     nr)
{
	int              ret;
	struct pavl_node dup;

	assert(nr);

	memset(&node->avl, 0, sizeof(node->avl));

	ret = pavl_append_node(tree, &node->avl, (void *)((long)node->value));

	cr_expect_eq(ret,
	             0,
	             "append node failed: %s (%d)\n",
	             strerror(-ret),
	             -ret);
	if (ret)
		return;

	pavlut_check_find(tree, expected, nr);

	pavlut_check_forward_inorder(tree, expected, nr);

	ret = pavl_append_node(tree,
	                       &dup,
	                       (void *)((long)node->value));
	cr_expect_eq(ret,
	             -EEXIST,
	             "append duplicate returned: %s (%d)\n",
	             strerror(-ret),
	             -ret);

	cr_expect(pavlut_check_tree(tree, nr), "tree property violation\n");
}

Test(pavlut_append, one_into_empty)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_single
	};

	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_single,
	                       expected,
	                       array_nr(expected));
}

Test(pavlut_append, left_child_into_single)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_double_left_child,
		&pavlut_single
	};

	pavlut_append_one_node(&pavlut_single_tree,
	                       &pavlut_double_left_child,
	                       expected,
	                       array_nr(expected));
}

Test(pavlut_append, right_child_into_single)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_single,
		&pavlut_double_right_child
	};

	pavlut_append_one_node(&pavlut_single_tree,
	                       &pavlut_double_right_child,
	                       expected,
	                       array_nr(expected));
}

Test(pavlut_append, small_then_large_into_single)
{
	const struct pavlut_node * const expected0[] = {
		&pavlut_double_left_child,
		&pavlut_single
	};
	const struct pavlut_node * const expected1[] = {
		&pavlut_double_left_child,
		&pavlut_single,
		&pavlut_double_right_child
	};

	pavlut_append_one_node(&pavlut_single_tree,
	                       &pavlut_double_left_child,
	                       expected0,
	                       array_nr(expected0));

	pavlut_append_one_node(&pavlut_single_tree,
	                       &pavlut_double_right_child,
	                       expected1,
	                       array_nr(expected1));
}

Test(pavlut_append, large_then_small_into_single)
{
	const struct pavlut_node * const expected0[] = {
		&pavlut_single,
		&pavlut_double_right_child
	};
	const struct pavlut_node * const expected1[] = {
		&pavlut_double_left_child,
		&pavlut_single,
		&pavlut_double_right_child
	};

	pavlut_append_one_node(&pavlut_single_tree,
	                       &pavlut_double_right_child,
	                       expected0,
	                       array_nr(expected0));

	pavlut_append_one_node(&pavlut_single_tree,
	                       &pavlut_double_left_child,
	                       expected1,
	                       array_nr(expected1));
}

Test(pavlut_append, rotate_right_into_empty)
{
	const struct pavlut_node * const expected0[] = {
		&pavlut_two,
	};
	const struct pavlut_node * const expected1[] = {
		&pavlut_one,
		&pavlut_two
	};
	const struct pavlut_node * const expected2[] = {
		&pavlut_zero,
		&pavlut_one,
		&pavlut_two
	};

	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_two,
	                       expected0,
	                       array_nr(expected0));
	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_one,
	                       expected1,
	                       array_nr(expected1));
	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_zero,
	                       expected2,
	                       array_nr(expected2));
}

Test(pavlut_append, rotate_left_into_empty)
{
	const struct pavlut_node * const expected0[] = {
		&pavlut_zero,
	};
	const struct pavlut_node * const expected1[] = {
		&pavlut_zero,
		&pavlut_one
	};
	const struct pavlut_node * const expected2[] = {
		&pavlut_zero,
		&pavlut_one,
		&pavlut_two
	};

	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_zero,
	                       expected0,
	                       array_nr(expected0));
	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_one,
	                       expected1,
	                       array_nr(expected1));
	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_two,
	                       expected2,
	                       array_nr(expected2));
}

Test(pavlut_append, rotate_left_right_into_empty)
{
	const struct pavlut_node * const expected0[] = {
		&pavlut_two,
	};
	const struct pavlut_node * const expected1[] = {
		&pavlut_zero,
		&pavlut_two
	};
	const struct pavlut_node * const expected2[] = {
		&pavlut_zero,
		&pavlut_one,
		&pavlut_two
	};

	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_two,
	                       expected0,
	                       array_nr(expected0));
	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_zero,
	                       expected1,
	                       array_nr(expected1));
	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_one,
	                       expected2,
	                       array_nr(expected2));
}

Test(pavlut_append, rotate_right_left_into_empty)
{
	const struct pavlut_node * const expected0[] = {
		&pavlut_zero,
	};
	const struct pavlut_node * const expected1[] = {
		&pavlut_zero,
		&pavlut_two
	};
	const struct pavlut_node * const expected2[] = {
		&pavlut_zero,
		&pavlut_one,
		&pavlut_two
	};

	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_zero,
	                       expected0,
	                       array_nr(expected0));
	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_two,
	                       expected1,
	                       array_nr(expected1));
	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_one,
	                       expected2,
	                       array_nr(expected2));
}

static void
pavlut_init_level1_tree(void)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_four,
		&pavlut_six,
		&pavlut_ten
	};

	pavl_append_node(&pavlut_empty_tree,
	                 &pavlut_six.avl,
	                 (void *)((unsigned long)pavlut_six.value));
	pavl_append_node(&pavlut_empty_tree,
	                 &pavlut_four.avl,
	                 (void *)((unsigned long)pavlut_four.value));
	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_ten,
	                       expected,
	                       array_nr(expected));
}

Test(pavlut_append, left_level1_rotate_right)
{
	const struct pavlut_node * const expected0[] = {
		&pavlut_one,
		&pavlut_four,
		&pavlut_six,
		&pavlut_ten
	};
	const struct pavlut_node * const expected1[] = {
		&pavlut_zero,
		&pavlut_one,
		&pavlut_four,
		&pavlut_six,
		&pavlut_ten
	};

	pavlut_init_level1_tree();
	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_one,
	                       expected0,
	                       array_nr(expected0));
	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_zero,
	                       expected1,
	                       array_nr(expected1));
}

Test(pavlut_append, left_level1_rotate_left_right)
{
	const struct pavlut_node * const expected0[] = {
		&pavlut_zero,
		&pavlut_four,
		&pavlut_six,
		&pavlut_ten
	};
	const struct pavlut_node * const expected1[] = {
		&pavlut_zero,
		&pavlut_one,
		&pavlut_four,
		&pavlut_six,
		&pavlut_ten
	};

	pavlut_init_level1_tree();
	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_zero,
	                       expected0,
	                       array_nr(expected0));
	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_one,
	                       expected1,
	                       array_nr(expected1));
}

Test(pavlut_append, left_level1_rotate_left_left)
{
	const struct pavlut_node * const expected0[] = {
		&pavlut_four,
		&pavlut_five,
		&pavlut_six,
		&pavlut_ten
	};
	const struct pavlut_node * const expected1[] = {
		&pavlut_four,
		&pavlut_five,
		&pavlut_six,
		&pavlut_seven,
		&pavlut_ten
	};

	pavlut_init_level1_tree();
	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_five,
	                       expected0,
	                       array_nr(expected0));
	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_seven,
	                       expected1,
	                       array_nr(expected1));
}

Test(pavlut_append, left_level1_rotate_right_left)
{
	const struct pavlut_node * const expected0[] = {
		&pavlut_four,
		&pavlut_five,
		&pavlut_six,
		&pavlut_ten
	};
	const struct pavlut_node * const expected1[] = {
		&pavlut_three,
		&pavlut_four,
		&pavlut_five,
		&pavlut_six,
		&pavlut_ten
	};

	pavlut_init_level1_tree();
	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_five,
	                       expected0,
	                       array_nr(expected0));
	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_three,
	                       expected1,
	                       array_nr(expected1));
}

Test(pavlut_append, right_level1_rotate_left)
{
	const struct pavlut_node * const expected0[] = {
		&pavlut_four,
		&pavlut_six,
		&pavlut_ten,
		&pavlut_eleven
	};
	const struct pavlut_node * const expected1[] = {
		&pavlut_four,
		&pavlut_six,
		&pavlut_ten,
		&pavlut_eleven,
		&pavlut_twelve
	};

	pavlut_init_level1_tree();
	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_eleven,
	                       expected0,
	                       array_nr(expected0));
	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_twelve,
	                       expected1,
	                       array_nr(expected1));
}

Test(pavlut_append, right_level1_rotate_right_left)
{
	const struct pavlut_node * const expected0[] = {
		&pavlut_four,
		&pavlut_six,
		&pavlut_ten,
		&pavlut_twelve
	};
	const struct pavlut_node * const expected1[] = {
		&pavlut_four,
		&pavlut_six,
		&pavlut_ten,
		&pavlut_eleven,
		&pavlut_twelve
	};

	pavlut_init_level1_tree();
	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_twelve,
	                       expected0,
	                       array_nr(expected0));
	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_eleven,
	                       expected1,
	                       array_nr(expected1));
}

Test(pavlut_append, right_level1_rotate_right_right)
{
	const struct pavlut_node * const expected0[] = {
		&pavlut_four,
		&pavlut_six,
		&pavlut_eight,
		&pavlut_ten
	};
	const struct pavlut_node * const expected1[] = {
		&pavlut_four,
		&pavlut_six,
		&pavlut_seven,
		&pavlut_eight,
		&pavlut_ten
	};

	pavlut_init_level1_tree();
	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_eight,
	                       expected0,
	                       array_nr(expected0));
	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_seven,
	                       expected1,
	                       array_nr(expected1));
}

Test(pavlut_append, right_level1_rotate_left_right)
{
	const struct pavlut_node * const expected0[] = {
		&pavlut_four,
		&pavlut_six,
		&pavlut_eight,
		&pavlut_ten
	};
	const struct pavlut_node * const expected1[] = {
		&pavlut_four,
		&pavlut_six,
		&pavlut_eight,
		&pavlut_nine,
		&pavlut_ten
	};

	pavlut_init_level1_tree();
	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_eight,
	                       expected0,
	                       array_nr(expected0));
	pavlut_append_one_node(&pavlut_empty_tree,
	                       &pavlut_nine,
	                       expected1,
	                       array_nr(expected1));
}

Test(pavlut_append, complex_increasing)
{
	unsigned int               n;
	struct pavlut_node * const expected[] = {
		&pavlut_zero,
		&pavlut_one,
		&pavlut_two,
		&pavlut_three,
		&pavlut_four,
		&pavlut_five,
		&pavlut_six,
		&pavlut_seven,
		&pavlut_eight,
		&pavlut_nine,
		&pavlut_ten,
		&pavlut_eleven,
		&pavlut_twelve
	};

	for (n = 0; n < (array_nr(expected) - 1); n++)
		pavl_append_node(&pavlut_empty_tree,
		                 &expected[n]->avl,
		                 (void *)((long)expected[n]->value));

	pavlut_append_one_node(&pavlut_empty_tree,
	                       expected[n],
	                       (const struct pavlut_node * const *)expected,
	                       array_nr(expected));
}

Test(pavlut_append, complex_decreasing)
{
	unsigned int                     n;
	struct pavlut_node * const       append[] = {
		&pavlut_twelve,
		&pavlut_eleven,
		&pavlut_ten,
		&pavlut_nine,
		&pavlut_eight,
		&pavlut_seven,
		&pavlut_six,
		&pavlut_five,
		&pavlut_four,
		&pavlut_three,
		&pavlut_two,
		&pavlut_one,
		&pavlut_zero
	};
	const struct pavlut_node * const expected[] = {
		&pavlut_zero,
		&pavlut_one,
		&pavlut_two,
		&pavlut_three,
		&pavlut_four,
		&pavlut_five,
		&pavlut_six,
		&pavlut_seven,
		&pavlut_eight,
		&pavlut_nine,
		&pavlut_ten,
		&pavlut_eleven,
		&pavlut_twelve
	};

	for (n = 0; n < (array_nr(append) - 1); n++)
		pavl_append_node(&pavlut_empty_tree,
		                 &append[n]->avl,
		                 (void *)((long)append[n]->value));

	pavlut_append_one_node(&pavlut_empty_tree,
	                       append[n],
	                       expected,
	                       array_nr(expected));
}

static void
pavlut_delete_one_node(struct pavl_tree                 *tree,
                       struct pavlut_node               *node,
                       const struct pavlut_node * const *expected,
                       unsigned long                     nr)
{
	pavl_delete_node(tree, &node->avl);

	pavlut_find_absent_key(tree, node->value);

	pavlut_check_find(tree, expected, nr);

	pavlut_check_forward_inorder(tree, expected, nr);

	cr_expect(pavlut_check_tree(tree, nr), "tree property violation\n");
}


Test(pavlut_delete, delete_from_single)
{
	const struct pavlut_node * const expected[] = { };

	pavlut_delete_one_node(&pavlut_single_tree,
	                       &pavlut_single,
	                       expected,
	                       0);

	cr_expect_null(pavlut_single_tree.root,
	               "unexpected tree root: not NULL\n");
}

Test(pavlut_delete, delete_root_from_double_left)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_double_left_child
	};

	pavlut_delete_one_node(&pavlut_double_left_tree,
	                       &pavlut_double_left_top,
	                       expected,
	                       1);
}

Test(pavlut_delete, delete_child_from_double_left)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_double_left_top
	};

	pavlut_delete_one_node(&pavlut_double_left_tree,
	                       &pavlut_double_left_child,
	                       expected,
	                       1);
}

Test(pavlut_delete, delete_root_from_double_right)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_double_right_child
	};

	pavlut_delete_one_node(&pavlut_double_right_tree,
	                       &pavlut_double_right_top,
	                       expected,
	                       1);
}

Test(pavlut_delete, delete_child_from_double_right)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_double_right_top
	};

	pavlut_delete_one_node(&pavlut_double_right_tree,
	                       &pavlut_double_right_child,
	                       expected,
	                       1);
}

Test(pavlut_delete, delete_root_from_triple)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_triple_left_child,
		&pavlut_triple_right_child
	};

	pavlut_delete_one_node(&pavlut_triple_tree,
	                       &pavlut_triple_top,
	                       expected,
	                       2);
}

Test(pavlut_delete, delete_one_from_complex)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_zero,
		&pavlut_two,
		&pavlut_three,
		&pavlut_four,
		&pavlut_five,
		&pavlut_six,
		&pavlut_seven,
		&pavlut_eight,
		&pavlut_nine,
		&pavlut_ten,
		&pavlut_eleven,
		&pavlut_twelve
	};

	pavlut_delete_one_node(&pavlut_complex_tree,
	                       &pavlut_one,
	                       expected,
	                       array_nr(expected));
}

Test(pavlut_delete, delete_three_from_complex)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_zero,
		&pavlut_one,
		&pavlut_two,
		&pavlut_four,
		&pavlut_five,
		&pavlut_six,
		&pavlut_seven,
		&pavlut_eight,
		&pavlut_nine,
		&pavlut_ten,
		&pavlut_eleven,
		&pavlut_twelve
	};

	pavlut_delete_one_node(&pavlut_complex_tree,
	                       &pavlut_three,
	                       expected,
	                       array_nr(expected));
}

Test(pavlut_delete, delete_five_from_complex)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_zero,
		&pavlut_one,
		&pavlut_two,
		&pavlut_three,
		&pavlut_four,
		&pavlut_six,
		&pavlut_seven,
		&pavlut_eight,
		&pavlut_nine,
		&pavlut_ten,
		&pavlut_eleven,
		&pavlut_twelve
	};

	pavlut_delete_one_node(&pavlut_complex_tree,
	                       &pavlut_five,
	                       expected,
	                       array_nr(expected));
}

Test(pavlut_delete, delete_seven_from_complex)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_zero,
		&pavlut_one,
		&pavlut_two,
		&pavlut_three,
		&pavlut_four,
		&pavlut_five,
		&pavlut_six,
		&pavlut_eight,
		&pavlut_nine,
		&pavlut_ten,
		&pavlut_eleven,
		&pavlut_twelve
	};

	pavlut_delete_one_node(&pavlut_complex_tree,
	                       &pavlut_seven,
	                       expected,
	                       array_nr(expected));
}

Test(pavlut_delete, delete_nine_from_complex)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_zero,
		&pavlut_one,
		&pavlut_two,
		&pavlut_three,
		&pavlut_four,
		&pavlut_five,
		&pavlut_six,
		&pavlut_seven,
		&pavlut_eight,
		&pavlut_ten,
		&pavlut_eleven,
		&pavlut_twelve
	};

	pavlut_delete_one_node(&pavlut_complex_tree,
	                       &pavlut_nine,
	                       expected,
	                       array_nr(expected));
}

Test(pavlut_delete, delete_eleven_from_complex)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_zero,
		&pavlut_one,
		&pavlut_two,
		&pavlut_three,
		&pavlut_four,
		&pavlut_five,
		&pavlut_six,
		&pavlut_seven,
		&pavlut_eight,
		&pavlut_nine,
		&pavlut_ten,
		&pavlut_twelve
	};

	pavlut_delete_one_node(&pavlut_complex_tree,
	                       &pavlut_eleven,
	                       expected,
	                       array_nr(expected));
}

Test(pavlut_delete, delete_zero_from_complex)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_one,
		&pavlut_two,
		&pavlut_three,
		&pavlut_four,
		&pavlut_five,
		&pavlut_six,
		&pavlut_seven,
		&pavlut_eight,
		&pavlut_nine,
		&pavlut_ten,
		&pavlut_eleven,
		&pavlut_twelve
	};

	pavlut_delete_one_node(&pavlut_complex_tree,
	                       &pavlut_zero,
	                       expected,
	                       array_nr(expected));
}

Test(pavlut_delete, delete_four_from_complex)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_zero,
		&pavlut_one,
		&pavlut_two,
		&pavlut_three,
		&pavlut_five,
		&pavlut_six,
		&pavlut_seven,
		&pavlut_eight,
		&pavlut_nine,
		&pavlut_ten,
		&pavlut_eleven,
		&pavlut_twelve
	};

	pavlut_delete_one_node(&pavlut_complex_tree,
	                       &pavlut_four,
	                       expected,
	                       array_nr(expected));
}

Test(pavlut_delete, delete_eight_from_complex)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_zero,
		&pavlut_one,
		&pavlut_two,
		&pavlut_three,
		&pavlut_four,
		&pavlut_five,
		&pavlut_six,
		&pavlut_seven,
		&pavlut_nine,
		&pavlut_ten,
		&pavlut_eleven,
		&pavlut_twelve
	};

	pavlut_delete_one_node(&pavlut_complex_tree,
	                       &pavlut_eight,
	                       expected,
	                       array_nr(expected));
}

Test(pavlut_delete, delete_twelve_from_complex)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_zero,
		&pavlut_one,
		&pavlut_two,
		&pavlut_three,
		&pavlut_four,
		&pavlut_five,
		&pavlut_six,
		&pavlut_seven,
		&pavlut_eight,
		&pavlut_nine,
		&pavlut_ten,
		&pavlut_eleven
	};

	pavlut_delete_one_node(&pavlut_complex_tree,
	                       &pavlut_twelve,
	                       expected,
	                       array_nr(expected));
}

Test(pavlut_delete, delete_two_from_complex)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_zero,
		&pavlut_one,
		&pavlut_three,
		&pavlut_four,
		&pavlut_five,
		&pavlut_six,
		&pavlut_seven,
		&pavlut_eight,
		&pavlut_nine,
		&pavlut_ten,
		&pavlut_eleven,
		&pavlut_twelve
	};

	pavlut_delete_one_node(&pavlut_complex_tree,
	                       &pavlut_two,
	                       expected,
	                       array_nr(expected));
}

Test(pavlut_delete, delete_ten_from_complex)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_zero,
		&pavlut_one,
		&pavlut_two,
		&pavlut_three,
		&pavlut_four,
		&pavlut_five,
		&pavlut_six,
		&pavlut_seven,
		&pavlut_eight,
		&pavlut_nine,
		&pavlut_eleven,
		&pavlut_twelve
	};

	pavlut_delete_one_node(&pavlut_complex_tree,
	                       &pavlut_ten,
	                       expected,
	                       array_nr(expected));
}

Test(pavlut_delete, delete_six_from_complex)
{
	const struct pavlut_node * const expected[] = {
		&pavlut_zero,
		&pavlut_one,
		&pavlut_two,
		&pavlut_three,
		&pavlut_four,
		&pavlut_five,
		&pavlut_seven,
		&pavlut_eight,
		&pavlut_nine,
		&pavlut_ten,
		&pavlut_eleven,
		&pavlut_twelve
	};

	pavlut_delete_one_node(&pavlut_complex_tree,
	                       &pavlut_six,
	                       expected,
	                       array_nr(expected));
}

static void
pavlut_delete_many_nodes(struct pavl_tree                 *tree,
                         struct pavlut_node * const       *nodes,
                         unsigned long                     node_nr,
                         const struct pavlut_node * const *expected,
                         unsigned long                     expect_nr)
{
	unsigned int  n;
	unsigned long cnt;

	cnt = pavl_tree_count(tree);

	for (n = 0; n < node_nr - 1; n++) {
		pavl_delete_node(tree, &nodes[n]->avl);
		pavlut_find_absent_key(tree, nodes[n]->value);
		cr_expect(pavlut_check_tree(tree, cnt - n - 1),
		          "tree property violation\n");
	}

	pavlut_delete_one_node(tree,
	                       nodes[node_nr - 1],
	                       expected,
	                       expect_nr);
}

Test(pavlut_delete, delete_left_left_spine_from_complex)
{
	struct pavlut_node * const       nodes[] = {
		&pavlut_zero,
		&pavlut_two,
		&pavlut_six
	};
	const struct pavlut_node * const expected[] = {
		&pavlut_one,
		&pavlut_three,
		&pavlut_four,
		&pavlut_five,
		&pavlut_seven,
		&pavlut_eight,
		&pavlut_nine,
		&pavlut_ten,
		&pavlut_eleven,
		&pavlut_twelve
	};

	pavlut_delete_many_nodes(&pavlut_complex_tree,
	                         nodes,
	                         array_nr(nodes),
	                         expected,
	                         array_nr(expected));
}

Test(pavlut_delete, delete_left_right_spine_from_complex)
{
	struct pavlut_node * const       nodes[] = {
		&pavlut_four,
		&pavlut_two,
		&pavlut_six
	};
	const struct pavlut_node * const expected[] = {
		&pavlut_zero,
		&pavlut_one,
		&pavlut_three,
		&pavlut_five,
		&pavlut_seven,
		&pavlut_eight,
		&pavlut_nine,
		&pavlut_ten,
		&pavlut_eleven,
		&pavlut_twelve
	};

	pavlut_delete_many_nodes(&pavlut_complex_tree,
	                         nodes,
	                         array_nr(nodes),
	                         expected,
	                         array_nr(expected));
}

Test(pavlut_delete, delete_right_right_spine_from_complex)
{
	struct pavlut_node * const       nodes[] = {
		&pavlut_twelve,
		&pavlut_ten,
		&pavlut_six
	};
	const struct pavlut_node * const expected[] = {
		&pavlut_zero,
		&pavlut_one,
		&pavlut_two,
		&pavlut_three,
		&pavlut_four,
		&pavlut_five,
		&pavlut_seven,
		&pavlut_eight,
		&pavlut_nine,
		&pavlut_eleven
	};

	pavlut_delete_many_nodes(&pavlut_complex_tree,
	                         nodes,
	                         array_nr(nodes),
	                         expected,
	                         array_nr(expected));
}

Test(pavlut_delete, delete_right_left_spine_from_complex)
{
	struct pavlut_node * const       nodes[] = {
		&pavlut_eight,
		&pavlut_ten,
		&pavlut_six
	};
	const struct pavlut_node * const expected[] = {
		&pavlut_zero,
		&pavlut_one,
		&pavlut_two,
		&pavlut_three,
		&pavlut_four,
		&pavlut_five,
		&pavlut_seven,
		&pavlut_nine,
		&pavlut_eleven,
		&pavlut_twelve,
	};

	pavlut_delete_many_nodes(&pavlut_complex_tree,
	                         nodes,
	                         array_nr(nodes),
	                         expected,
	                         array_nr(expected));
}

Test(pavlut_delete, delete_increasing_from_complex)
{
	struct pavlut_node * const       nodes[] = {
		&pavlut_zero,
		&pavlut_one,
		&pavlut_two,
		&pavlut_three,
		&pavlut_four,
		&pavlut_five,
		&pavlut_six,
		&pavlut_seven,
		&pavlut_eight,
		&pavlut_nine,
		&pavlut_ten,
		&pavlut_eleven,
		&pavlut_twelve
	};
	const struct pavlut_node * const expected[] = {
	};

	pavlut_delete_many_nodes(&pavlut_complex_tree,
	                         nodes,
	                         array_nr(nodes),
	                         expected,
	                         array_nr(expected));
}

Test(pavlut_delete, delete_decreasing_from_complex)
{
	struct pavlut_node * const       nodes[] = {
		&pavlut_twelve,
		&pavlut_eleven,
		&pavlut_ten,
		&pavlut_nine,
		&pavlut_eight,
		&pavlut_seven,
		&pavlut_six,
		&pavlut_five,
		&pavlut_four,
		&pavlut_three,
		&pavlut_two,
		&pavlut_one,
		&pavlut_zero
	};
	const struct pavlut_node * const expected[] = {
	};

	pavlut_delete_many_nodes(&pavlut_complex_tree,
	                         nodes,
	                         array_nr(nodes),
	                         expected,
	                         array_nr(expected));
}

void
pavlut_check_clear(struct pavl_tree *tree)
{
	unsigned int                       n;
	const struct pavlut_cleared_nodes *cleared = tree->data;

	pavl_clear_tree(tree);

	cr_expect_eq(cleared->excess,
	             0,
	             "excessive number of released nodes: %u\n",
	             cleared->excess);

	for (n = 0; n < cleared->nr; n++) {
		cr_expect_null(cleared->nodes[n],
		               "%d[%p] node not released\n",
		               cleared->nodes[n]->value,
		               cleared->nodes[n]);
	}

	cr_expect_null(tree->root,
	               "unexpected tree root: not NULL\n");

	cr_expect_eq(pavl_tree_count(tree),
	             0,
	             "unexpected tree count: %lu != 0\n",
	             pavl_tree_count(tree));
}

Test(pavlut_clear, empty)
{
	pavlut_check_clear(&pavlut_empty_tree);
}

Test(pavlut_clear, single)
{
	pavlut_check_clear(&pavlut_single_tree);
}

Test(pavlut_clear, double_left)
{
	pavlut_check_clear(&pavlut_double_left_tree);
}

Test(pavlut_clear, double_right)
{
	pavlut_check_clear(&pavlut_double_right_tree);
}

Test(pavlut_clear, complex)
{
	pavlut_check_clear(&pavlut_complex_tree);
}

struct pavlut_cloned_node {
	struct pavl_node          pavl;
	const struct pavlut_node *orig;
};

struct pavlut_cloned_track {
	unsigned int                      nr;
	const struct pavlut_cloned_node **nodes;
	unsigned int                      excess;
	unsigned int                      err_cnt;
};

static struct pavl_node *
pavlut_clone_node(const struct pavl_node *orig, void *data)
{
	struct pavlut_cloned_track *track = data;
	struct pavlut_cloned_node  *node;

	if (track) {
		if (!track->err_cnt) {
			track->err_cnt--;
			errno = ENOMEM;
			return NULL;
		}

		track->err_cnt--;
	}

	node = malloc(sizeof(*node));
	if (!node)
		return NULL;

	node->orig = (struct pavlut_node *)orig;

	if (track)
		track->nodes[track->nr++] = node;

	return &node->pavl;
}

static int
pavlut_compare_cloned(const struct pavl_node *node,
                      const void             *key,
                      const void             *data __unused)
{
	int k = (unsigned long)key;

	return ((struct pavlut_cloned_node *)node)->orig->value - k;
}

static int
pavlut_compare_cloned_nodes(const struct pavl_node *first,
                            const struct pavl_node *second)
{
	return ((struct pavlut_cloned_node *)first)->orig->value -
	       ((struct pavlut_cloned_node *)second)->orig->value;
}

static void
pavlut_release_cloned_node(struct pavl_node *node, void *data)
{
	const struct pavlut_cloned_node *cloned = (struct pavlut_cloned_node *)
	                                          node;
	struct pavlut_cloned_track      *track = data;
	unsigned int                     n;

	for (n = 0; n < track->nr; n++) {
		if (cloned == track->nodes[n]) {
			track->nodes[n] = NULL;
			return;
		}
	}

	track->excess++;
}

static void
pavlut_check_clone(const struct pavl_tree *orig)
{
	struct pavl_tree  tree;
	int               ret;
	struct pavl_node *src;
	struct pavl_node *dst;
	unsigned long     cnt;

	pavl_init_tree(&tree, pavlut_compare_cloned, NULL, NULL);

	ret = pavl_clone_tree(&tree, orig, pavlut_clone_node, NULL);

	cr_expect_eq(ret,
	             0,
	             "tree cloning failed: %s (%d)\n",
	             strerror(-ret),
	             -ret);
	if (ret)
		return;

	cr_expect_eq(pavl_tree_count(&tree),
	             pavl_tree_count(orig),
	             "unexpected tree count: %lu != %lu\n",
	             pavl_tree_count(&tree),
	             pavl_tree_count(orig));

	if (!pavl_tree_count(&tree))
		cr_expect_null(tree.root,
		               "unexpected tree root: not NULL\n");

	cnt = 0;
	for (src = pavl_iter_first_inorder(orig),
	     dst = pavl_iter_first_inorder(&tree);
	     src && dst;
	     src = pavl_iter_next_inorder(src),
	     dst = pavl_iter_next_inorder(dst)) {
		cr_expect_eq(((struct pavlut_cloned_node *)dst)->orig,
		             (struct pavlut_node *)src,
		             "wrong tree node found: %d[%p] != %d[%p]\n",
		             ((struct pavlut_cloned_node *)dst)->orig->value,
		             ((struct pavlut_cloned_node *)dst)->orig,
		             ((struct pavlut_node *)src)->value,
		             (struct pavlut_node *)src);
		cnt++;
	}

	cr_expect_eq(cnt,
	             pavl_tree_count(orig),
	             "unexpected iteration count: %lu != %lu\n",
	             cnt,
	             pavl_tree_count(orig));

	cr_expect(pavl_check_tree(&tree,
	                          pavl_tree_count(orig),
	                          pavlut_compare_cloned_nodes),
	          "cloned tree property violation\n");
}

Test(pavlut_clone, empty)
{
	pavlut_check_clone(&pavlut_empty_tree);
}

Test(pavlut_clone, single)
{
	pavlut_check_clone(&pavlut_single_tree);
}

Test(pavlut_clone, double_left)
{
	pavlut_check_clone(&pavlut_double_left_tree);
}

Test(pavlut_clone, double_right)
{
	pavlut_check_clone(&pavlut_double_right_tree);
}

Test(pavlut_clone, triple)
{
	pavlut_check_clone(&pavlut_triple_tree);
}

Test(pavlut_clone, complex)
{
	pavlut_check_clone(&pavlut_complex_tree);
}

static void
pavlut_check_clone_error(const struct pavl_tree     *orig,
                         struct pavlut_cloned_track *track,
                         unsigned int                count)
{
	struct pavl_tree tree;
	int              ret;
	unsigned int     n;

	assert(count < pavl_tree_count(orig));

	track->nr = 0;
	track->excess = 0;
	track->err_cnt = count;
	track->nodes = calloc(pavl_tree_count(orig), sizeof(track->nodes[0]));
	cr_assert_not_null(track->nodes, "cloned node tracking alloc failed\n");

	pavl_init_tree(&tree,
	               pavlut_compare_cloned,
	               pavlut_release_cloned_node,
	               track);

	ret = pavl_clone_tree(&tree, orig, pavlut_clone_node, track);

	cr_expect_eq(ret,
	             -ENOMEM,
	             "invalid tree cloning failure return code: %d != %d\n",
	             -ret,
	             ENOMEM);
	if (ret != -ENOMEM)
		goto free;

	cr_expect_eq(track->nr,
	             count,
	             "invalid number of cloned nodes: %u != %u\n",
	             track->nr,
	             count);

	cr_expect_eq(track->excess,
	             0,
	             "excessive number of released nodes: %u\n",
	             track->excess);

	cr_expect_eq((int)track->err_cnt,
	             -1,
	             "excessive number of clone calls: %u\n",
	             count + 1 - track->err_cnt);

	for (n = 0; n < track->nr; n++) {
		cr_expect_null(track->nodes[n],
		               "%d[%p] node not released\n",
		               track->nodes[n]->orig->value,
		               track->nodes[n]);
	}

	cr_expect_eq(pavl_tree_count(&tree),
	             0,
	             "unexpected tree count: %lu != 0\n",
	             pavl_tree_count(&tree));

	cr_expect_null(tree.root, "unexpected tree root: not NULL\n");

free:
	free(track->nodes);
}

Test(pavlut_clone, error)
{
	struct pavlut_cloned_track track;
	unsigned int               err;

	for (err = 0; err < pavl_tree_count(&pavlut_complex_tree); err++)
		pavlut_check_clone_error(&pavlut_complex_tree, &track, err);
}

static struct pavl_node *
pavlut_get_node_byid(unsigned int index, const void *keys)
{
	return ((struct pavl_node **)keys)[index];
}

static void
pavlut_check_load_from_sorted(struct pavl_node * const *expected,
                              unsigned long             nr)
{
	unsigned long     cnt;
	struct pavl_node *node;

	pavl_load_tree_from_sorted(&pavlut_empty_tree,
	                           expected,
	                           nr,
	                           pavlut_get_node_byid);

	cr_expect(pavlut_check_tree(&pavlut_empty_tree, nr),
	          "tree property violation\n");

	cnt = 0;
	pavl_walk_forward_inorder(&pavlut_empty_tree, node) {
		const struct pavlut_node *curr = (struct pavlut_node *)node;
		const struct pavlut_node *ref = (struct pavlut_node *)
		                                expected[cnt];

		cr_expect_eq(curr,
		             ref,
		             "unexpected tree iteration node: "
		             "%d[%p] != %d[%p]\n",
		             curr->value,
		             &curr->avl,
		             ref->value,
		             &ref->avl);
		if (curr != ref)
			return;

		cnt++;
	}

	cr_expect_eq(cnt,
	             nr,
	             "unexpected tree iteration count: %lu != %lu\n",
	             cnt,
	             nr);
}

Test(pavlut_load_from_sorted, empty)
{
	struct pavl_node * const expected[] = { };

	pavlut_check_load_from_sorted(expected, array_nr(expected));
}

Test(pavlut_load_from_sorted, single)
{
	struct pavl_node * const expected[] = {
		&pavlut_single.avl
	};

	pavlut_check_load_from_sorted(expected, array_nr(expected));
}

Test(pavlut_load_from_sorted, double)
{
	struct pavl_node * const expected[] = {
		&pavlut_double_left_child.avl,
		&pavlut_double_left_top.avl
	};

	pavlut_check_load_from_sorted(expected, array_nr(expected));
}

Test(pavlut_load_from_sorted, triple)
{
	struct pavl_node * const expected[] = {
		&pavlut_triple_left_child.avl,
		&pavlut_triple_top.avl,
		&pavlut_triple_right_child.avl
	};

	pavlut_check_load_from_sorted(expected, array_nr(expected));
}

Test(pavlut_load_from_sorted, complex)
{
	struct pavl_node * const expected[] = {
		&pavlut_zero.avl,
		&pavlut_one.avl,
		&pavlut_two.avl,
		&pavlut_three.avl,
		&pavlut_four.avl,
		&pavlut_five.avl,
		&pavlut_six.avl,
		&pavlut_seven.avl,
		&pavlut_eight.avl,
		&pavlut_nine.avl,
		&pavlut_ten.avl,
		&pavlut_eleven.avl,
		&pavlut_twelve.avl
	};

	pavlut_check_load_from_sorted(expected, array_nr(expected));
}

Test(pavlut_load_from_sorted, perfect_minus_one)
{
	struct pavl_node * const expected[] = {
		&pavlut_zero.avl,
		&pavlut_one.avl,
		&pavlut_two.avl,
		&pavlut_three.avl,
		&pavlut_four.avl,
		&pavlut_five.avl,
	};

	pavlut_check_load_from_sorted(expected, array_nr(expected));
}

Test(pavlut_load_from_sorted, perfect)
{
	struct pavl_node * const expected[] = {
		&pavlut_zero.avl,
		&pavlut_one.avl,
		&pavlut_two.avl,
		&pavlut_three.avl,
		&pavlut_four.avl,
		&pavlut_five.avl,
		&pavlut_six.avl,
	};

	pavlut_check_load_from_sorted(expected, array_nr(expected));
}

Test(pavlut_load_from_sorted, perfect_plus_one)
{
	struct pavl_node * const expected[] = {
		&pavlut_zero.avl,
		&pavlut_one.avl,
		&pavlut_two.avl,
		&pavlut_three.avl,
		&pavlut_four.avl,
		&pavlut_five.avl,
		&pavlut_six.avl,
		&pavlut_seven.avl,
	};

	pavlut_check_load_from_sorted(expected, array_nr(expected));
}
	//pavlut_print_tree(&pavlut_empty_tree);
