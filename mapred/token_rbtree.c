/******************************************************************************
 * This file is part of Mapred
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
 ******************************************************************************/

#include <stdlib.h>
#include <search.h>
#include <errno.h>

int
mapred_register_token(struct mapred_token_store *store,
                      const char                *data,
                      size_t                     length)
{
	struct mapred_token  *tok;
	void                 *item;
	int                   err;

	tok = malloc(sizeof(*tok));
	if (!tok)
		return -ENOMEM;

	tok->tok_data = data;
	tok->tok_length = length;
	tok->tok_store = store;
	tok->tok_rate = 0;

	item = tsearch(tok, &store->tok_root, mapred_compare_token);
	if (!item) {
		/* Internal tree allocation failed: bail out. */
		err = -ENOMEM;
		goto free;
	}

	/* Update current count for this token. */
	(*(struct mapred_token **)item)->tok_rate++;

	if ((*(struct mapred_token **)item) != tok) {
		/* Token already registered: free memory allocated above. */
		err = 0;
		goto free;
	}

	/* Update overall unique token count for this store. */
	store->tok_count++;

	return 0;

free:
	free(tok);

	return err;
}

static void
mapred_link_token(const void *node, const VISIT which, const int depth __unused)
{
	if ((which == postorder) || (which == leaf)) {
		struct mapred_token *tok = *((struct mapred_token **)node);

		slist_nqueue(&tok->tok_store->tok_list, &tok->tok_node);
	}
}

static void
mapred_dummy_free_token(void *node __unused)
{
}

int
mapred_flatten_token_store(struct mapred_token_store *store)
{
	twalk(store->tok_root, mapred_link_token);

	/* Free tree nodes only. */
	tdestroy(store->tok_root, mapred_dummy_free_token);
	store->tok_root = NULL;

	return 0;
}

int
mapred_init_token_store(struct mapred_token_store *store)
{
	store->tok_root  = NULL;

	slist_init(&store->tok_list);

	store->tok_count = 0;

	return 0;
}

void mapred_fini_token_store(const struct mapred_token_store *store)
{
	if (!store->tok_root) {
		struct mapred_token *curr;
		struct mapred_token *prev = NULL;

		/* Free all nodes sitting in the linked list. */
		slist_foreach_entry(&store->tok_list, curr, tok_node) {
			/* No operation is performed if prev is NULL. */
			free(prev);
			prev = curr;
		}

		free(prev);
	}
	else {
		mapred_assert(store->tok_root);

		tdestroy(store->tok_root, free);
	}
}
