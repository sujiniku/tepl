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

#include "gtef-view.h"
#include "gtef-buffer.h"

/**
 * SECTION:view
 * @Short_description: Widget that displays a GtefBuffer
 * @Title: GtefView
 */

typedef struct _GtefViewPrivate GtefViewPrivate;

struct _GtefViewPrivate
{
	gint something;
};

G_DEFINE_TYPE_WITH_PRIVATE (GtefView, gtef_view, GTK_SOURCE_TYPE_VIEW)

static void
gtef_view_dispose (GObject *object)
{

	G_OBJECT_CLASS (gtef_view_parent_class)->dispose (object);
}

static GtkTextBuffer *
gtef_view_create_buffer (GtkTextView *view)
{
	return GTK_TEXT_BUFFER (gtef_buffer_new ());
}

static void
gtef_view_class_init (GtefViewClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkTextViewClass *text_view_class = GTK_TEXT_VIEW_CLASS (klass);

	object_class->dispose = gtef_view_dispose;

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
