/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * Gtef is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * Gtef is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GTEF_UTILS_H
#define GTEF_UTILS_H

#if !defined (GTEF_H_INSIDE) && !defined (GTEF_COMPILATION)
#error "Only <gtef/gtef.h> can be included directly."
#endif

#include <gio/gio.h>

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

G_GNUC_INTERNAL
gchar *		_gtef_utils_get_fallback_basename_for_display	(GFile *location);

/* String utilities */

G_GNUC_INTERNAL
gchar **	_gtef_utils_strv_copy				(const gchar * const *strv);

G_END_DECLS

#endif /* GTEF_UTILS_H */
