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

#include "utils.h"
#include <unistd.h>

size_t
mapred_forward_delim_len(const char *data, size_t size)
{
	const char *ptr = data;

	while (size) {
		if (!*ptr)
			/* Premature end of string. */
			break;

		if (!mapred_ischar_delim(*ptr))
			/* Not a delimiter character: get out. */
			break;
		ptr++;
		size--;
	}

	return ptr - data;
}

size_t
mapred_forward_token_len(const char *data, size_t size)
{
	const char *ptr = data;

	while (size) {
		if (!*ptr)
			/* Premature end of string. */
			break;

		if (mapred_ischar_delim(*ptr))
			/* This is a delimiter character: get out. */
			break;
		ptr++;
		size--;
	}

	return ptr - data;
}

size_t
mapred_backward_token_len(const char *data, size_t size)
{
	size_t sz  = size;

	while (sz) {
		int c = data[sz - 1];

		if (!c)
			/* Premature end of string. */
			break;

		if (mapred_ischar_delim(c))
			/* This is a delimiter character: get out. */
			break;

		sz--;
	}

	return size - sz;
}
