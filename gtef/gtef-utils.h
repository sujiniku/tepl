/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GTEF_UTILS_H
#define GTEF_UTILS_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

/* File utilities */

G_GNUC_INTERNAL
gchar *		_gtef_utils_replace_home_dir_with_tilde		(const gchar *filename);

G_GNUC_INTERNAL
gboolean	_gtef_utils_decode_uri				(const gchar  *uri,
								 gchar       **scheme,
								 gchar       **user,
								 gchar       **host,
								 gchar       **port,
								 gchar       **path);

/* String utilities */

G_GNUC_INTERNAL
gchar *		_gtef_utils_make_valid_utf8			(const gchar *_str);

G_END_DECLS

#endif /* GTEF_UTILS_H */
