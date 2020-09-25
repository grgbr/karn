/**
 * @file      common.h
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      05 Jul 2017
 * @copyright GNU Public License v3
 *
 * Common declarations
 *
 * This file is part of Karn
 *
 * Copyright (C) 2019 Grégor Boirie <gregor.boirie@free.fr>
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

#ifndef _KARN_COMMON_H
#define _KARN_COMMON_H

#include <karn/config.h>
#include <utils/cdefs.h>

#if defined(CONFIG_KARN_ASSERT)

#include <karn/assert.h>

#define karn_assert(_expr) \
	uassert("karn", _expr)

#else  /* !defined(CONFIG_KARN_ASSERT) */

#define karn_assert(_expr)

#endif /* defined(CONFIG_KARN_ASSERT) */

#endif /* _KARN_COMMON_H */
