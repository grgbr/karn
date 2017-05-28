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

#include "token.h"
#include "utils.h"
#include <stdio.h>

extern int  mapred_flatten_token_store(struct mapred_token_store *store);

extern int  mapred_register_token(struct mapred_token_store *store,
                                  const char                *data,
                                  size_t                     length);

static inline struct mapred_token *
mapred_list_node_token(struct slist_node *node)
{
	return slist_entry(node, struct mapred_token, tok_node);
}

int
mapred_compare_token(const void *first, const void *second)
{
	const struct mapred_token *fst = first;
	const struct mapred_token *snd = second;

	return mapred_compare_strings(fst->tok_data, fst->tok_length,
	                              snd->tok_data, snd->tok_length);
}

static struct slist_node *
mapred_merge_token_list(struct mapred_token_store *result,
                        struct slist_node         *merge,
                        struct mapred_token_store *source)
{
	struct slist        *res_lst = &result->tok_list;
	struct slist_node   *res_cur = merge;
	struct slist_node   *res_nxt = merge;
	struct mapred_token *res_tok;
	struct slist        *src_lst = &source->tok_list;
	struct mapred_token *ref_tok = mapred_list_node_token(slist_first(src_lst));
	struct slist_node   *src_cur;
	struct slist_node   *src_nxt;
	unsigned int         src_cnt;
	int                  cmp = 1;

	mapred_assert(!slist_empty(res_lst));
	mapred_assert(!slist_empty(src_lst));

#if 0
	cmp = mapred_compare_token(mapred_list_node_token(slist_last(src_lst)),
	                           mapred_list_node_token(slist_first(res_lst)));
	if (cmp < 0) {
		res_cur = slist_head(res_lst);
		src_cur = slist_last(src_lst);
		src_cnt = source->tok_count;
		goto splice;
	}
#endif

	while (true) {
		res_nxt = slist_next(res_nxt);
		if (!res_nxt)
			break;

		res_tok = mapred_list_node_token(res_nxt);

		cmp = mapred_compare_token(res_tok, ref_tok);
		if (cmp >= 0)
			break;

		res_cur = res_nxt;
	}

	if (!cmp) {
		res_tok->tok_rate += ref_tok->tok_rate;

		slist_dqueue(src_lst);
		free(ref_tok);
		source->tok_count--;

		return res_nxt;
	}

	if (!res_nxt) {
		src_cur = slist_last(src_lst);
		src_cnt = source->tok_count;
		goto splice;
	}

#if 0
	cmp = mapred_compare_token(mapred_list_node_token(slist_last(src_lst)),
	                           res_tok);
	if (cmp < 0) {
		src_cur = slist_last(src_lst);
		src_cnt = source->tok_count;
		goto splice;
	}
#endif

	src_cur = slist_head(src_lst);
	src_nxt = src_cur;
	src_cnt = 0;
	while (true) {
		struct mapred_token *src_tok;

		src_nxt = slist_next(src_nxt);
		if (!src_nxt)
			break;

		src_tok = mapred_list_node_token(src_nxt);

		cmp = mapred_compare_token(src_tok, res_tok);
		if (cmp >= 0)
			break;

		src_cnt++;

		src_cur = src_nxt;
	}

splice:
	slist_splice(res_lst, res_cur, src_lst, slist_head(src_lst), src_cur);

	result->tok_count += src_cnt;
	source->tok_count -= src_cnt;

	return src_cur;
}

void
mapred_merge_token_store(struct mapred_token_store *result,
                         struct mapred_token_store *source)
{
	struct slist_node *res_mrg;

	mapred_assert(!slist_empty(&result->tok_list));
	mapred_assert(!slist_empty(&source->tok_list));

	res_mrg = slist_head(&result->tok_list);
	do {
		res_mrg = mapred_merge_token_list(result, res_mrg, source);
	} while (!slist_empty(&source->tok_list));

#if 0
clear:
#endif
	source->tok_count = 0;
}

int
mapred_tokenize(struct mapred_token_store *store,
                const char                *data,
                size_t                     size)
{
	while (size) {
		size_t len;

		/*
		 * Skip sequence of contiguous delimiter characters, i.e.
		 * jump to start of next token.
		 */
		len = mapred_forward_delim_len(data, size);
		data += len;
		size -= len;

		/* Find current token boundaries. */
		len = mapred_forward_token_len(data, size);
		if (len) {
			int err;

			err = mapred_register_token(store, data, len);
			if (err)
				return err;
		}

		data += len;
		size -= len;
	}

	return mapred_flatten_token_store(store);
}

int
mapred_dump_token_store(const struct mapred_token_store *store)
{
	struct mapred_token *tok;
	unsigned int         count = 0;

	slist_foreach_entry(&store->tok_list, tok, tok_node) {
		count += tok->tok_rate;

		printf("%.*s: %u\n",
		       (int)tok->tok_length, tok->tok_data, tok->tok_rate);
	}

	printf("Total number of tokens: %u unique out of %u\n",
	       store->tok_count, count);

	return 0;
}
