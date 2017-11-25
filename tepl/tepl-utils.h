/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2016, 2017 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef TEPL_UTILS_H
#define TEPL_UTILS_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

/* String utilities */

gchar *		_tepl_utils_str_middle_truncate			(const gchar *str,
								 guint        truncate_length);

/* File utilities */

G_GNUC_INTERNAL
gchar *		_tepl_utils_replace_home_dir_with_tilde		(const gchar *filename);

G_GNUC_INTERNAL
gboolean	_tepl_utils_decode_uri				(const gchar  *uri,
								 gchar       **scheme,
								 gchar       **user,
								 gchar       **host,
								 gchar       **port,
								 gchar       **path);

G_GNUC_INTERNAL
gchar *		_tepl_utils_get_fallback_basename_for_display	(GFile *location);

/* Widget utilities */

GtkWidget *	_tepl_utils_create_close_button			(void);

G_GNUC_INTERNAL
void		_tepl_utils_associate_secondary_window		(GtkWindow *secondary_window,
								 GtkWidget *main_window_widget);

G_END_DECLS

#endif /* TEPL_UTILS_H */
