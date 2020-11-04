/* SPDX-FileCopyrightText: 2016-2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
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

_TEPL_EXTERN
gchar *		tepl_utils_markup_escape_text			(const gchar *src);

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

_TEPL_EXTERN
void		tepl_utils_list_box_clear			(GtkListBox *list_box);

_TEPL_EXTERN
void		tepl_utils_list_box_setup_scrolling		(GtkListBox        *list_box,
								 GtkScrolledWindow *scrolled_window);

_TEPL_EXTERN
void		tepl_utils_list_box_scroll_to_row		(GtkListBox    *list_box,
								 GtkListBoxRow *row);

_TEPL_EXTERN
void		tepl_utils_list_box_scroll_to_selected_row	(GtkListBox *list_box);

_TEPL_EXTERN
GtkListBoxRow *	tepl_utils_list_box_get_row_at_index_with_filter (GtkListBox           *list_box,
								  gint                  index,
								  GtkListBoxFilterFunc  filter_func,
								  gpointer              user_data);

_TEPL_EXTERN
GtkListBoxRow **tepl_utils_list_box_get_filtered_children	(GtkListBox           *list_box,
								 GtkListBoxFilterFunc  filter_func,
								 gpointer              user_data,
								 gint                 *n_filtered_children);

/* Other */

_TEPL_EXTERN
gboolean	tepl_utils_binding_transform_func_smart_bool	(GBinding     *binding,
								 const GValue *from_value,
								 GValue       *to_value,
								 gpointer      user_data);

G_END_DECLS

#endif /* TEPL_UTILS_H */
