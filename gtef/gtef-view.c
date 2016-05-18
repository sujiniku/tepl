/*
 * This file is part of Gtef, a text editor library.
 *
 * From gedit-view.c:
 * Copyright 1998, 1999 - Alex Roberts, Evan Lawrence
 * Copyright 2000, 2002 - Chema Celorio, Paolo Maggi
 * Copyright 2003-2005 - Paolo Maggi
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

#include "gtef-view.h"
#include "gtef-buffer.h"

/**
 * SECTION:view
 * @Short_description: Widget that displays a GtefBuffer
 * @Title: GtefView
 */

#define SCROLL_MARGIN 0.02

G_DEFINE_TYPE (GtefView, gtef_view, GTK_SOURCE_TYPE_VIEW)

static GtkTextBuffer *
gtef_view_create_buffer (GtkTextView *view)
{
	return GTK_TEXT_BUFFER (gtef_buffer_new ());
}

static void
gtef_view_class_init (GtefViewClass *klass)
{
	GtkTextViewClass *text_view_class = GTK_TEXT_VIEW_CLASS (klass);

	text_view_class->create_buffer = gtef_view_create_buffer;
}

static void
gtef_view_init (GtefView *view)
{
}

/**
 * gtef_view_new:
 *
 * Returns: a new #GtefView.
 * Since: 1.0
 */
GtkWidget *
gtef_view_new (void)
{
	return g_object_new (GTEF_TYPE_VIEW, NULL);
}

/**
 * gtef_view_cut_clipboard:
 * @view: a #GtefView.
 *
 * Cuts the clipboard and then scrolls to the cursor position.
 *
 * Since: 1.0
 */
void
gtef_view_cut_clipboard (GtefView *view)
{
	GtkTextBuffer *buffer;
	GtkClipboard *clipboard;

	g_return_if_fail (GTEF_IS_VIEW (view));

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

	clipboard = gtk_widget_get_clipboard (GTK_WIDGET (view),
					      GDK_SELECTION_CLIPBOARD);

	gtk_text_buffer_cut_clipboard (buffer,
				       clipboard,
				       gtk_text_view_get_editable (GTK_TEXT_VIEW (view)));

	gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (view),
				      gtk_text_buffer_get_insert (buffer),
				      SCROLL_MARGIN,
				      FALSE,
				      0.0,
				      0.0);
}

/**
 * gtef_view_copy_clipboard:
 * @view: a #GtefView.
 *
 * Copies the clipboard.
 *
 * Since: 1.0
 */
void
gtef_view_copy_clipboard (GtefView *view)
{
	GtkTextBuffer *buffer;
	GtkClipboard *clipboard;

	g_return_if_fail (GTEF_IS_VIEW (view));

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

	clipboard = gtk_widget_get_clipboard (GTK_WIDGET (view),
					      GDK_SELECTION_CLIPBOARD);

	gtk_text_buffer_copy_clipboard (buffer, clipboard);

	/* On copy do not scroll, we are already on screen. */
}

/**
 * gtef_view_paste_clipboard:
 * @view: a #GtefView.
 *
 * Pastes the clipboard and then scrolls to the cursor position.
 *
 * Since: 1.0
 */
void
gtef_view_paste_clipboard (GtefView *view)
{
	GtkTextBuffer *buffer;
	GtkClipboard *clipboard;

	g_return_if_fail (GTEF_IS_VIEW (view));

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

	clipboard = gtk_widget_get_clipboard (GTK_WIDGET (view),
					      GDK_SELECTION_CLIPBOARD);

	gtk_text_buffer_paste_clipboard (buffer,
					 clipboard,
					 NULL,
					 gtk_text_view_get_editable (GTK_TEXT_VIEW (view)));

	gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (view),
				      gtk_text_buffer_get_insert (buffer),
				      SCROLL_MARGIN,
				      FALSE,
				      0.0,
				      0.0);
}

/**
 * gtef_view_delete_selection:
 * @view: a #GtefView.
 *
 * Deletes the text currently selected in the #GtkTextBuffer associated
 * to the view and then scrolls to the cursor position.
 *
 * Since: 1.0
 */
void
gtef_view_delete_selection (GtefView *view)
{
	GtkTextBuffer *buffer;

	g_return_if_fail (GTEF_IS_VIEW (view));

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

	gtk_text_buffer_delete_selection (buffer,
					  TRUE,
					  gtk_text_view_get_editable (GTK_TEXT_VIEW (view)));

	gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (view),
				      gtk_text_buffer_get_insert (buffer),
				      SCROLL_MARGIN,
				      FALSE,
				      0.0,
				      0.0);
}

/**
 * gtef_view_select_all:
 * @view: a #GtefView.
 *
 * Selects all the text.
 *
 * Since: 1.0
 */
void
gtef_view_select_all (GtefView *view)
{
	GtkTextBuffer *buffer;
	GtkTextIter start;
	GtkTextIter end;

	g_return_if_fail (GTEF_IS_VIEW (view));

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

	gtk_text_buffer_get_bounds (buffer, &start, &end);
	gtk_text_buffer_select_range (buffer, &start, &end);
}

/**
 * gtef_view_scroll_to_cursor:
 * @view: a #GtefView.
 *
 * Scrolls the @view to the cursor position.
 *
 * Since: 1.0
 */
void
gtef_view_scroll_to_cursor (GtefView *view)
{
	GtkTextBuffer *buffer;

	g_return_if_fail (GTEF_IS_VIEW (view));

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

	gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (view),
				      gtk_text_buffer_get_insert (buffer),
				      0.25,
				      FALSE,
				      0.0,
				      0.0);
}
