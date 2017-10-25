/**
 * @file      dlist_ut.c
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      31 Aug 2017
 * @copyright GNU Public License v3
 *
 * Doubly linked list unit tests implementation
 *
 * @defgroup dlistut Doubly linked list unit tests
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

#include "dlist.h"
#include <cute/cute.h>
#include <string.h>

static struct dlist_node dlistut_list;

static void dlistut_setup(void)
{
	dlist_init(&dlistut_list);
}

static CUTE_PNP_FIXTURED_SUITE(dlistut, NULL, dlistut_setup, NULL);

/**
 * Check an empty dlist is really exposed as empty.
 *
 * @ingroup dlistut
 */
CUTE_PNP_TEST(dlistut_isempty, &dlistut)
{
	cute_ensure(dlist_empty(&dlistut_list) == true);
}

/**
 * Check iteration over an empty dlist
 *
 * @ingroup dlistut
 */
CUTE_PNP_TEST(dlistut_iterate_empty, &dlistut)
{
	struct dlist_node *node;

	dlist_foreach_node(&dlistut_list, node)
		cute_fail("empty dlist cannot be iterated over");
}

/**
 * Check front enqueueing into an empty dlist
 *
 * @ingroup dlistut
 */
CUTE_PNP_TEST(dlistut_nqueue_front_empty, &dlistut)
{
	struct dlist_node  entry;
	struct dlist_node *node;
	unsigned int       cnt = 0;

	dlist_nqueue_front(&dlistut_list, &entry);

	dlist_foreach_node(&dlistut_list, node)
		cnt++;

	cute_ensure(dlist_empty(&dlistut_list) == false);
	cute_ensure(cnt == 1);
	cute_ensure(dlist_next(&dlistut_list) == &entry);
	cute_ensure(dlist_prev(&dlistut_list) == &entry);
}

/**
 * Check back enqueueing into an empty dlist
 *
 * @ingroup dlistut
 */
CUTE_PNP_TEST(dlistut_nqueue_back_empty, &dlistut)
{
	struct dlist_node  entry;
	struct dlist_node *node;
	unsigned int       cnt = 0;

	dlist_nqueue_back(&dlistut_list, &entry);

	dlist_foreach_node(&dlistut_list, node)
		cnt++;

	cute_ensure(dlist_empty(&dlistut_list) == false);
	cute_ensure(cnt == 1);
	cute_ensure(dlist_next(&dlistut_list) == &entry);
	cute_ensure(dlist_prev(&dlistut_list) == &entry);
}

CUTE_PNP_TEST(dlistut_nqueue_back, &dlistut)
{
	unsigned int       n;
	struct dlist_node *node;
	struct dlist_node  nodes[5];

	for (n = 0; n < array_nr(nodes); n++)
		dlist_nqueue_back(&dlistut_list, &nodes[n]);

	cute_ensure(dlist_empty(&dlistut_list) == false);

	n = 0;
	dlist_foreach_node(&dlistut_list, node) {
		cute_ensure(node == &nodes[n]);
		n++;
	}

	cute_ensure(n == array_nr(nodes));
}

CUTE_PNP_TEST(dlistut_remove, &dlistut)
{
	unsigned int             n;
	const struct dlist_node *node = (struct dlist_node *)0xdeadbeef;
	struct dlist_node        nodes[3];

	for (n = 0; n < array_nr(nodes); n++)
		dlist_nqueue_back(&dlistut_list, &nodes[n]);

	cute_ensure(dlist_empty(&dlistut_list) == false);

	dlist_remove(&nodes[1]);

	node = dlist_next(&dlistut_list);
	cute_ensure(node == &nodes[0]);

	node = dlist_next(node);
	cute_ensure(node == &nodes[2]);

	node = dlist_next(node);
	cute_ensure(node == &dlistut_list);

	node = (struct dlist_node *)0xdeadbeef;

	node = dlist_prev(&dlistut_list);
	cute_ensure(node == &nodes[2]);

	node = dlist_prev(node);
	cute_ensure(node == &nodes[0]);

	node = dlist_prev(node);
	cute_ensure(node == &dlistut_list);
}

CUTE_PNP_TEST(dlistut_front_fifo, &dlistut)
{
	unsigned int       n;
	struct dlist_node  nodes[5];

	for (n = 0; n < array_nr(nodes); n++)
		dlist_nqueue_front(&dlistut_list, &nodes[n]);

	cute_ensure(dlist_empty(&dlistut_list) == false);

	for (n = 0; n < array_nr(nodes); n++) {
		const struct dlist_node *node = (struct dlist_node *)0xdeadbeef;

		node = dlist_dqueue_back(&dlistut_list);

		cute_ensure(node == &nodes[n]);
	}

	cute_ensure(dlist_empty(&dlistut_list) == true);
}

CUTE_PNP_TEST(dlistut_back_fifo, &dlistut)
{
	unsigned int       n;
	struct dlist_node  nodes[5];

	for (n = 0; n < array_nr(nodes); n++)
		dlist_nqueue_back(&dlistut_list, &nodes[n]);

	cute_ensure(dlist_empty(&dlistut_list) == false);

	for (n = 0; n < array_nr(nodes); n++) {
		const struct dlist_node *node = (struct dlist_node *)0xdeadbeef;

		node = dlist_dqueue_front(&dlistut_list);

		cute_ensure(node == &nodes[n]);
	}

	cute_ensure(dlist_empty(&dlistut_list) == true);
}

CUTE_PNP_TEST(dlistut_front_lifo, &dlistut)
{
	unsigned int       n;
	struct dlist_node  nodes[5];

	for (n = 0; n < array_nr(nodes); n++)
		dlist_nqueue_front(&dlistut_list, &nodes[n]);

	cute_ensure(dlist_empty(&dlistut_list) == false);

	n = array_nr(nodes);
	while (n--) {
		const struct dlist_node *node = (struct dlist_node *)0xdeadbeef;

		node = dlist_dqueue_front(&dlistut_list);

		cute_ensure(node == &nodes[n]);
	}

	cute_ensure(dlist_empty(&dlistut_list) == true);
}

CUTE_PNP_TEST(dlistut_back_lifo, &dlistut)
{
	unsigned int       n;
	struct dlist_node  nodes[5];

	for (n = 0; n < array_nr(nodes); n++)
		dlist_nqueue_back(&dlistut_list, &nodes[n]);

	cute_ensure(dlist_empty(&dlistut_list) == false);

	n = array_nr(nodes);
	while (n--) {
		const struct dlist_node *node = (struct dlist_node *)0xdeadbeef;

		node = dlist_dqueue_back(&dlistut_list);

		cute_ensure(node == &nodes[n]);
	}

	cute_ensure(dlist_empty(&dlistut_list) == true);
}

CUTE_PNP_TEST(dlistut_replace_first, &dlistut)
{
	unsigned int       n;
	struct dlist_node  nodes[3];
	struct dlist_node  replace;

	for (n = 0; n < array_nr(nodes); n++)
		dlist_nqueue_back(&dlistut_list, &nodes[n]);

	cute_ensure(dlist_empty(&dlistut_list) == false);

	dlist_replace(&nodes[0], &replace);

	cute_ensure(dlist_next(&dlistut_list) == &replace);
	cute_ensure(dlist_prev(&dlistut_list) == &nodes[2]);

	cute_ensure(dlist_next(&replace) == &nodes[1]);
	cute_ensure(dlist_prev(&replace) == &dlistut_list);

	cute_ensure(dlist_next(&nodes[1]) == &nodes[2]);
	cute_ensure(dlist_prev(&nodes[1]) == &replace);

	cute_ensure(dlist_next(&nodes[2]) == &dlistut_list);
	cute_ensure(dlist_prev(&nodes[2]) == &nodes[1]);
}

CUTE_PNP_TEST(dlistut_replace_last, &dlistut)
{
	unsigned int       n;
	struct dlist_node  nodes[3];
	struct dlist_node  replace;

	for (n = 0; n < array_nr(nodes); n++)
		dlist_nqueue_back(&dlistut_list, &nodes[n]);

	cute_ensure(dlist_empty(&dlistut_list) == false);

	dlist_replace(&nodes[2], &replace);

	cute_ensure(dlist_next(&dlistut_list) == &nodes[0]);
	cute_ensure(dlist_prev(&dlistut_list) == &replace);

	cute_ensure(dlist_next(&nodes[0]) == &nodes[1]);
	cute_ensure(dlist_prev(&nodes[0]) == &dlistut_list);

	cute_ensure(dlist_next(&nodes[1]) == &replace);
	cute_ensure(dlist_prev(&nodes[1]) == &nodes[0]);

	cute_ensure(dlist_next(&replace) == &dlistut_list);
	cute_ensure(dlist_prev(&replace) == &nodes[1]);
}

CUTE_PNP_TEST(dlistut_replace_middle, &dlistut)
{
	unsigned int       n;
	struct dlist_node  nodes[3];
	struct dlist_node  replace;

	for (n = 0; n < array_nr(nodes); n++)
		dlist_nqueue_back(&dlistut_list, &nodes[n]);

	cute_ensure(dlist_empty(&dlistut_list) == false);

	dlist_replace(&nodes[1], &replace);

	cute_ensure(dlist_next(&dlistut_list) == &nodes[0]);
	cute_ensure(dlist_prev(&dlistut_list) == &nodes[2]);

	cute_ensure(dlist_next(&nodes[0]) == &replace);
	cute_ensure(dlist_prev(&nodes[0]) == &dlistut_list);

	cute_ensure(dlist_next(&replace) == &nodes[2]);
	cute_ensure(dlist_prev(&replace) == &nodes[0]);

	cute_ensure(dlist_next(&nodes[2]) == &dlistut_list);
	cute_ensure(dlist_prev(&nodes[2]) == &replace);
}

CUTE_PNP_TEST(dlistut_replace_single, &dlistut)
{
	struct dlist_node orig;
	struct dlist_node replace;

	dlist_nqueue_back(&dlistut_list, &orig);

	cute_ensure(dlist_empty(&dlistut_list) == false);

	dlist_replace(&orig, &replace);

	cute_ensure(dlist_next(&dlistut_list) == &replace);
	cute_ensure(dlist_prev(&dlistut_list) == &replace);

	cute_ensure(dlist_next(&replace) == &dlistut_list);
	cute_ensure(dlist_prev(&replace) == &dlistut_list);
}

CUTE_PNP_TEST(dlistut_withdraw_empty, &dlistut)
{
	dlist_withdraw(&dlistut_list, &dlistut_list);

	cute_ensure(dlist_empty(&dlistut_list) == true);
	cute_ensure(dlist_next(&dlistut_list) == &dlistut_list);
	cute_ensure(dlist_prev(&dlistut_list) == &dlistut_list);
}

CUTE_PNP_TEST(dlistut_withdraw_single, &dlistut)
{
	unsigned int             n;
	const struct dlist_node *node = (struct dlist_node *)0xdeadbeef;
	struct dlist_node        nodes[3];

	for (n = 0; n < array_nr(nodes); n++)
		dlist_nqueue_back(&dlistut_list, &nodes[n]);

	cute_ensure(dlist_empty(&dlistut_list) == false);

	dlist_withdraw(&nodes[1], &nodes[1]);

	node = dlist_next(&dlistut_list);
	cute_ensure(node == &nodes[0]);

	node = dlist_next(node);
	cute_ensure(node == &nodes[2]);

	node = dlist_next(node);
	cute_ensure(node == &dlistut_list);

	node = (struct dlist_node *)0xdeadbeef;

	node = dlist_prev(&dlistut_list);
	cute_ensure(node == &nodes[2]);

	node = dlist_prev(node);
	cute_ensure(node == &nodes[0]);

	node = dlist_prev(node);
	cute_ensure(node == &dlistut_list);
}

CUTE_PNP_TEST(dlistut_withdraw_multiple, &dlistut)
{
	unsigned int             n;
	const struct dlist_node *node = (struct dlist_node *)0xdeadbeef;
	struct dlist_node        nodes[5];

	for (n = 0; n < array_nr(nodes); n++)
		dlist_nqueue_back(&dlistut_list, &nodes[n]);

	cute_ensure(dlist_empty(&dlistut_list) == false);

	dlist_withdraw(&nodes[1], &nodes[3]);

	node = dlist_next(&dlistut_list);
	cute_ensure(node == &nodes[0]);

	node = dlist_next(node);
	cute_ensure(node == &nodes[4]);

	node = dlist_next(node);
	cute_ensure(node == &dlistut_list);

	node = (struct dlist_node *)0xdeadbeef;

	node = dlist_prev(&dlistut_list);
	cute_ensure(node == &nodes[4]);

	node = dlist_prev(node);
	cute_ensure(node == &nodes[0]);

	node = dlist_prev(node);
	cute_ensure(node == &dlistut_list);
}

CUTE_PNP_TEST(dlistut_embed_single, &dlistut)
{
	unsigned int             n;
	const struct dlist_node *node = (struct dlist_node *)0xdeadbeef;
	struct dlist_node        nodes[3];

	dlist_nqueue_back(&dlistut_list, &nodes[0]);
	dlist_nqueue_back(&dlistut_list, &nodes[2]);

	cute_ensure(dlist_empty(&dlistut_list) == false);

	dlist_embed(&nodes[0], &nodes[1], &nodes[1]);

	n = 0;
	dlist_foreach_node(&dlistut_list, node) {
		cute_ensure(node == &nodes[n]);
		n++;
	}

	cute_ensure(n == array_nr(nodes));
}

CUTE_PNP_TEST(dlistut_embed_multiple, &dlistut)
{
	unsigned int             n;
	const struct dlist_node *node = (struct dlist_node *)0xdeadbeef;
	struct dlist_node        nodes[5];

	dlist_nqueue_back(&dlistut_list, &nodes[0]);
	dlist_nqueue_back(&dlistut_list, &nodes[4]);

	cute_ensure(dlist_empty(&dlistut_list) == false);

	dlist_init(&nodes[1]);
	dlist_append(&nodes[1], &nodes[2]);
	dlist_append(&nodes[2], &nodes[3]);

	dlist_embed(&nodes[0], &nodes[1], &nodes[3]);

	n = 0;
	dlist_foreach_node(&dlistut_list, node) {
		cute_ensure(node == &nodes[n]);
		n++;
	}

	cute_ensure(n == array_nr(nodes));
}

CUTE_PNP_TEST(dlistut_splice, &dlistut)
{
	unsigned int             n;
	const struct dlist_node *node = (struct dlist_node *)0xdeadbeef;
	struct dlist_node        nodes[10];
	struct dlist_node        src = DLIST_INIT(src);

	memset(nodes, 0xde, sizeof(nodes));

	dlist_nqueue_back(&dlistut_list, &nodes[0]);
	dlist_nqueue_back(&dlistut_list, &nodes[1]);
	dlist_nqueue_back(&dlistut_list, &nodes[2]);
	dlist_nqueue_back(&dlistut_list, &nodes[6]);
	dlist_nqueue_back(&dlistut_list, &nodes[7]);
	dlist_nqueue_back(&dlistut_list, &nodes[8]);

	dlist_nqueue_back(&src, &nodes[3]);
	dlist_nqueue_back(&src, &nodes[4]);
	dlist_nqueue_back(&src, &nodes[5]);
	dlist_nqueue_back(&src, &nodes[9]);

	dlist_splice(&nodes[2], &nodes[3], &nodes[5]);
	dlist_splice(&nodes[8], &nodes[9], &nodes[9]);

	n = 0;
	dlist_foreach_node(&dlistut_list, node) {
		cute_ensure(node == &nodes[n]);
		n++;
	}

	cute_ensure(dlist_empty(&dlistut_list) == false);
	cute_ensure(dlist_empty(&src) == true);
}
