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

#ifndef _TOKEN_RBTREE_H
#define _TOKEN_RBTREE_H

#include "token.h"
#include "utils.h"
#include "slist.h"
#include <stddef.h>

struct mapred_token_store {
	void         *tok_root;
	struct slist  tok_list;
	unsigned int  tok_count;
};

struct mapred_token {
	struct slist_node          tok_node;
	const char                *tok_data;
	size_t                     tok_length;
	struct mapred_token_store *tok_store;
	unsigned int               tok_rate;
};

#endif
