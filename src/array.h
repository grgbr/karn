/**
 * @file      array.h
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      05 Jul 2017
 * @copyright GNU Public License v3
 *
 * Fixed length arrays interface
 *
 * @defgroup array_fixed Fixed length arrays
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

#ifndef _ARRAY_H
#define _ARRAY_H

#include <utils.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>

/**
 * Retrieve the maximum number of items a static array may contain
 *
 * @param _array statically declared array
 *
 * @return number of items
 *
 * @ingroup array_fixed
 */
#define array_nr(_array) \
	(sizeof(_array) / sizeof((_array)[0]))

/**
 * @typedef array_compare_fn
 *
 * @brief Array item comparison function prototype
 *
 * @param first  array item to compare with @p second
 * @param second array item to compare with @p first
 *
 * @retval <0 @p first precedes @p second
 * @retval 0  @p first and @p second are equal
 * @retval 0  @p first follows @p second
 *
 * @ingroup array_fixed
 */
typedef int (array_compare_fn)(const char *restrict first,
                               const char *restrict second);

/**
 * Fixed length array
 *
 * @ingroup array_fixed
 */
struct array_fixed {
	/** maximum number of items this array can hold */
	unsigned int  arr_nr;
	/** underlying memory area holding items */
	char         *arr_items;
};

/* Internal array_fixed consistency checker */
#define array_assert(_array)       \
	assert(_array);            \
	assert(_array->arr_items); \
	assert(_array->arr_nr)

/**
 * Retrieve the maximum number of items a array_fixed may contain
 *
 * @param array array_fixed to retrieve number of items from
 *
 * @return number of items
 *
 * @ingroup array_fixed
 */
static inline unsigned int
array_fixed_nr(const struct array_fixed *array)
{
	array_assert(array);

	return array->arr_nr;
}

/**
 * Retrieve item specified by index
 *
 * @param array     array_fixed to retrieve item from
 * @param item_size size of @p item in bytes
 * @param index     array index of item to retrieve
 *
 * @return item
 *
 * @warning Behavior is undefined when called with a zero @p item_size or an out
 *          of bound @p index.
 *
 * @ingroup array_fixed
 */
static inline char *
array_fixed_item(const struct array_fixed *array,
                 size_t                    item_size,
                 unsigned int              index)
{
	array_assert(array);
	assert(item_size);
	assert(index < array->arr_nr);

	return &array->arr_items[index * item_size];
}

/**
 * Return type casted pointer to item specified by index
 *
 * @param _array array_fixed to retrieve item from
 * @param _type  item type
 * @param _index array index of item to retrieve
 *
 * @return pointer to item casted to @p _type
 *
 * @warning Behavior is undefined when called with an out of bound @p index.
 *
 * @ingroup array_fixed
 */
#define array_fixed_entry(_array, _type, _index) \
	((_type *)array_fixed_item(_array, sizeof(_type), _index))

/**
 * Retrieve index of item owned by specified array_fixed
 *
 * @param array     array_fixed owning @p item
 * @param item_size size of @p item in bytes
 * @param item      item to calculate index for
 *
 * @return index
 *
 * @warning Behavior is undefined when called with a zero @p item_size.
 *
 * @ingroup array_fixed
 */
static inline unsigned int
array_fixed_item_index(const struct array_fixed *array,
                       size_t                    item_size,
                       const char               *item)
{
	array_assert(array);
	assert(item_size);
	assert(item >= &array->arr_items[0]);
	assert(item < &array->arr_items[array->arr_nr]);
	assert(!((size_t)item % item_size));

	return (item - &array->arr_items[0]) / item_size;
}

/**
 * Retrieve index of typed item owned by specified array_fixed
 *
 * @param _array array_fixed owning @p item
 * @param _entry typed item to calculate index for
 *
 * @return index
 *
 * @ingroup array_fixed
 */
#define array_fixed_entry_index(_array, _entry) \
	array_fixed_item_index(_array, sizeof(*(_entry)), (const char *)_entry)

/**
 * Initialize an array_fixed
 *
 * @param array array_fixed to initialize
 * @param items underlying memory area containing items
 * @param nr    maximum number of items @p array may contain
 *
 * @p items must point to a memory area large enough to contain at least @p nr
 * items
 *
 * @warning Behavior is undefined when called with a zero @p nr.
 *
 * @ingroup array_fixed
 */
static inline void array_init_fixed(struct array_fixed *array,
                                    char               *items,
                                    unsigned int        nr)
{
	assert(array);
	assert(items);
	assert(nr);

	array->arr_nr = nr;
	array->arr_items = items;
}

/**
 * Release resources allocated by an array_fixed
 *
 * @param array array_fixed to release resources for
 *
 * @ingroup array_fixed
 */
static inline void array_fini_fixed(struct array_fixed *array __unused)
{
	array_assert(array);
}

#endif
