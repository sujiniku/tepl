/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2016-2020 - Sébastien Wilmet <swilmet@gnome.org>
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
#include <tepl/tepl-macros.h>

G_BEGIN_DECLS

/* String utilities */

_TEPL_EXTERN
gchar *		tepl_utils_str_middle_truncate			(const gchar *str,
								 guint        truncate_length);

_TEPL_EXTERN
gchar *		tepl_utils_str_end_truncate			(const gchar *str,
								 guint        truncate_length);

_TEPL_EXTERN
gchar *		tepl_utils_str_replace				(const gchar *string,
								 const gchar *search,
								 const gchar *replacement);

/* File utilities */

_TEPL_EXTERN
gchar *		tepl_utils_get_file_extension			(const gchar *filename);

_TEPL_EXTERN
gchar *		tepl_utils_get_file_shortname			(const gchar *filename);

_TEPL_EXTERN
gchar *		tepl_utils_replace_home_dir_with_tilde		(const gchar *filename);

_TEPL_EXTERN
gboolean	tepl_utils_decode_uri				(const gchar  *uri,
								 gchar       **scheme,
								 gchar       **user,
								 gchar       **host,
								 gchar       **port,
								 gchar       **path);

G_GNUC_INTERNAL
gchar *		_tepl_utils_get_fallback_basename_for_display	(GFile *location);

_TEPL_EXTERN
gboolean	tepl_utils_create_parent_directories		(GFile         *file,
								 GCancellable  *cancellable,
								 GError       **error);

_TEPL_EXTERN
void		tepl_utils_file_query_exists_async		(GFile               *file,
								 GCancellable        *cancellable,
								 GAsyncReadyCallback  callback,
								 gpointer             user_data);

_TEPL_EXTERN
gboolean	tepl_utils_file_query_exists_finish		(GFile        *file,
								 GAsyncResult *result);

/* Widget utilities */

_TEPL_EXTERN
GtkWidget *	tepl_utils_create_close_button			(void);

G_GNUC_INTERNAL
void		_tepl_utils_associate_secondary_window		(GtkWindow *secondary_window,
								 GtkWidget *main_window_widget);

_TEPL_EXTERN
void		tepl_utils_show_warning_dialog			(GtkWindow   *parent,
								 const gchar *format,
								 ...) G_GNUC_PRINTF(2, 3);

/* Other */

_TEPL_EXTERN
gboolean	tepl_utils_binding_transform_func_smart_bool	(GBinding     *binding,
								 const GValue *from_value,
								 GValue       *to_value,
								 gpointer      user_data);

G_END_DECLS

#endif /* TEPL_UTILS_H */
