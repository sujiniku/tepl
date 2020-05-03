/* Copyright 2014, 2017 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * Tepl is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * Tepl is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TEPL_ENCODING_PRIVATE_H
#define TEPL_ENCODING_PRIVATE_H

#include <glib.h>

G_BEGIN_DECLS

/*
 * TeplEncodingDuplicates:
 * @TEPL_ENCODING_DUPLICATES_KEEP_FIRST: Keep the first occurrence.
 * @TEPL_ENCODING_DUPLICATES_KEEP_LAST: Keep the last occurrence.
 *
 * Specifies which encoding occurrence to keep when removing duplicated
 * encodings in a list with _tepl_encoding_remove_duplicates().
 */
typedef enum _TeplEncodingDuplicates
{
	TEPL_ENCODING_DUPLICATES_KEEP_FIRST,
	TEPL_ENCODING_DUPLICATES_KEEP_LAST
} TeplEncodingDuplicates;

G_GNUC_INTERNAL
GSList *	_tepl_encoding_remove_duplicates	(GSList                 *encodings,
							 TeplEncodingDuplicates  removal_type);

G_END_DECLS

#endif  /* TEPL_ENCODING_PRIVATE_H */
