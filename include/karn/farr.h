/**
 * @file      farr.h
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      05 Jul 2017
 * @copyright GNU Public License v3
 *
 * Fixed length arrays interface
 *
 * @defgroup farr Fixed length arrays
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

#ifndef _KARN_FARR_H
#define _KARN_FARR_H

#include <karn/common.h>
#include <stdlib.h>
#include <sys/types.h>

/**
 * @typedef farr_compare_fn
 *
 * @brief Array slot comparison function prototype
 *
 * @param first  array slot to compare with @p second
 * @param second array slot to compare with @p first
 *
 * @retval <0 @p first precedes @p second
 * @retval 0  @p first and @p second are equal
 * @retval 0  @p first follows @p second
 *
 * @ingroup farr
 */
typedef int (farr_compare_fn)(const char *restrict first,
                              const char *restrict second);

/**
 * @typedef farr_copy_fn
 *
 * @brief Array slot copy function prototype
 *
 * @param destination array slot to copy into
 * @param source      array slot to copy from
 *
 * @ingroup farr
 */
typedef void (farr_copy_fn)(char       *restrict destination,
                            const char *restrict source);

/**
 * Fixed length array
 *
 * @ingroup farr
 */
struct farr {
        size_t        farr_size;
	/** maximum number of slots this array can hold */
	unsigned int  farr_nr;
	/** underlying memory area holding slots */
	char         *farr_slots;
};

/* Internal farr consistency checker */
#define farr_assert(_array) \
	karn_assert(_array); \
	karn_assert((_array)->farr_size); \
	karn_assert((_array)->farr_slots); \
	karn_assert((_array)->farr_nr)

/**
 * Retrieve the maximum number of slots a farr may contain
 *
 * @param array farr to retrieve number of slots from
 *
 * @return number of slots
 *
 * @ingroup farr
 */
static inline unsigned int
farr_nr(const struct farr *array)
{
	farr_assert(array);

	return array->farr_nr;
}

/**
 * Retrieve size of a single slot contained by the specified farr
 *
 * @param array farr to retrieve slot size from
 *
 * @return size in bytes
 *
 * @ingroup farr
 */
static inline size_t
farr_slot_size(const struct farr *array)
{
	farr_assert(array);

	return array->farr_size;
}

/**
 * Retrieve slot specified by index
 *
 * @param array     farr to retrieve slot from
 * @param slot_size size of @p slot in bytes
 * @param index     array index of slot to retrieve
 *
 * @return slot
 *
 * @warning Behavior is undefined when called with a zero @p slot_size or an out
 *          of bound @p index.
 *
 * @ingroup farr
 */
static inline char * farr_slot(const struct farr *array, unsigned int index)
{
	farr_assert(array);
	karn_assert(index < array->farr_nr);

	return &array->farr_slots[index * array->farr_size];
}

/**
 * Return type casted pointer to slot specified by index
 *
 * @param _array farr to retrieve slot from
 * @param _type  slot type
 * @param _index array index of slot to retrieve
 *
 * @return pointer to slot casted to @p _type
 *
 * @warning Behavior is undefined when called with an out of bound @p index.
 *
 * @ingroup farr
 */
#define farr_entry(_array, _type, _index) \
	((_type *)farr_slot(_array, sizeof(_type), _index))

/**
 * Retrieve index of slot owned by specified farr
 *
 * @param array     farr owning @p slot
 * @param slot_size size of @p slot in bytes
 * @param slot      slot to calculate index for
 *
 * @return index
 *
 * @warning Behavior is undefined when called with a zero @p slot_size.
 *
 * @ingroup farr
 */
static inline unsigned int farr_slot_index(const struct farr *array,
                                           const char        *slot)
{
	farr_assert(array);
	karn_assert(slot >= &array->farr_slots[0]);
	karn_assert(slot < &array->farr_slots[array->farr_size * array->farr_nr]);
	karn_assert(!((size_t)slot % array->farr_size));

	return (slot - &array->farr_slots[0]) / array->farr_size;
}

/**
 * Retrieve index of typed slot owned by specified farr
 *
 * @param _array farr owning @p slot
 * @param _entry typed slot to calculate index for
 *
 * @return index
 *
 * @ingroup farr
 */
#define farr_entry_index(_array, _entry) \
	farr_slot_index(_array, sizeof(*(_entry)), (const char *)_entry)

/**
 * Exchange 2 slot contents by copy.
 *
 * @param first     first slot to swap with @p second
 * @param second    second slot to swap with @p first
 * @param auxiliary temporary memory space used to perform the swap
 * @param copy      slot copy function
 *
 * @p first, @p second and @p auxiliary must point to equally sized memory
 * spaces as expected by @p copy.
 *
 * @ingroup farr
 */
static inline void farr_swap(char         *first,
                             char         *second,
                             char         *auxiliary,
                             farr_copy_fn *copy)
{
	copy(auxiliary, first);
	copy(first, second);
	copy(second, auxiliary);
}

/**
 * Initialize an farr
 *
 * @param array   farr to initialize
 * @param slots   underlying memory area containing slots
 * @param slot_nr maximum number of slots @p array may contain
 *
 * @p slots must point to a memory area large enough to contain at least
 * @p slot_nr slots
 *
 * @warning Behavior is undefined when called with a zero @p slot_nr.
 *
 * @ingroup farr
 */
static inline void farr_init(struct farr  *array,
                             char         *slots,
                             size_t        slot_size,
                             unsigned int  slot_nr)
{
	karn_assert(array);
	karn_assert(slots);
	karn_assert(slot_size);
	karn_assert(slot_nr);

	array->farr_size = slot_size;
	array->farr_nr = slot_nr;
	array->farr_slots = slots;
}

/**
 * Release resources allocated by an farr
 *
 * @param array farr to release resources for
 *
 * @ingroup farr
 */
static inline void farr_fini(struct farr *array __unused)
{
	farr_assert(array);
}

#if defined(CONFIG_KARN_FARR_BUBBLE_SORT)

extern void farr_bubble_sort(char            *entries,
                             size_t           entry_size,
                             unsigned int     entry_nr,
                             farr_compare_fn *compare,
                             farr_copy_fn    *copy);

#endif /* defined(CONFIG_KARN_FARR_BUBBLE_SORT) */

#if defined(CONFIG_KARN_FARR_SELECTION_SORT)

extern void farr_selection_sort(char            *entries,
                                size_t           entry_size,
                                unsigned int     entry_nr,
                                farr_compare_fn *compare,
                                farr_copy_fn    *copy);

#endif /* defined(CONFIG_KARN_FARR_SELECTION_SORT) */

#if defined(CONFIG_KARN_FARR_INSERTION_SORT)

extern void farr_insertion_sort(char            *entries,
                                size_t           entry_size,
                                unsigned int     entry_nr,
                                farr_compare_fn *compare,
                                farr_copy_fn    *copy);

#endif /* defined(CONFIG_KARN_FARR_INSERTION_SORT) */

#if defined(CONFIG_KARN_FARR_QUICK_SORT)

extern void farr_quick_sort(char            *entries,
                            size_t           entry_size,
                            unsigned int     entry_nr,
                            farr_compare_fn *compare,
                            farr_copy_fn    *copy);

#endif /* defined(CONFIG_KARN_FARR_QUICK_SORT) */

#if defined(CONFIG_KARN_FARR_INTRO_SORT)

extern void farr_intro_sort(char            *entries,
                            size_t           entry_size,
                            unsigned int     entry_nr,
                            farr_compare_fn *compare,
                            farr_copy_fn    *copy);

#endif /* defined(CONFIG_KARN_FARR_INTRO_SORT) */

#endif /* _KARN_FARR_H */
