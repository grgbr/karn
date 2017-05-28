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

#include <errno.h>

int
mapred_register_token(struct mapred_token_store *store,
                      const char                *data,
                      size_t                     length)
{
	struct mapred_token *tok;
	value_t             *itm;

	itm = hattrie_get(store->tok_trie, data, length);
	if (!itm)
		return -errno;

	if (!*itm) {
		tok = malloc(sizeof(*tok));
		if (!tok) {
			hattrie_del(store->tok_trie, data, length);
			return -ENOMEM;
		}

		tok->tok_data = data;
		tok->tok_length = length;
		tok->tok_rate = 1;
		*(struct mapred_token **)itm = tok;
	}
	else
		(*(struct mapred_token **)itm)->tok_rate++;

	return 0;
}

int
mapred_flatten_token_store(struct mapred_token_store *store)
{
	hattrie_iter_t *iter;

	iter = hattrie_iter_begin(store->tok_trie, true);
	if (!iter)
		return -errno;

	while (!hattrie_iter_finished(iter)) {
		struct mapred_token *tok;

		tok = *((struct mapred_token **)hattrie_iter_val(iter));

		slist_nqueue(&store->tok_list, &tok->tok_node);

		hattrie_iter_next(iter);
	}

	hattrie_iter_free(iter);
	store->tok_count = hattrie_size(store->tok_trie);

	hattrie_free(store->tok_trie);
	store->tok_trie = NULL;

	return 0;
}

int
mapred_init_token_store(struct mapred_token_store *store)
{
	store->tok_trie = hattrie_create();
	if (!store->tok_trie)
		return -errno;

	slist_init(&store->tok_list);

	store->tok_count = 0;

	return 0;
}

void
mapred_fini_token_store(const struct mapred_token_store *store)
{
	if (!store->tok_trie) {
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
		hattrie_iter_t *iter;

		iter = hattrie_iter_begin(store->tok_trie, false);
		if (!iter)
			goto free;

		while (!hattrie_iter_finished(iter)) {
			struct mapred_token *tok;

			tok = *(struct mapred_token **)hattrie_iter_val(iter);
			free(tok);

			hattrie_iter_next(iter);
		}

		hattrie_iter_free(iter);

free:
		hattrie_free(store->tok_trie);
	}
}
