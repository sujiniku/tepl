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

#include "gtef-buffer.h"
#include "gtef-file.h"

/**
 * SECTION:buffer
 * @Short_description: Subclass of GtkSourceBuffer
 * @Title: GtefBuffer
 */

typedef struct _GtefBufferPrivate GtefBufferPrivate;

struct _GtefBufferPrivate
{
	GtefFile *file;

	guint n_nested_user_actions;
	guint idle_cursor_moved_id;
};

enum
{
	SIGNAL_CURSOR_MOVED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

G_DEFINE_TYPE_WITH_PRIVATE (GtefBuffer, gtef_buffer, GTK_SOURCE_TYPE_BUFFER)

static void
gtef_buffer_dispose (GObject *object)
{
	GtefBufferPrivate *priv = gtef_buffer_get_instance_private (GTEF_BUFFER (object));

	g_clear_object (&priv->file);

	if (priv->idle_cursor_moved_id != 0)
	{
		g_source_remove (priv->idle_cursor_moved_id);
		priv->idle_cursor_moved_id = 0;
	}

	G_OBJECT_CLASS (gtef_buffer_parent_class)->dispose (object);
}

static gboolean
idle_cursor_moved_cb (gpointer user_data)
{
	GtefBuffer *buffer = GTEF_BUFFER (user_data);
	GtefBufferPrivate *priv = gtef_buffer_get_instance_private (buffer);

	g_signal_emit (buffer, signals[SIGNAL_CURSOR_MOVED], 0);

	priv->idle_cursor_moved_id = 0;
	return G_SOURCE_REMOVE;
}

static void
install_idle_cursor_moved (GtefBuffer *buffer)
{
	GtefBufferPrivate *priv = gtef_buffer_get_instance_private (buffer);

	if (priv->idle_cursor_moved_id == 0)
	{
		priv->idle_cursor_moved_id = g_idle_add (idle_cursor_moved_cb, buffer);
	}
}

static void
gtef_buffer_begin_user_action (GtkTextBuffer *buffer)
{
	GtefBufferPrivate *priv = gtef_buffer_get_instance_private (GTEF_BUFFER (buffer));

	priv->n_nested_user_actions++;

	if (GTK_TEXT_BUFFER_CLASS (gtef_buffer_parent_class)->begin_user_action != NULL)
	{
		GTK_TEXT_BUFFER_CLASS (gtef_buffer_parent_class)->begin_user_action (buffer);
	}
}

static void
gtef_buffer_end_user_action (GtkTextBuffer *buffer)
{
	GtefBufferPrivate *priv = gtef_buffer_get_instance_private (GTEF_BUFFER (buffer));

	if (GTK_TEXT_BUFFER_CLASS (gtef_buffer_parent_class)->end_user_action != NULL)
	{
		GTK_TEXT_BUFFER_CLASS (gtef_buffer_parent_class)->end_user_action (buffer);
	}

	g_return_if_fail (priv->n_nested_user_actions > 0);
	priv->n_nested_user_actions--;

	if (priv->n_nested_user_actions == 0)
	{
		install_idle_cursor_moved (GTEF_BUFFER (buffer));
	}
}

static void
gtef_buffer_mark_set (GtkTextBuffer     *buffer,
		      const GtkTextIter *location,
		      GtkTextMark       *mark)
{
	GtefBufferPrivate *priv = gtef_buffer_get_instance_private (GTEF_BUFFER (buffer));

	if (GTK_TEXT_BUFFER_CLASS (gtef_buffer_parent_class)->mark_set != NULL)
	{
		GTK_TEXT_BUFFER_CLASS (gtef_buffer_parent_class)->mark_set (buffer, location, mark);
	}

	if (priv->n_nested_user_actions == 0 &&
	    mark == gtk_text_buffer_get_insert (buffer))
	{
		install_idle_cursor_moved (GTEF_BUFFER (buffer));
	}
}

static void
gtef_buffer_changed (GtkTextBuffer *buffer)
{
	GtefBufferPrivate *priv = gtef_buffer_get_instance_private (GTEF_BUFFER (buffer));

	if (GTK_TEXT_BUFFER_CLASS (gtef_buffer_parent_class)->changed != NULL)
	{
		GTK_TEXT_BUFFER_CLASS (gtef_buffer_parent_class)->changed (buffer);
	}

	if (priv->n_nested_user_actions == 0)
	{
		install_idle_cursor_moved (GTEF_BUFFER (buffer));
	}
}

static void
gtef_buffer_class_init (GtefBufferClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkTextBufferClass *text_buffer_class = GTK_TEXT_BUFFER_CLASS (klass);

	object_class->dispose = gtef_buffer_dispose;

	text_buffer_class->begin_user_action = gtef_buffer_begin_user_action;
	text_buffer_class->end_user_action = gtef_buffer_end_user_action;
	text_buffer_class->mark_set = gtef_buffer_mark_set;
	text_buffer_class->changed = gtef_buffer_changed;

	/**
	 * GtefBuffer::cursor-moved:
	 * @buffer: the #GtefBuffer emitting the signal.
	 *
	 * The ::cursor-moved signal is emitted when the insert mark is moved
	 * explicitely or when the buffer changes (insert/delete).
	 *
	 * A typical use-case for this signal is to update the cursor position
	 * in a statusbar.
	 *
	 * Since: 1.0
	 */
	signals[SIGNAL_CURSOR_MOVED] =
		g_signal_new ("cursor-moved",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (GtefBufferClass, cursor_moved),
			      NULL, NULL, NULL,
			      G_TYPE_NONE, 0);
}

static void
gtef_buffer_init (GtefBuffer *buffer)
{
	GtefBufferPrivate *priv;

	priv = gtef_buffer_get_instance_private (buffer);

	priv->file = gtef_file_new ();
}

/**
 * gtef_buffer_get_file:
 * @buffer: a #GtefBuffer.
 *
 * Returns: (transfer none): the associated #GtefFile.
 * Since: 1.0
 */
GtefFile *
gtef_buffer_get_file (GtefBuffer *buffer)
{
	GtefBufferPrivate *priv;

	g_return_val_if_fail (GTEF_IS_BUFFER (buffer), NULL);

	priv = gtef_buffer_get_instance_private (buffer);
	return priv->file;
}
