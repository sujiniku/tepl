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

#include "config.h"
#include "gtef-buffer.h"
#include <glib/gi18n-lib.h>
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

	gchar *short_name;

	guint n_nested_user_actions;
	guint idle_cursor_moved_id;
};

enum
{
	PROP_0,
	PROP_TITLE,
	N_PROPERTIES
};

enum
{
	SIGNAL_CURSOR_MOVED,
	N_SIGNALS
};

static GParamSpec *properties[N_PROPERTIES];
static guint signals[N_SIGNALS];

G_DEFINE_TYPE_WITH_PRIVATE (GtefBuffer, gtef_buffer, GTK_SOURCE_TYPE_BUFFER)

static void
gtef_buffer_get_property (GObject    *object,
			  guint       prop_id,
			  GValue     *value,
			  GParamSpec *pspec)
{
	GtefBuffer *buffer = GTEF_BUFFER (object);

	switch (prop_id)
	{
		case PROP_TITLE:
			g_value_take_string (value, gtef_buffer_get_title (buffer));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gtef_buffer_set_property (GObject      *object,
			  guint         prop_id,
			  const GValue *value,
			  GParamSpec   *pspec)
{
	switch (prop_id)
	{
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

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

static void
gtef_buffer_finalize (GObject *object)
{
	GtefBufferPrivate *priv = gtef_buffer_get_instance_private (GTEF_BUFFER (object));

	g_free (priv->short_name);

	G_OBJECT_CLASS (gtef_buffer_parent_class)->finalize (object);
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
		/* High idle priority, because after loading a big file, the
		 * GtkTextView works in the background to compute the whole size
		 * etc. HIGH_IDLE permits to send the signal as soon as the
		 * content is fully loaded in the GtkTextBuffer, even if
		 * GtkTextView has not finished.
		 */
		priv->idle_cursor_moved_id = g_idle_add_full (G_PRIORITY_HIGH_IDLE,
							      idle_cursor_moved_cb,
							      buffer,
							      NULL);
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
gtef_buffer_modified_changed (GtkTextBuffer *buffer)
{
	if (GTK_TEXT_BUFFER_CLASS (gtef_buffer_parent_class)->modified_changed != NULL)
	{
		GTK_TEXT_BUFFER_CLASS (gtef_buffer_parent_class)->modified_changed (buffer);
	}

	g_object_notify_by_pspec (G_OBJECT (buffer), properties[PROP_TITLE]);
}

static void
gtef_buffer_class_init (GtefBufferClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkTextBufferClass *text_buffer_class = GTK_TEXT_BUFFER_CLASS (klass);

	object_class->get_property = gtef_buffer_get_property;
	object_class->set_property = gtef_buffer_set_property;
	object_class->dispose = gtef_buffer_dispose;
	object_class->finalize = gtef_buffer_finalize;

	text_buffer_class->begin_user_action = gtef_buffer_begin_user_action;
	text_buffer_class->end_user_action = gtef_buffer_end_user_action;
	text_buffer_class->mark_set = gtef_buffer_mark_set;
	text_buffer_class->changed = gtef_buffer_changed;
	text_buffer_class->modified_changed = gtef_buffer_modified_changed;

	/**
	 * GtefBuffer:title:
	 *
	 * The buffer title. See gtef_buffer_get_title().
	 *
	 * Since: 1.0
	 */
	properties[PROP_TITLE] =
		g_param_spec_string ("title",
				     "Title",
				     "",
				     NULL,
				     G_PARAM_READABLE |
				     G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, N_PROPERTIES, properties);

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
query_display_name_cb (GObject      *source_object,
		       GAsyncResult *result,
		       gpointer      user_data)
{
	GFile *location = G_FILE (source_object);
	GtefBuffer *buffer = GTEF_BUFFER (user_data);
	GtefBufferPrivate *priv;
	GFileInfo *info;
	GError *error = NULL;

	priv = gtef_buffer_get_instance_private (buffer);

	info = g_file_query_info_finish (location, result, &error);

	if (error != NULL)
	{
		g_warning (_("Error when querying file information: %s"), error->message);
		g_clear_error (&error);
		goto out;
	}

	g_free (priv->short_name);
	priv->short_name = g_strdup (g_file_info_get_display_name (info));

	g_object_notify_by_pspec (G_OBJECT (buffer), properties[PROP_TITLE]);

out:
	g_clear_object (&info);

	/* Async operation finished */
	g_object_unref (buffer);
}

static void
update_short_name (GtefBuffer *buffer)
{
	GtefBufferPrivate *priv;
	GFile *location;

	priv = gtef_buffer_get_instance_private (buffer);

	location = gtk_source_file_get_location (GTK_SOURCE_FILE (priv->file));

	if (location == NULL)
	{
		g_free (priv->short_name);
		priv->short_name = g_strdup (_("Untitled File"));

		g_object_notify_by_pspec (G_OBJECT (buffer), properties[PROP_TITLE]);
	}
	else
	{
		g_file_query_info_async (location,
					 G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
					 G_FILE_QUERY_INFO_NONE,
					 G_PRIORITY_DEFAULT,
					 NULL,
					 query_display_name_cb,
					 g_object_ref (buffer));
	}
}

static void
location_notify_cb (GtkSourceFile *file,
		    GParamSpec    *pspec,
		    GtefBuffer    *buffer)
{
	update_short_name (buffer);
}

static void
gtef_buffer_init (GtefBuffer *buffer)
{
	GtefBufferPrivate *priv;

	priv = gtef_buffer_get_instance_private (buffer);

	priv->file = gtef_file_new ();

	update_short_name (buffer);

	g_signal_connect_object (priv->file,
				 "notify::location",
				 G_CALLBACK (location_notify_cb),
				 buffer,
				 0);
}

/**
 * gtef_buffer_new:
 *
 * Returns: a new #GtefBuffer.
 * Since: 1.0
 */
GtefBuffer *
gtef_buffer_new (void)
{
	return g_object_new (GTEF_TYPE_BUFFER, NULL);
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

/**
 * gtef_buffer_is_untouched:
 * @buffer: a #GtefBuffer.
 *
 * Returns whether @buffer is untouched.
 *
 * This function is for example useful to know if we can re-use this buffer to
 * load a file, instead of opening a new tab or window.
 *
 * For this function to return %TRUE, the @buffer must be empty, non-modified,
 * the undo/redo #GtkSourceBuffer history must be empty, and the
 * #GtkSourceFile:location must be %NULL.
 *
 * Returns: %TRUE if @buffer has not been touched, %FALSE otherwise.
 * Since: 1.0
 */
gboolean
gtef_buffer_is_untouched (GtefBuffer *buffer)
{
	GtefBufferPrivate *priv;

	g_return_val_if_fail (GTEF_IS_BUFFER (buffer), FALSE);

	priv = gtef_buffer_get_instance_private (buffer);

	return (gtk_text_buffer_get_char_count (GTK_TEXT_BUFFER (buffer)) == 0 &&
		!gtk_text_buffer_get_modified (GTK_TEXT_BUFFER (buffer)) &&
		!gtk_source_buffer_can_undo (GTK_SOURCE_BUFFER (buffer)) &&
		!gtk_source_buffer_can_redo (GTK_SOURCE_BUFFER (buffer)) &&
		gtk_source_file_get_location (GTK_SOURCE_FILE (priv->file)) == NULL);
}

/**
 * gtef_buffer_get_short_name:
 * @buffer: a #GtefBuffer.
 *
 * Gets the @buffer short name. If the #GtkSourceFile:location isn't %NULL,
 * returns its display-name (see #G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME).
 * Otherwise returns "Untitled File".
 *
 * Returns: the @buffer short name.
 */
const gchar *
gtef_buffer_get_short_name (GtefBuffer *buffer)
{
	GtefBufferPrivate *priv;

	g_return_val_if_fail (GTEF_IS_BUFFER (buffer), NULL);

	priv = gtef_buffer_get_instance_private (buffer);
	return priv->short_name;
}

/**
 * gtef_buffer_get_title:
 * @buffer: a #GtefBuffer.
 *
 * Returns a title suitable for a #GtkWindow title. It contains (in that order):
 * - '*' if the buffer is modified;
 * - the short name as returned by gtef_buffer_get_short_name();
 * - the directory path in parenthesis if the #GtkSourceFile:location isn't
 *   %NULL.
 *
 * Returns: the @buffer title. Free the return value with g_free() when no
 * longer needed.
 * Since: 1.0
 */
gchar *
gtef_buffer_get_title (GtefBuffer *buffer)
{
	GtefBufferPrivate *priv;
	GFile *location;
	gchar *title;

	g_return_val_if_fail (GTEF_IS_BUFFER (buffer), NULL);

	priv = gtef_buffer_get_instance_private (buffer);

	location = gtk_source_file_get_location (GTK_SOURCE_FILE (priv->file));

	if (location == NULL)
	{
		title = g_strdup (priv->short_name);
	}
	else
	{
		GFile *parent;
		gchar *directory;

		parent = g_file_get_parent (location);
		g_return_val_if_fail (parent != NULL, NULL);

		directory = g_file_get_parse_name (parent);
		g_object_unref (parent);

		title = g_strdup_printf ("%s (%s)", priv->short_name, directory);
		g_free (directory);
	}

	if (gtk_text_buffer_get_modified (GTK_TEXT_BUFFER (buffer)))
	{
		gchar *full_title;

		full_title = g_strconcat ("*", title, NULL);
		g_free (title);

		return full_title;
	}

	return title;
}
