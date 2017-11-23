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

#include "lcrs.h"
#include <cute/cute.h>

static CUTE_PNP_SUITE(lcrsut, NULL);

#define LCRSUT_INIT_NODE(_sibling, _youngest)  \
	{                                      \
		.lcrs_sibling = _sibling,      \
		.lcrs_youngest = _youngest     \
	}

CUTE_PNP_TEST(lcrsut_init, &lcrsut)
{
	struct lcrs_node root;

	lcrs_init_node(&root);
	cute_ensure(!lcrs_istail_node(&root));
	cute_ensure(lcrs_eldest_sibling(&root) == &root);
	cute_ensure(lcrs_isroot_node(&root));
}

static void lcrsut_check(const struct lcrs_node **nodes, unsigned int count)
{
	const struct lcrs_node *queue[count];
	unsigned int            head = 0;
	unsigned int            busy = 1;
	unsigned int            idx = 0;

	cute_ensure(lcrs_isroot_node(nodes[0]));

	queue[0] = nodes[0];
	while (busy) {
		const struct lcrs_node *node = queue[head];
		struct lcrs_node       *child;

		head = (head + 1) % count;
		busy--;

		cute_ensure(idx < count);
		cute_ensure(node == nodes[idx]);
		idx++;

		child = node->lcrs_youngest;
		if (!child)
			continue;

		cute_ensure(lcrs_node_has_child(node));

		while (!lcrs_istail_node(child)) {
			cute_ensure(lcrs_parent_node(child) == node);

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
		LCRSUT_INIT_NODE(lcrs_mktail_node(NULL), NULL)
	};
	const struct lcrs_node *check[] = {
		&nodes[0]
	};

	lcrsut_check(check, array_nr(check));
}

CUTE_PNP_TEST(lcrsut_single_child, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail_node(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[0]), NULL)
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
		LCRSUT_INIT_NODE(lcrs_mktail_node(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], NULL),
		LCRSUT_INIT_NODE(&nodes[3], NULL),
		LCRSUT_INIT_NODE(&nodes[4], NULL),
		LCRSUT_INIT_NODE(&nodes[5], NULL),
		LCRSUT_INIT_NODE(&nodes[6], NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[0]), NULL)
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
	cute_ensure(lcrs_previous_sibling(&nodes[2], &nodes[1]) == &nodes[1]);
	cute_ensure(lcrs_previous_sibling(&nodes[3], &nodes[1]) == &nodes[2]);
	cute_ensure(lcrs_previous_sibling(&nodes[4], &nodes[2]) == &nodes[3]);
	cute_ensure(lcrs_previous_sibling(&nodes[5], &nodes[4]) == &nodes[4]);
	cute_ensure(lcrs_previous_sibling(&nodes[6], &nodes[3]) == &nodes[5]);
}

CUTE_PNP_TEST(lcrsut_swap_2node, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail_node(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[0]), NULL)
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
	lcrs_swap_down_node(&nodes[0], &nodes[1]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_2level_3node_youngest, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail_node(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[0]), NULL)
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
	lcrs_swap_down_node(&nodes[0], &nodes[1]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_2level_3node_eldest, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail_node(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[0]), NULL)
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
	lcrs_swap_down_node(&nodes[0], &nodes[2]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_2level_4node_youngest, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail_node(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], NULL),
		LCRSUT_INIT_NODE(&nodes[3], NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[0]), NULL)
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
	lcrs_swap_down_node(&nodes[0], &nodes[1]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_2level_4node_middle, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail_node(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], NULL),
		LCRSUT_INIT_NODE(&nodes[3], NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[0]), NULL)
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
	lcrs_swap_down_node(&nodes[0], &nodes[2]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_2level_4node_eldest, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail_node(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], NULL),
		LCRSUT_INIT_NODE(&nodes[3], NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[0]), NULL)
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
	lcrs_swap_down_node(&nodes[0], &nodes[3]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_3level_youngest_1gchild, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail_node(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], &nodes[4]),
		LCRSUT_INIT_NODE(&nodes[3], NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[0]), NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[1]), NULL)
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
	lcrs_swap_down_node(&nodes[0], &nodes[1]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_3level_youngest_2gchild, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail_node(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], &nodes[4]),
		LCRSUT_INIT_NODE(&nodes[3], NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[0]), NULL),
		LCRSUT_INIT_NODE(&nodes[5], NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[1]), NULL)
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
	lcrs_swap_down_node(&nodes[0], &nodes[1]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_3level_youngest_3grand_child, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail_node(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], &nodes[4]),
		LCRSUT_INIT_NODE(&nodes[3], NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[0]), NULL),
		LCRSUT_INIT_NODE(&nodes[5], NULL),
		LCRSUT_INIT_NODE(&nodes[6], NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[1]), NULL)
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
	lcrs_swap_down_node(&nodes[0], &nodes[1]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_3level_middle_1gchild, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail_node(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], NULL),
		LCRSUT_INIT_NODE(&nodes[3], &nodes[4]),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[0]), NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[2]), NULL)
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
	lcrs_swap_down_node(&nodes[0], &nodes[2]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_3level_middle_2gchild, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail_node(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], &nodes[4]),
		LCRSUT_INIT_NODE(&nodes[3], &nodes[5]),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[0]), NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[1]), NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[2]), NULL)
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
	lcrs_swap_down_node(&nodes[0], &nodes[2]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_3level_middle_3gchild, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail_node(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], &nodes[4]),
		LCRSUT_INIT_NODE(&nodes[3], &nodes[5]),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[0]), NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[1]), NULL),
		LCRSUT_INIT_NODE(&nodes[6], NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[2]), NULL)
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
	lcrs_swap_down_node(&nodes[0], &nodes[2]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_3level_eldest_2gchild, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail_node(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], &nodes[4]),
		LCRSUT_INIT_NODE(&nodes[3], NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[0]), &nodes[5]),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[1]), NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[3]), NULL)
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
	lcrs_swap_down_node(&nodes[0], &nodes[3]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_3level_eldest_3gchild, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail_node(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], &nodes[4]),
		LCRSUT_INIT_NODE(&nodes[3], NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[0]), &nodes[5]),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[1]), NULL),
		LCRSUT_INIT_NODE(&nodes[6], NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[3]), NULL)
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
	lcrs_swap_down_node(&nodes[0], &nodes[3]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_3level_parented_0gchild, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail_node(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[0]), &nodes[2]),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[1]), NULL),
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
	lcrs_swap_down_node(&nodes[1], &nodes[2]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_4level_parented_1gchild, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail_node(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[0]), &nodes[2]),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[1]), &nodes[3]),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[2]), NULL)
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
	lcrs_swap_down_node(&nodes[1], &nodes[2]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_4level_parented_2gchild, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail_node(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[0]), &nodes[2]),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[1]), &nodes[3]),
		LCRSUT_INIT_NODE(&nodes[4], NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[2]), NULL)
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
	lcrs_swap_down_node(&nodes[1], &nodes[2]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_4level_parented_5node, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail_node(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], &nodes[3]),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[0]), NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[1]), &nodes[4]),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[3]), NULL)
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
	lcrs_swap_down_node(&nodes[1], &nodes[3]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_3level_parented_4node_0gchild, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail_node(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], &nodes[3]),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[0]), NULL),
		LCRSUT_INIT_NODE(&nodes[4], NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[1]), NULL)
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
	lcrs_swap_down_node(&nodes[1], &nodes[3]);
	lcrsut_check(post, array_nr(post));

}

CUTE_PNP_TEST(lcrsut_swap_4level_parented_7node, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail_node(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], &nodes[3]),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[0]), NULL),
		LCRSUT_INIT_NODE(&nodes[4], &nodes[5]),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[1]), &nodes[6]),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[3]), NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[4]), NULL)
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
	lcrs_swap_down_node(&nodes[1], &nodes[3]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_4level_parented_7node_eldest, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail_node(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], &nodes[3]),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[0]), NULL),
		LCRSUT_INIT_NODE(&nodes[4], &nodes[5]),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[1]), &nodes[6]),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[3]), NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[4]), NULL)
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
	lcrs_swap_down_node(&nodes[1], &nodes[4]);
	lcrsut_check(post, array_nr(post));
}

CUTE_PNP_TEST(lcrsut_swap_4level_parented_7node_middle, &lcrsut)
{
	struct lcrs_node        nodes[] = {
		LCRSUT_INIT_NODE(lcrs_mktail_node(NULL), &nodes[1]),
		LCRSUT_INIT_NODE(&nodes[2], &nodes[3]),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[0]), NULL),
		LCRSUT_INIT_NODE(&nodes[4], NULL),
		LCRSUT_INIT_NODE(&nodes[5], &nodes[6]),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[1]), &nodes[8]),
		LCRSUT_INIT_NODE(&nodes[7], NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[4]), NULL),
		LCRSUT_INIT_NODE(lcrs_mktail_node(&nodes[5]), NULL)
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
	lcrs_swap_down_node(&nodes[1], &nodes[4]);
	lcrsut_check(post, array_nr(post));
}
