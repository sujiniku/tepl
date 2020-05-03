/* Copyright 2016-2020 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef TEPL_VIEW_H
#define TEPL_VIEW_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtksourceview/gtksource.h>
#include <tepl/tepl-macros.h>

G_BEGIN_DECLS

#define TEPL_TYPE_VIEW (tepl_view_get_type ())
_TEPL_EXTERN
G_DECLARE_DERIVABLE_TYPE (TeplView, tepl_view,
			  TEPL, VIEW,
			  GtkSourceView)

struct _TeplViewClass
{
	GtkSourceViewClass parent_class;

	gpointer padding[12];
};

_TEPL_EXTERN
GtkWidget *		tepl_view_new					(void);

_TEPL_EXTERN
GtkWidget *		tepl_view_new_with_buffer			(GtkSourceBuffer *buffer);

_TEPL_EXTERN
void			tepl_view_cut_clipboard				(TeplView *view);

_TEPL_EXTERN
void			tepl_view_copy_clipboard			(TeplView *view);

_TEPL_EXTERN
void			tepl_view_paste_clipboard			(TeplView *view);

_TEPL_EXTERN
void			tepl_view_delete_selection			(TeplView *view);

_TEPL_EXTERN
void			tepl_view_select_all				(TeplView *view);

_TEPL_EXTERN
void			tepl_view_scroll_to_cursor			(TeplView *view);

_TEPL_EXTERN
gboolean		tepl_view_goto_line				(TeplView *view,
									 gint      line);

_TEPL_EXTERN
gboolean		tepl_view_goto_line_offset			(TeplView *view,
									 gint      line,
									 gint      line_offset);

_TEPL_EXTERN
void			tepl_view_select_lines				(TeplView *view,
									 gint      start_line,
									 gint      end_line);

G_END_DECLS

#endif /* TEPL_VIEW_H */
