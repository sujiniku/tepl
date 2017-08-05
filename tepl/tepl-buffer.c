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

#include "tepl-buffer.h"
#include "tepl-file.h"
#include "tepl-utils.h"

/**
 * SECTION:buffer
 * @Short_description: Subclass of #GtkSourceBuffer
 * @Title: TeplBuffer
 *
 * #TeplBuffer is a subclass of #GtkSourceBuffer, to add more features useful
 * for a text editor.
 *
 * It also adds an association to a #TeplFile that can be retrieved with
 * tepl_buffer_get_file(). The association cannot change.
 *
 * The properties and signals have the tepl namespace, to avoid potential
 * conflicts in the future if the property or signal is moved to
 * #GtkSourceBuffer.
 */

typedef struct _TeplBufferPrivate TeplBufferPrivate;

struct _TeplBufferPrivate
{
	TeplFile *file;

	GtkTextTag *invalid_char_tag;

	guint n_nested_user_actions;
	guint idle_cursor_moved_id;
};

enum
{
	PROP_0,
	PROP_TEPL_SHORT_TITLE,
	PROP_TEPL_FULL_TITLE,
	PROP_TEPL_STYLE_SCHEME_ID,
	N_PROPERTIES
};

enum
{
	SIGNAL_TEPL_CURSOR_MOVED,
	N_SIGNALS
};

static GParamSpec *properties[N_PROPERTIES];
static guint signals[N_SIGNALS];

G_DEFINE_TYPE_WITH_PRIVATE (TeplBuffer, tepl_buffer, GTK_SOURCE_TYPE_BUFFER)

static void
update_invalid_char_tag_style (TeplBuffer *buffer)
{
	TeplBufferPrivate *priv;
	GtkSourceStyleScheme *style_scheme;
	GtkSourceStyle *style = NULL;

	priv = tepl_buffer_get_instance_private (buffer);

	if (priv->invalid_char_tag == NULL)
	{
		return;
	}

	style_scheme = gtk_source_buffer_get_style_scheme (GTK_SOURCE_BUFFER (buffer));

	if (style_scheme != NULL)
	{
		style = gtk_source_style_scheme_get_style (style_scheme, "def:error");
	}

	gtk_source_style_apply (style, priv->invalid_char_tag);
}

static void
tepl_buffer_get_property (GObject    *object,
			  guint       prop_id,
			  GValue     *value,
			  GParamSpec *pspec)
{
	TeplBuffer *buffer = TEPL_BUFFER (object);

	switch (prop_id)
	{
		case PROP_TEPL_SHORT_TITLE:
			g_value_take_string (value, tepl_buffer_get_short_title (buffer));
			break;

		case PROP_TEPL_FULL_TITLE:
			g_value_take_string (value, tepl_buffer_get_full_title (buffer));
			break;

		case PROP_TEPL_STYLE_SCHEME_ID:
			g_value_take_string (value, tepl_buffer_get_style_scheme_id (buffer));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_buffer_set_property (GObject      *object,
			  guint         prop_id,
			  const GValue *value,
			  GParamSpec   *pspec)
{
	TeplBuffer *buffer = TEPL_BUFFER (object);

	switch (prop_id)
	{
		case PROP_TEPL_STYLE_SCHEME_ID:
			tepl_buffer_set_style_scheme_id (buffer, g_value_get_string (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_buffer_dispose (GObject *object)
{
	TeplBufferPrivate *priv = tepl_buffer_get_instance_private (TEPL_BUFFER (object));

	g_clear_object (&priv->file);

	if (priv->idle_cursor_moved_id != 0)
	{
		g_source_remove (priv->idle_cursor_moved_id);
		priv->idle_cursor_moved_id = 0;
	}

	G_OBJECT_CLASS (tepl_buffer_parent_class)->dispose (object);
}

static gboolean
idle_cursor_moved_cb (gpointer user_data)
{
	TeplBuffer *buffer = TEPL_BUFFER (user_data);
	TeplBufferPrivate *priv = tepl_buffer_get_instance_private (buffer);

	g_signal_emit (buffer, signals[SIGNAL_TEPL_CURSOR_MOVED], 0);

	priv->idle_cursor_moved_id = 0;
	return G_SOURCE_REMOVE;
}

static void
install_idle_cursor_moved (TeplBuffer *buffer)
{
	TeplBufferPrivate *priv = tepl_buffer_get_instance_private (buffer);

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
tepl_buffer_begin_user_action (GtkTextBuffer *buffer)
{
	TeplBufferPrivate *priv = tepl_buffer_get_instance_private (TEPL_BUFFER (buffer));

	priv->n_nested_user_actions++;

	if (GTK_TEXT_BUFFER_CLASS (tepl_buffer_parent_class)->begin_user_action != NULL)
	{
		GTK_TEXT_BUFFER_CLASS (tepl_buffer_parent_class)->begin_user_action (buffer);
	}
}

static void
tepl_buffer_end_user_action (GtkTextBuffer *buffer)
{
	TeplBufferPrivate *priv = tepl_buffer_get_instance_private (TEPL_BUFFER (buffer));

	if (GTK_TEXT_BUFFER_CLASS (tepl_buffer_parent_class)->end_user_action != NULL)
	{
		GTK_TEXT_BUFFER_CLASS (tepl_buffer_parent_class)->end_user_action (buffer);
	}

	g_return_if_fail (priv->n_nested_user_actions > 0);
	priv->n_nested_user_actions--;

	if (priv->n_nested_user_actions == 0)
	{
		install_idle_cursor_moved (TEPL_BUFFER (buffer));
	}
}

static void
tepl_buffer_mark_set (GtkTextBuffer     *buffer,
		      const GtkTextIter *location,
		      GtkTextMark       *mark)
{
	TeplBufferPrivate *priv = tepl_buffer_get_instance_private (TEPL_BUFFER (buffer));

	if (GTK_TEXT_BUFFER_CLASS (tepl_buffer_parent_class)->mark_set != NULL)
	{
		GTK_TEXT_BUFFER_CLASS (tepl_buffer_parent_class)->mark_set (buffer, location, mark);
	}

	if (priv->n_nested_user_actions == 0 &&
	    mark == gtk_text_buffer_get_insert (buffer))
	{
		install_idle_cursor_moved (TEPL_BUFFER (buffer));
	}
}

static void
tepl_buffer_changed (GtkTextBuffer *buffer)
{
	TeplBufferPrivate *priv = tepl_buffer_get_instance_private (TEPL_BUFFER (buffer));

	if (GTK_TEXT_BUFFER_CLASS (tepl_buffer_parent_class)->changed != NULL)
	{
		GTK_TEXT_BUFFER_CLASS (tepl_buffer_parent_class)->changed (buffer);
	}

	if (priv->n_nested_user_actions == 0)
	{
		install_idle_cursor_moved (TEPL_BUFFER (buffer));
	}
}

static void
tepl_buffer_modified_changed (GtkTextBuffer *buffer)
{
	if (GTK_TEXT_BUFFER_CLASS (tepl_buffer_parent_class)->modified_changed != NULL)
	{
		GTK_TEXT_BUFFER_CLASS (tepl_buffer_parent_class)->modified_changed (buffer);
	}

	g_object_notify_by_pspec (G_OBJECT (buffer), properties[PROP_TEPL_SHORT_TITLE]);
	g_object_notify_by_pspec (G_OBJECT (buffer), properties[PROP_TEPL_FULL_TITLE]);
}

static void
tepl_buffer_class_init (TeplBufferClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkTextBufferClass *text_buffer_class = GTK_TEXT_BUFFER_CLASS (klass);

	object_class->get_property = tepl_buffer_get_property;
	object_class->set_property = tepl_buffer_set_property;
	object_class->dispose = tepl_buffer_dispose;

	text_buffer_class->begin_user_action = tepl_buffer_begin_user_action;
	text_buffer_class->end_user_action = tepl_buffer_end_user_action;
	text_buffer_class->mark_set = tepl_buffer_mark_set;
	text_buffer_class->changed = tepl_buffer_changed;
	text_buffer_class->modified_changed = tepl_buffer_modified_changed;

	/**
	 * TeplBuffer:tepl-short-title:
	 *
	 * The short title. See tepl_buffer_get_short_title().
	 *
	 * Since: 3.0
	 */
	properties[PROP_TEPL_SHORT_TITLE] =
		g_param_spec_string ("tepl-short-title",
				     "tepl-short-title",
				     "",
				     NULL,
				     G_PARAM_READABLE |
				     G_PARAM_STATIC_STRINGS);

	/**
	 * TeplBuffer:tepl-full-title:
	 *
	 * The full title. See tepl_buffer_get_full_title().
	 *
	 * Since: 3.0
	 */
	properties[PROP_TEPL_FULL_TITLE] =
		g_param_spec_string ("tepl-full-title",
				     "tepl-full-title",
				     "",
				     NULL,
				     G_PARAM_READABLE |
				     G_PARAM_STATIC_STRINGS);

	/**
	 * TeplBuffer:tepl-style-scheme-id:
	 *
	 * The #GtkSourceBuffer:style-scheme ID, as a string. This property is
	 * useful for binding it to a #GSettings key.
	 *
	 * When the #GtkSourceBuffer:style-scheme is %NULL,
	 * #TeplBuffer:tepl-style-scheme-id contains the empty string.
	 *
	 * Since: 2.0
	 */
	properties[PROP_TEPL_STYLE_SCHEME_ID] =
		g_param_spec_string ("tepl-style-scheme-id",
				     "Tepl Style Scheme ID",
				     "",
				     "",
				     G_PARAM_READWRITE |
				     G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, N_PROPERTIES, properties);

	/**
	 * TeplBuffer::tepl-cursor-moved:
	 * @buffer: the #TeplBuffer emitting the signal.
	 *
	 * The ::tepl-cursor-moved signal is emitted when the insert mark is
	 * moved explicitely or when the buffer changes (insert/delete).
	 *
	 * A typical use-case for this signal is to update the cursor position
	 * in a statusbar.
	 *
	 * Since: 2.0
	 */
	signals[SIGNAL_TEPL_CURSOR_MOVED] =
		g_signal_new ("tepl-cursor-moved",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (TeplBufferClass, tepl_cursor_moved),
			      NULL, NULL, NULL,
			      G_TYPE_NONE, 0);
}

static void
file_short_name_notify_cb (TeplFile   *file,
			   GParamSpec *pspec,
			   TeplBuffer *buffer)
{
	g_object_notify_by_pspec (G_OBJECT (buffer), properties[PROP_TEPL_SHORT_TITLE]);
	g_object_notify_by_pspec (G_OBJECT (buffer), properties[PROP_TEPL_FULL_TITLE]);
}

static void
style_scheme_notify_cb (GtkSourceBuffer *buffer,
			GParamSpec      *pspec,
			gpointer         user_data)
{
	update_invalid_char_tag_style (TEPL_BUFFER (buffer));

	g_object_notify_by_pspec (G_OBJECT (buffer), properties[PROP_TEPL_STYLE_SCHEME_ID]);
}

static void
tepl_buffer_init (TeplBuffer *buffer)
{
	TeplBufferPrivate *priv;

	priv = tepl_buffer_get_instance_private (buffer);

	priv->file = tepl_file_new ();

	g_signal_connect_object (priv->file,
				 "notify::short-name",
				 G_CALLBACK (file_short_name_notify_cb),
				 buffer,
				 0);

	g_signal_connect (buffer,
			  "notify::style-scheme",
			  G_CALLBACK (style_scheme_notify_cb),
			  NULL);
}

/**
 * tepl_buffer_new:
 *
 * Returns: a new #TeplBuffer.
 * Since: 1.0
 */
TeplBuffer *
tepl_buffer_new (void)
{
	return g_object_new (TEPL_TYPE_BUFFER, NULL);
}

/**
 * tepl_buffer_get_file:
 * @buffer: a #TeplBuffer.
 *
 * Returns the #TeplFile of @buffer. The returned object is guaranteed to be the
 * same for the lifetime of @buffer.
 *
 * Returns: (transfer none): the associated #TeplFile.
 * Since: 1.0
 */
TeplFile *
tepl_buffer_get_file (TeplBuffer *buffer)
{
	TeplBufferPrivate *priv;

	g_return_val_if_fail (TEPL_IS_BUFFER (buffer), NULL);

	priv = tepl_buffer_get_instance_private (buffer);
	return priv->file;
}

/**
 * tepl_buffer_is_untouched:
 * @buffer: a #TeplBuffer.
 *
 * Returns whether @buffer is untouched.
 *
 * This function is for example useful to know if we can re-use this buffer to
 * load a file, instead of opening a new tab or window.
 *
 * For this function to return %TRUE, the @buffer must be empty, non-modified,
 * the undo/redo #GtkSourceBuffer history must be empty, and the
 * #TeplFile:location must be %NULL.
 *
 * Returns: %TRUE if @buffer has not been touched, %FALSE otherwise.
 * Since: 1.0
 */
gboolean
tepl_buffer_is_untouched (TeplBuffer *buffer)
{
	TeplBufferPrivate *priv;

	g_return_val_if_fail (TEPL_IS_BUFFER (buffer), FALSE);

	priv = tepl_buffer_get_instance_private (buffer);

	return (gtk_text_buffer_get_char_count (GTK_TEXT_BUFFER (buffer)) == 0 &&
		!gtk_text_buffer_get_modified (GTK_TEXT_BUFFER (buffer)) &&
		!gtk_source_buffer_can_undo (GTK_SOURCE_BUFFER (buffer)) &&
		!gtk_source_buffer_can_redo (GTK_SOURCE_BUFFER (buffer)) &&
		tepl_file_get_location (priv->file) == NULL);
}

/**
 * tepl_buffer_get_short_title:
 * @buffer: a #TeplBuffer.
 *
 * Returns a title suitable for a tab label. It contains (in that order):
 * - '*' if the buffer is modified;
 * - the #TeplFile:short-name;
 *
 * Returns: the @buffer short title. Free the return value with g_free() when no
 * longer needed.
 * Since: 3.0
 */
gchar *
tepl_buffer_get_short_title (TeplBuffer *buffer)
{
	TeplBufferPrivate *priv;
	const gchar *short_name;
	gchar *short_title;

	g_return_val_if_fail (TEPL_IS_BUFFER (buffer), NULL);

	priv = tepl_buffer_get_instance_private (buffer);

	short_name = tepl_file_get_short_name (priv->file);

	if (gtk_text_buffer_get_modified (GTK_TEXT_BUFFER (buffer)))
	{
		short_title = g_strconcat ("*", short_name, NULL);
	}
	else
	{
		short_title = g_strdup (short_name);
	}

	return short_title;
}

/**
 * tepl_buffer_get_full_title:
 * @buffer: a #TeplBuffer.
 *
 * Returns a title suitable for a #GtkWindow title. It contains (in that order):
 * - the #TeplBuffer:tepl-short-title;
 * - the directory path in parenthesis if the #TeplFile:location isn't
 *   %NULL.
 *
 * Returns: the @buffer full title. Free the return value with g_free() when no
 * longer needed.
 * Since: 3.0
 */
gchar *
tepl_buffer_get_full_title (TeplBuffer *buffer)
{
	TeplBufferPrivate *priv;
	GFile *location;
	gchar *short_title;
	gchar *full_title;

	g_return_val_if_fail (TEPL_IS_BUFFER (buffer), NULL);

	priv = tepl_buffer_get_instance_private (buffer);

	location = tepl_file_get_location (priv->file);
	short_title = tepl_buffer_get_short_title (buffer);

	if (location != NULL &&
	    g_file_has_parent (location, NULL))
	{
		GFile *parent;
		gchar *directory;
		gchar *directory_tilde;

		parent = g_file_get_parent (location);
		directory = g_file_get_parse_name (parent);
		directory_tilde = _tepl_utils_replace_home_dir_with_tilde (directory);

		full_title = g_strdup_printf ("%s (%s)", short_title, directory_tilde);
		g_free (short_title);

		g_object_unref (parent);
		g_free (directory);
		g_free (directory_tilde);
	}
	else
	{
		full_title = short_title;
	}

	return full_title;
}

/**
 * tepl_buffer_get_style_scheme_id:
 * @buffer: a #TeplBuffer.
 *
 * Returns: the #TeplBuffer:tepl-style-scheme-id. Free with g_free().
 * Since: 2.0
 */
gchar *
tepl_buffer_get_style_scheme_id (TeplBuffer *buffer)
{
	GtkSourceStyleScheme *style_scheme;
	const gchar *id;

	g_return_val_if_fail (TEPL_IS_BUFFER (buffer), g_strdup (""));

	style_scheme = gtk_source_buffer_get_style_scheme (GTK_SOURCE_BUFFER (buffer));

	if (style_scheme == NULL)
	{
		return g_strdup ("");
	}

	id = gtk_source_style_scheme_get_id (style_scheme);

	return id != NULL ? g_strdup (id) : g_strdup ("");
}

/**
 * tepl_buffer_set_style_scheme_id:
 * @buffer: a #TeplBuffer.
 * @style_scheme_id: the new value.
 *
 * Sets the #TeplBuffer:tepl-style-scheme-id property.
 *
 * The #GtkSourceStyleScheme is taken from the default
 * #GtkSourceStyleSchemeManager as returned by
 * gtk_source_style_scheme_manager_get_default().
 *
 * Since: 2.0
 */
void
tepl_buffer_set_style_scheme_id (TeplBuffer  *buffer,
				 const gchar *style_scheme_id)
{
	GtkSourceStyleSchemeManager *manager;
	GtkSourceStyleScheme *style_scheme;

	g_return_if_fail (TEPL_IS_BUFFER (buffer));
	g_return_if_fail (style_scheme_id != NULL);

	manager = gtk_source_style_scheme_manager_get_default ();
	style_scheme = gtk_source_style_scheme_manager_get_scheme (manager, style_scheme_id);
	gtk_source_buffer_set_style_scheme (GTK_SOURCE_BUFFER (buffer), style_scheme);
}

/**
 * tepl_buffer_get_selection_type:
 * @buffer: a #TeplBuffer.
 *
 * Returns: the current #TeplSelectionType.
 * Since: 1.0
 */
TeplSelectionType
tepl_buffer_get_selection_type (TeplBuffer *buffer)
{
	GtkTextIter start;
	GtkTextIter end;
	gint start_line;
	gint end_line;

	g_return_val_if_fail (TEPL_IS_BUFFER (buffer), TEPL_SELECTION_TYPE_NO_SELECTION);

	if (!gtk_text_buffer_get_selection_bounds (GTK_TEXT_BUFFER (buffer), &start, &end))
	{
		return TEPL_SELECTION_TYPE_NO_SELECTION;
	}

	start_line = gtk_text_iter_get_line (&start);
	end_line = gtk_text_iter_get_line (&end);

	if (start_line == end_line)
	{
		return TEPL_SELECTION_TYPE_ON_SAME_LINE;
	}

	return TEPL_SELECTION_TYPE_MULTIPLE_LINES;
}

static void
text_tag_set_highest_priority (GtkTextTag    *tag,
			       GtkTextBuffer *buffer)
{
	GtkTextTagTable *table;
	gint n;

	table = gtk_text_buffer_get_tag_table (buffer);
	n = gtk_text_tag_table_get_size (table);
	gtk_text_tag_set_priority (tag, n - 1);
}

void
_tepl_buffer_set_as_invalid_character (TeplBuffer        *buffer,
				       const GtkTextIter *start,
				       const GtkTextIter *end)
{
	TeplBufferPrivate *priv;

	g_return_if_fail (TEPL_IS_BUFFER (buffer));
	g_return_if_fail (start != NULL);
	g_return_if_fail (end != NULL);

	priv = tepl_buffer_get_instance_private (buffer);

	if (priv->invalid_char_tag == NULL)
	{
		priv->invalid_char_tag = gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (buffer),
								     NULL,
								     NULL);

		update_invalid_char_tag_style (buffer);
	}

	/* Make sure the 'error' tag has the priority over
	 * syntax highlighting tags.
	 */
	text_tag_set_highest_priority (priv->invalid_char_tag,
	                               GTK_TEXT_BUFFER (buffer));

	gtk_text_buffer_apply_tag (GTK_TEXT_BUFFER (buffer),
	                           priv->invalid_char_tag,
	                           start,
	                           end);
}

gboolean
_tepl_buffer_has_invalid_chars (TeplBuffer *buffer)
{
	TeplBufferPrivate *priv;
	GtkTextIter start;

	g_return_val_if_fail (TEPL_IS_BUFFER (buffer), FALSE);

	priv = tepl_buffer_get_instance_private (buffer);

	if (priv->invalid_char_tag == NULL)
	{
		return FALSE;
	}

	gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (buffer), &start);

	if (gtk_text_iter_starts_tag (&start, priv->invalid_char_tag) ||
	    gtk_text_iter_forward_to_tag_toggle (&start, priv->invalid_char_tag))
	{
		return TRUE;
	}

	return FALSE;
}
