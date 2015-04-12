/* =========================================================================
 * This file is part of NITRO
 * =========================================================================
 * 
 * (C) Copyright 2004 - 2008, General Dynamics - Advanced Information Systems
 *
 * NITRO is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this program; if not, If not, 
 * see <http://www.gnu.org/licenses/>.
 *
 */

#include "nitf/Pair.h"

NITFAPI(void) nitf_Pair_init(nitf_Pair * pair, const char *key,
                             NITF_DATA * data)
{
    size_t len = strlen(key);
    pair->key = (char *) NITF_MALLOC(len + 1);
    /* Help, we have an unchecked malloc here! */
    pair->key[len] = 0;
    strcpy(pair->key, key);
    pair->data = data;
}


NITFAPI(void) nitf_Pair_copy(nitf_Pair * dst, nitf_Pair * src)
{
    nitf_Pair_init(dst, src->key, src->data);
}
