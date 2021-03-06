/**
 * @file      lcrs_ut.c
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      22 Nov 2017
 * @copyright GNU Public License v3
 *
 * Left child right sibling unit tests
 *
 * @defgroup lcrsut Left child right sibling unit tests
 *
 * This file is part of Karn
 *
 * Copyright (C) 2017 Grégor Boirie <gregor.boirie@free.fr>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <karn/lcrs.h>
#include <cute/cute.h>

static CUTE_PNP_SUITE(lcrsut, NULL);

#define LCRSUT_INIT_NODE(_sibling, _youngest)  \
	{                                      \
		.lcrs_sibling = _sibling,      \
		.lcrs_youngest = _youngest     \
	}

static inline struct lcrs_node *
lcrsut_nochild(const struct lcrs_node *node)
{
	return lcrs_mktail(node);
}

CUTE_PNP_TEST(lcrsut_init, &lcrsut)
{
	struct lcrs_node root;

	lcrs_init(&root);
	cute_ensure(!lcrs_istail(&root));
	cute_ensure(lcrs_eldest(&root) == &root);
	cute_ensure(!lcrs_has_parent(&root));
}

static void lcrsut_check(const struct lcrs_node **nodes, unsigned int count)
{
	const struct lcrs_node *queue[count];
	unsigned int            head = 0;
	unsigned int            busy = 1;
	unsigned int            idx = 0;

	cute_ensure(!lcrs_has_parent(nodes[0]));

	queue[0] = nodes[0];
	while (busy) {
		const struct lcrs_node *node = queue[head];
		struct lcrs_node       *child;

		head = (head + 1) % count;
		busy--;

		cute_ensure(idx < count);
		cute_ensure(node == nodes[idx]);
		idx++;

		if (!lcrs_has_child(node))
			continue;

		child = node->lcrs_youngest;

		while (!lcrs_istail(child)) {
			cute_ensure(lcrs_parent(child) == node);

			queue[(head + busy) % count] = child;
			busy++;
			cute_ensure(busy < count);

			child = child->lcrs_sibling;
		}
	}

	cute_ensure(idx == count);
}

CUTE_PNP_TEST(lcrsut_single_root, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail(NULL),
		                 lcrsut_nochild(&nodes[0]))
	};
	const struct lcrs_node *check[] = {
		&nodes[0]
	};

	lcrsut_check(check, array_nr(check));
}

CUTE_PNP_TEST(lcrsut_single_child, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[0]),
		                 lcrsut_nochild(&nodes[1]))
	};
	const struct lcrs_node *check[] = {
		&nodes[0],
		&nodes[1]
	};

	lcrsut_check(check, array_nr(check));
}

CUTE_PNP_TEST(lcrsut_2level_tree, &lcrsut)
{
	struct lcrs_node nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], lcrsut_nochild(&nodes[1])),
		LCRSUT_INIT_NODE(&nodes[3], lcrsut_nochild(&nodes[2])),
		LCRSUT_INIT_NODE(&nodes[4], lcrsut_nochild(&nodes[3])),
		LCRSUT_INIT_NODE(&nodes[5], lcrsut_nochild(&nodes[4])),
		LCRSUT_INIT_NODE(&nodes[6], lcrsut_nochild(&nodes[5])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[0]),
		                 lcrsut_nochild(&nodes[6]))
	};
	const struct lcrs_node *check[] = {
		&nodes[0],
		&nodes[1],
		&nodes[2],
		&nodes[3],
		&nodes[4],
		&nodes[5],
		&nodes[6]
	};

	lcrsut_check(check, array_nr(check));
	cute_ensure(lcrs_previous(&nodes[2], &nodes[1]) == &nodes[1]);
	cute_ensure(lcrs_previous(&nodes[3], &nodes[1]) == &nodes[2]);
	cute_ensure(lcrs_previous(&nodes[4], &nodes[2]) == &nodes[3]);
	cute_ensure(lcrs_previous(&nodes[5], &nodes[4]) == &nodes[4]);
	cute_ensure(lcrs_previous(&nodes[6], &nodes[3]) == &nodes[5]);
}

CUTE_PNP_TEST(lcrsut_swap_2node, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[0]),
		                 lcrsut_nochild(&nodes[1]))
	};
	const struct lcrs_node *pre[] = {
		&nodes[0],
		&nodes[1]
	};
	const struct lcrs_node *post[] = {
		&nodes[1],
		&nodes[0]
	};

	lcrsut_check(pre, array_nr(pre));
	lcrs_swap_down(&nodes[0], &nodes[1]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_2level_3node_youngest, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], lcrsut_nochild(&nodes[1])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[0]),
		                 lcrsut_nochild(&nodes[2]))
	};
	const struct lcrs_node *pre[] = {
		&nodes[0],
		&nodes[1],
		&nodes[2]
	};
	const struct lcrs_node *post[] = {
		&nodes[1],
		&nodes[0],
		&nodes[2]
	};

	lcrsut_check(pre, array_nr(pre));
	lcrs_swap_down(&nodes[0], &nodes[1]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_2level_3node_eldest, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], lcrsut_nochild(&nodes[1])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[0]),
		                 lcrsut_nochild(&nodes[2]))
	};
	const struct lcrs_node *pre[] = {
		&nodes[0],
		&nodes[1],
		&nodes[2]
	};
	const struct lcrs_node *post[] = {
		&nodes[2],
		&nodes[1],
		&nodes[0]
	};

	lcrsut_check(pre, array_nr(pre));
	lcrs_swap_down(&nodes[0], &nodes[2]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_2level_4node_youngest, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], lcrsut_nochild(&nodes[1])),
		LCRSUT_INIT_NODE(&nodes[3], lcrsut_nochild(&nodes[2])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[0]),
		                 lcrsut_nochild(&nodes[3]))
	};
	const struct lcrs_node *pre[] = {
		&nodes[0],
		&nodes[1],
		&nodes[2],
		&nodes[3]
	};
	const struct lcrs_node *post[] = {
		&nodes[1],
		&nodes[0],
		&nodes[2],
		&nodes[3]
	};

	lcrsut_check(pre, array_nr(pre));
	lcrs_swap_down(&nodes[0], &nodes[1]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_2level_4node_middle, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], lcrsut_nochild(&nodes[1])),
		LCRSUT_INIT_NODE(&nodes[3], lcrsut_nochild(&nodes[2])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[0]),
		                 lcrsut_nochild(&nodes[3]))
	};
	const struct lcrs_node *pre[] = {
		&nodes[0],
		&nodes[1],
		&nodes[2],
		&nodes[3]
	};
	const struct lcrs_node *post[] = {
		&nodes[2],
		&nodes[1],
		&nodes[0],
		&nodes[3]
	};

	lcrsut_check(pre, array_nr(pre));
	lcrs_swap_down(&nodes[0], &nodes[2]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_2level_4node_eldest, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], lcrsut_nochild(&nodes[1])),
		LCRSUT_INIT_NODE(&nodes[3], lcrsut_nochild(&nodes[2])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[0]),
		                 lcrsut_nochild(&nodes[3]))
	};
	const struct lcrs_node *pre[] = {
		&nodes[0],
		&nodes[1],
		&nodes[2],
		&nodes[3]
	};
	const struct lcrs_node *post[] = {
		&nodes[3],
		&nodes[1],
		&nodes[2],
		&nodes[0]
	};

	lcrsut_check(pre, array_nr(pre));
	lcrs_swap_down(&nodes[0], &nodes[3]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_3level_youngest_1gchild, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], &nodes[4]),
		LCRSUT_INIT_NODE(&nodes[3], lcrsut_nochild(&nodes[2])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[0]),
		                 lcrsut_nochild(&nodes[3])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[1]),
		                 lcrsut_nochild(&nodes[4]))
	};
	const struct lcrs_node *pre[] = {
		&nodes[0],
		&nodes[1],
		&nodes[2],
		&nodes[3],
		&nodes[4]
	};
	const struct lcrs_node *post[] = {
		&nodes[1],
		&nodes[0],
		&nodes[2],
		&nodes[3],
		&nodes[4]
	};

	lcrsut_check(pre, array_nr(pre));
	lcrs_swap_down(&nodes[0], &nodes[1]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_3level_youngest_2gchild, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], &nodes[4]),
		LCRSUT_INIT_NODE(&nodes[3], lcrsut_nochild(&nodes[2])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[0]),
		                 lcrsut_nochild(&nodes[3])),
		LCRSUT_INIT_NODE(&nodes[5], lcrsut_nochild(&nodes[4])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[1]),
		                 lcrsut_nochild(&nodes[5]))
	};
	const struct lcrs_node *pre[] = {
		&nodes[0],
		&nodes[1],
		&nodes[2],
		&nodes[3],
		&nodes[4],
		&nodes[5]
	};
	const struct lcrs_node *post[] = {
		&nodes[1],
		&nodes[0],
		&nodes[2],
		&nodes[3],
		&nodes[4],
		&nodes[5]
	};

	lcrsut_check(pre, array_nr(pre));
	lcrs_swap_down(&nodes[0], &nodes[1]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_3level_youngest_3grand_child, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], &nodes[4]),
		LCRSUT_INIT_NODE(&nodes[3], lcrsut_nochild(&nodes[2])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[0]),
		                 lcrsut_nochild(&nodes[3])),
		LCRSUT_INIT_NODE(&nodes[5], lcrsut_nochild(&nodes[4])),
		LCRSUT_INIT_NODE(&nodes[6], lcrsut_nochild(&nodes[5])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[1]),
		                 lcrsut_nochild(&nodes[6]))
	};
	const struct lcrs_node *pre[] = {
		&nodes[0],
		&nodes[1],
		&nodes[2],
		&nodes[3],
		&nodes[4],
		&nodes[5],
		&nodes[6]
	};
	const struct lcrs_node *post[] = {
		&nodes[1],
		&nodes[0],
		&nodes[2],
		&nodes[3],
		&nodes[4],
		&nodes[5],
		&nodes[6]
	};

	lcrsut_check(pre, array_nr(pre));
	lcrs_swap_down(&nodes[0], &nodes[1]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_3level_middle_1gchild, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], lcrsut_nochild(&nodes[1])),
		LCRSUT_INIT_NODE(&nodes[3], &nodes[4]),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[0]),
		                 lcrsut_nochild(&nodes[3])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[2]),
		                 lcrsut_nochild(&nodes[4]))
	};
	const struct lcrs_node *pre[] = {
		&nodes[0],
		&nodes[1],
		&nodes[2],
		&nodes[3],
		&nodes[4]
	};
	const struct lcrs_node *post[] = {
		&nodes[2],
		&nodes[1],
		&nodes[0],
		&nodes[3],
		&nodes[4]
	};

	lcrsut_check(pre, array_nr(pre));
	lcrs_swap_down(&nodes[0], &nodes[2]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_3level_middle_2gchild, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], &nodes[4]),
		LCRSUT_INIT_NODE(&nodes[3], &nodes[5]),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[0]),
		                 lcrsut_nochild(&nodes[3])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[1]),
		                 lcrsut_nochild(&nodes[4])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[2]),
		                 lcrsut_nochild(&nodes[5]))
	};
	const struct lcrs_node *pre[] = {
		&nodes[0],
		&nodes[1],
		&nodes[2],
		&nodes[3],
		&nodes[4],
		&nodes[5]
	};
	const struct lcrs_node *post[] = {
		&nodes[2],
		&nodes[1],
		&nodes[0],
		&nodes[3],
		&nodes[4],
		&nodes[5]
	};

	lcrsut_check(pre, array_nr(pre));
	lcrs_swap_down(&nodes[0], &nodes[2]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_3level_middle_3gchild, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], &nodes[4]),
		LCRSUT_INIT_NODE(&nodes[3], &nodes[5]),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[0]),
		                 lcrsut_nochild(&nodes[3])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[1]),
		                 lcrsut_nochild(&nodes[4])),
		LCRSUT_INIT_NODE(&nodes[6], lcrsut_nochild(&nodes[5])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[2]),
		                 lcrsut_nochild(&nodes[6]))
	};
	const struct lcrs_node *pre[] = {
		&nodes[0],
		&nodes[1],
		&nodes[2],
		&nodes[3],
		&nodes[4],
		&nodes[5],
		&nodes[6]
	};
	const struct lcrs_node *post[] = {
		&nodes[2],
		&nodes[1],
		&nodes[0],
		&nodes[3],
		&nodes[4],
		&nodes[5],
		&nodes[6]
	};

	lcrsut_check(pre, array_nr(pre));
	lcrs_swap_down(&nodes[0], &nodes[2]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_3level_eldest_2gchild, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], &nodes[4]),
		LCRSUT_INIT_NODE(&nodes[3],
		                 lcrsut_nochild(&nodes[2])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[0]), &nodes[5]),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[1]),
		                 lcrsut_nochild(&nodes[4])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[3]),
		                 lcrsut_nochild(&nodes[5]))
	};
	const struct lcrs_node *pre[] = {
		&nodes[0],
		&nodes[1],
		&nodes[2],
		&nodes[3],
		&nodes[4],
		&nodes[5]
	};
	const struct lcrs_node *post[] = {
		&nodes[3],
		&nodes[1],
		&nodes[2],
		&nodes[0],
		&nodes[4],
		&nodes[5]
	};

	lcrsut_check(pre, array_nr(pre));
	lcrs_swap_down(&nodes[0], &nodes[3]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_3level_eldest_3gchild, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], &nodes[4]),
		LCRSUT_INIT_NODE(&nodes[3], lcrsut_nochild(&nodes[2])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[0]), &nodes[5]),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[1]),
		                 lcrsut_nochild(&nodes[4])),
		LCRSUT_INIT_NODE(&nodes[6], lcrsut_nochild(&nodes[5])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[3]),
		                 lcrsut_nochild(&nodes[6]))
	};
	const struct lcrs_node *pre[] = {
		&nodes[0],
		&nodes[1],
		&nodes[2],
		&nodes[3],
		&nodes[4],
		&nodes[5],
		&nodes[6]
	};
	const struct lcrs_node *post[] = {
		&nodes[3],
		&nodes[1],
		&nodes[2],
		&nodes[0],
		&nodes[4],
		&nodes[5],
		&nodes[6]
	};

	lcrsut_check(pre, array_nr(pre));
	lcrs_swap_down(&nodes[0], &nodes[3]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_3level_parented_0gchild, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[0]), &nodes[2]),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[1]),
		                 lcrsut_nochild(&nodes[2]))
	};
	const struct lcrs_node *pre[] = {
		&nodes[0],
		&nodes[1],
		&nodes[2]
	};
	const struct lcrs_node *post[] = {
		&nodes[0],
		&nodes[2],
		&nodes[1]
	};

	lcrsut_check(pre, array_nr(pre));
	lcrs_swap_down(&nodes[1], &nodes[2]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_4level_parented_1gchild, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[0]), &nodes[2]),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[1]), &nodes[3]),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[2]),
		                 lcrsut_nochild(&nodes[3]))
	};
	const struct lcrs_node *pre[] = {
		&nodes[0],
		&nodes[1],
		&nodes[2],
		&nodes[3]
	};
	const struct lcrs_node *post[] = {
		&nodes[0],
		&nodes[2],
		&nodes[1],
		&nodes[3]
	};

	lcrsut_check(pre, array_nr(pre));
	lcrs_swap_down(&nodes[1], &nodes[2]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_4level_parented_2gchild, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[0]), &nodes[2]),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[1]), &nodes[3]),
		LCRSUT_INIT_NODE(&nodes[4],
		                 lcrsut_nochild(&nodes[3])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[2]),
		                 lcrsut_nochild(&nodes[4]))
	};
	const struct lcrs_node *pre[] = {
		&nodes[0],
		&nodes[1],
		&nodes[2],
		&nodes[3],
		&nodes[4]
	};
	const struct lcrs_node *post[] = {
		&nodes[0],
		&nodes[2],
		&nodes[1],
		&nodes[3],
		&nodes[4]
	};

	lcrsut_check(pre, array_nr(pre));
	lcrs_swap_down(&nodes[1], &nodes[2]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_4level_parented_5node, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], &nodes[3]),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[0]),
		                 lcrsut_nochild(&nodes[2])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[1]), &nodes[4]),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[3]),
		                 lcrsut_nochild(&nodes[4]))
	};
	const struct lcrs_node *pre[] = {
		&nodes[0],
		&nodes[1],
		&nodes[2],
		&nodes[3],
		&nodes[4]
	};
	const struct lcrs_node *post[] = {
		&nodes[0],
		&nodes[3],
		&nodes[2],
		&nodes[1],
		&nodes[4]
	};

	lcrsut_check(pre, array_nr(pre));
	lcrs_swap_down(&nodes[1], &nodes[3]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_3level_parented_4node_0gchild, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], &nodes[3]),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[0]),
		                 lcrsut_nochild(&nodes[2])),
		LCRSUT_INIT_NODE(&nodes[4], lcrsut_nochild(&nodes[3])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[1]),
		                 lcrsut_nochild(&nodes[4]))
	};
	const struct lcrs_node *pre[] = {
		&nodes[0],
		&nodes[1],
		&nodes[2],
		&nodes[3],
		&nodes[4]
	};
	const struct lcrs_node *post[] = {
		&nodes[0],
		&nodes[3],
		&nodes[2],
		&nodes[1],
		&nodes[4]
	};

	lcrsut_check(pre, array_nr(pre));
	lcrs_swap_down(&nodes[1], &nodes[3]);
	lcrsut_check(post, array_nr(post));

}

CUTE_PNP_TEST(lcrsut_swap_4level_parented_7node, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], &nodes[3]),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[0]),
		                 lcrsut_nochild(&nodes[2])),
		LCRSUT_INIT_NODE(&nodes[4], &nodes[5]),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[1]), &nodes[6]),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[3]),
		                 lcrsut_nochild(&nodes[5])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[4]),
		                 lcrsut_nochild(&nodes[6]))
	};
	const struct lcrs_node *pre[] = {
		&nodes[0],
		&nodes[1],
		&nodes[2],
		&nodes[3],
		&nodes[4],
		&nodes[5],
		&nodes[6],
	};
	const struct lcrs_node *post[] = {
		&nodes[0],
		&nodes[3],
		&nodes[2],
		&nodes[1],
		&nodes[4],
		&nodes[5],
		&nodes[6]
	};

	lcrsut_check(pre, array_nr(pre));
	lcrs_swap_down(&nodes[1], &nodes[3]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_4level_parented_7node_eldest, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], &nodes[3]),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[0]),
		                 lcrsut_nochild(&nodes[2])),
		LCRSUT_INIT_NODE(&nodes[4], &nodes[5]),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[1]), &nodes[6]),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[3]),
		                 lcrsut_nochild(&nodes[5])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[4]),
		                 lcrsut_nochild(&nodes[6]))
	};
	const struct lcrs_node *pre[] = {
		&nodes[0],
		&nodes[1],
		&nodes[2],
		&nodes[3],
		&nodes[4],
		&nodes[5],
		&nodes[6]
	};
	const struct lcrs_node *post[] = {
		&nodes[0],
		&nodes[4],
		&nodes[2],
		&nodes[3],
		&nodes[1],
		&nodes[5],
		&nodes[6]
	};

	lcrsut_check(pre, array_nr(pre));
	lcrs_swap_down(&nodes[1], &nodes[4]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_4level_parented_7node_middle, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], &nodes[3]),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[0]),
		                 lcrsut_nochild(&nodes[2])),
		LCRSUT_INIT_NODE(&nodes[4], lcrsut_nochild(&nodes[3])),
		LCRSUT_INIT_NODE(&nodes[5], &nodes[6]),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[1]), &nodes[8]),
		LCRSUT_INIT_NODE(&nodes[7], lcrsut_nochild(&nodes[6])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[4]),
		                 lcrsut_nochild(&nodes[7])),
		LCRSUT_INIT_NODE(lcrs_mktail(&nodes[5]),
		                 lcrsut_nochild(&nodes[8]))
	};
	const struct lcrs_node *pre[] = {
		&nodes[0],
		&nodes[1],
		&nodes[2],
		&nodes[3],
		&nodes[4],
		&nodes[5],
		&nodes[6],
		&nodes[7],
		&nodes[8]
	};
	const struct lcrs_node *post[] = {
		&nodes[0],
		&nodes[4],
		&nodes[2],
		&nodes[3],
		&nodes[1],
		&nodes[5],
		&nodes[6],
		&nodes[7],
		&nodes[8]
	};

	lcrsut_check(pre, array_nr(pre));
	lcrs_swap_down(&nodes[1], &nodes[4]);
	lcrsut_check(post, array_nr(post));
}
