/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2016, 2017 - Sébastien Wilmet <swilmet@gnome.org>
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

#include "config.h"
#include "gtef-file-loader.h"
#include <uchardet.h>
#include <glib/gi18n-lib.h>
#include "gtef-buffer.h"
#include "gtef-file.h"
#include "gtef-file-content-loader.h"
#include "gtef-encoding.h"
#include "gtef-encoding-converter.h"

/**
 * SECTION:file-loader
 * @Short_description: Load a file into a GtefBuffer
 * @Title: GtefFileLoader
 * @See_also: #GtefFile, #GtefFileSaver
 *
 * #GtefFileLoader is not a fork of #GtkSourceFileLoader, it is a new
 * implementation based on
 * [uchardet](https://www.freedesktop.org/wiki/Software/uchardet/).
 *
 * A #GtefFileLoader object permits to load the content of a #GFile into a
 * #GtefBuffer.
 *
 * A file loader should be used only for one load operation, including errors
 * handling. If an error occurs, you can reconfigure the loader and relaunch the
 * operation with gtef_file_loader_load_async().
 *
 * Running a #GtefFileLoader is an undoable action for the #GtefBuffer. That is,
 * gtk_source_buffer_begin_not_undoable_action() and
 * gtk_source_buffer_end_not_undoable_action() are called, which delete the
 * undo/redo history.
 *
 * After a file loading, the buffer is reset to the content provided by the
 * #GFile, so the buffer is set as “unmodified”, that is,
 * gtk_text_buffer_set_modified() is called with %FALSE.
 */

typedef struct _GtefFileLoaderPrivate GtefFileLoaderPrivate;
typedef struct _TaskData TaskData;

struct _GtefFileLoaderPrivate
{
	/* Weak ref to the GtefBuffer. A strong ref could create a reference
	 * cycle in an application. For example a subclass of GtefBuffer can
	 * have a strong ref to the FileLoader.
	 */
	GtefBuffer *buffer;

	/* Weak ref to the GtefFile. A strong ref could create a reference
	 * cycle in an application. For example a subclass of GtefFile can
	 * have a strong ref to the FileLoader.
	 */
	GtefFile *file;

	GFile *location;
	gint64 max_size;
	gint64 chunk_size;
	GTask *task;

	GtefEncoding *detected_encoding;
	GtefNewlineType detected_newline_type;
};

struct _TaskData
{
	GtefFileContentLoader *content_loader;

	/* TODO report progress also when determining encoding, and when
	 * converting and inserting the content.
	 */
	GFileProgressCallback progress_cb;
	gpointer progress_cb_data;
	GDestroyNotify progress_cb_notify;

	guint tried_mount : 1;

	/* Whether the next char to insert in the GtkTextBuffer is a carriage
	 * return. If it is followed by a newline, the \r\n must be inserted in
	 * one block, because of a bug in GtkTextBuffer:
	 * https://bugzilla.gnome.org/show_bug.cgi?id=631468
	 */
	guint insert_carriage_return : 1;
};

enum
{
	PROP_0,
	PROP_BUFFER,
	PROP_FILE,
	PROP_LOCATION,
	PROP_MAX_SIZE,
	PROP_CHUNK_SIZE,
	N_PROPERTIES
};

/* Take the default buffer-size of GtefEncodingConverter. */
#define ENCODING_CONVERTER_BUFFER_SIZE (-1)

static GParamSpec *properties[N_PROPERTIES];

G_DEFINE_TYPE_WITH_PRIVATE (GtefFileLoader, gtef_file_loader, G_TYPE_OBJECT)

/* Prototypes */
static void load_content (GTask *task);

GQuark
gtef_file_loader_error_quark (void)
{
	static GQuark quark = 0;

	if (G_UNLIKELY (quark == 0))
	{
		quark = g_quark_from_static_string ("gtef-file-loader-error");
	}

	return quark;
}

static TaskData *
task_data_new (void)
{
	return g_new0 (TaskData, 1);
}

static void
task_data_free (gpointer data)
{
	TaskData *task_data = data;

	if (task_data == NULL)
	{
		return;
	}

	g_clear_object (&task_data->content_loader);

	if (task_data->progress_cb_notify != NULL)
	{
		task_data->progress_cb_notify (task_data->progress_cb_data);
	}

	g_free (task_data);
}

static void
empty_buffer (GtefFileLoader *loader)
{
	GtefFileLoaderPrivate *priv;

	priv = gtef_file_loader_get_instance_private (loader);

	if (priv->buffer != NULL)
	{
		gtk_text_buffer_set_text (GTK_TEXT_BUFFER (priv->buffer), "", -1);
	}
}

static void
detect_newline_type (GtefFileLoader *loader)
{
	GtefFileLoaderPrivate *priv;
	GtkTextIter iter;
	gunichar first_char;

	priv = gtef_file_loader_get_instance_private (loader);

	if (priv->buffer == NULL)
	{
		priv->detected_newline_type = GTEF_NEWLINE_TYPE_DEFAULT;
		return;
	}

	gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (priv->buffer), &iter);
	if (!gtk_text_iter_ends_line (&iter))
	{
		gtk_text_iter_forward_to_line_end (&iter);
	}

	first_char = gtk_text_iter_get_char (&iter);

	if (first_char == '\n')
	{
		priv->detected_newline_type = GTEF_NEWLINE_TYPE_LF;
	}
	else if (first_char == '\r')
	{
		gunichar second_char;

		gtk_text_iter_forward_char (&iter);
		second_char = gtk_text_iter_get_char (&iter);

		if (second_char == '\n')
		{
			priv->detected_newline_type = GTEF_NEWLINE_TYPE_CR_LF;
		}
		else
		{
			priv->detected_newline_type = GTEF_NEWLINE_TYPE_CR;
		}
	}
	else
	{
		priv->detected_newline_type = GTEF_NEWLINE_TYPE_DEFAULT;
	}
}

static void
remove_trailing_newline_if_needed (GtefFileLoader *loader)
{
	GtefFileLoaderPrivate *priv;
	GtkTextIter start;
	GtkTextIter end;

	priv = gtef_file_loader_get_instance_private (loader);

	if (priv->buffer == NULL)
	{
		return;
	}

	if (!gtk_source_buffer_get_implicit_trailing_newline (GTK_SOURCE_BUFFER (priv->buffer)))
	{
		return;
	}

	gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (priv->buffer), &end);
	start = end;

	gtk_text_iter_set_line_offset (&start, 0);

	if (gtk_text_iter_ends_line (&start) &&
	    gtk_text_iter_backward_line (&start))
	{
		if (!gtk_text_iter_ends_line (&start))
		{
			gtk_text_iter_forward_to_line_end (&start);
		}

		gtk_text_buffer_delete (GTK_TEXT_BUFFER (priv->buffer),
					&start,
					&end);
	}
}

static void
gtef_file_loader_get_property (GObject    *object,
			       guint       prop_id,
			       GValue     *value,
			       GParamSpec *pspec)
{
	GtefFileLoader *loader = GTEF_FILE_LOADER (object);

	switch (prop_id)
	{
		case PROP_BUFFER:
			g_value_set_object (value, gtef_file_loader_get_buffer (loader));
			break;

		case PROP_FILE:
			g_value_set_object (value, gtef_file_loader_get_file (loader));
			break;

		case PROP_LOCATION:
			g_value_set_object (value, gtef_file_loader_get_location (loader));
			break;

		case PROP_MAX_SIZE:
			g_value_set_int64 (value, gtef_file_loader_get_max_size (loader));
			break;

		case PROP_CHUNK_SIZE:
			g_value_set_int64 (value, gtef_file_loader_get_chunk_size (loader));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gtef_file_loader_set_property (GObject      *object,
			       guint         prop_id,
			       const GValue *value,
			       GParamSpec   *pspec)
{
	GtefFileLoader *loader = GTEF_FILE_LOADER (object);
	GtefFileLoaderPrivate *priv = gtef_file_loader_get_instance_private (loader);

	switch (prop_id)
	{
		case PROP_BUFFER:
			g_assert (priv->buffer == NULL);
			priv->buffer = g_value_get_object (value);
			g_object_add_weak_pointer (G_OBJECT (priv->buffer),
						   (gpointer *) &priv->buffer);
			break;

		case PROP_FILE:
			g_assert (priv->file == NULL);
			priv->file = g_value_get_object (value);
			g_object_add_weak_pointer (G_OBJECT (priv->file),
						   (gpointer *) &priv->file);
			break;

		case PROP_LOCATION:
			g_assert (priv->location == NULL);
			priv->location = g_value_dup_object (value);
			break;

		case PROP_MAX_SIZE:
			gtef_file_loader_set_max_size (loader, g_value_get_int64 (value));
			break;

		case PROP_CHUNK_SIZE:
			gtef_file_loader_set_chunk_size (loader, g_value_get_int64 (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gtef_file_loader_constructed (GObject *object)
{
	GtefFileLoaderPrivate *priv = gtef_file_loader_get_instance_private (GTEF_FILE_LOADER (object));

	G_OBJECT_CLASS (gtef_file_loader_parent_class)->constructed (object);

	if (priv->file != NULL &&
	    priv->location == NULL)
	{
		priv->location = gtef_file_get_location (priv->file);

		if (priv->location != NULL)
		{
			g_object_ref (priv->location);
		}
		else
		{
			g_warning ("GtefFileLoader: the GtefFile location is NULL. "
				   "Call gtef_file_set_location() before creating the FileLoader.");
		}
	}
}

static void
gtef_file_loader_dispose (GObject *object)
{
	GtefFileLoaderPrivate *priv = gtef_file_loader_get_instance_private (GTEF_FILE_LOADER (object));

	if (priv->buffer != NULL)
	{
		g_object_remove_weak_pointer (G_OBJECT (priv->buffer),
					      (gpointer *) &priv->buffer);
		priv->buffer = NULL;
	}

	if (priv->file != NULL)
	{
		g_object_remove_weak_pointer (G_OBJECT (priv->file),
					      (gpointer *) &priv->file);
		priv->file = NULL;
	}

	g_clear_object (&priv->location);
	g_clear_object (&priv->task);

	G_OBJECT_CLASS (gtef_file_loader_parent_class)->dispose (object);
}

static void
gtef_file_loader_finalize (GObject *object)
{
	GtefFileLoaderPrivate *priv = gtef_file_loader_get_instance_private (GTEF_FILE_LOADER (object));

	gtef_encoding_free (priv->detected_encoding);

	G_OBJECT_CLASS (gtef_file_loader_parent_class)->finalize (object);
}

static void
gtef_file_loader_class_init (GtefFileLoaderClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gtef_file_loader_get_property;
	object_class->set_property = gtef_file_loader_set_property;
	object_class->constructed = gtef_file_loader_constructed;
	object_class->dispose = gtef_file_loader_dispose;
	object_class->finalize = gtef_file_loader_finalize;

	/**
	 * GtefFileLoader:buffer:
	 *
	 * The #GtefBuffer to load the content into. The #GtefFileLoader object
	 * has a weak reference to the buffer.
	 *
	 * Since: 1.0
	 */
	properties[PROP_BUFFER] =
		g_param_spec_object ("buffer",
				     "GtefBuffer",
				     "",
				     GTEF_TYPE_BUFFER,
				     G_PARAM_READWRITE |
				     G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS);

	/**
	 * GtefFileLoader:file:
	 *
	 * The #GtefFile. The #GtefFileLoader object has a weak
	 * reference to the file.
	 *
	 * Since: 1.0
	 */
	properties[PROP_FILE] =
		g_param_spec_object ("file",
				     "GtefFile",
				     "",
				     GTEF_TYPE_FILE,
				     G_PARAM_READWRITE |
				     G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS);

	/**
	 * GtefFileLoader:location:
	 *
	 * The #GFile to load. By default the location is taken from the
	 * #GtefFile at construction time.
	 *
	 * Since: 1.0
	 */
	properties[PROP_LOCATION] =
		g_param_spec_object ("location",
				     "Location",
				     "",
				     G_TYPE_FILE,
				     G_PARAM_READWRITE |
				     G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS);

	/**
	 * GtefFileLoader:max-size:
	 *
	 * The maximum content size, in bytes. Keep in mind that all the
	 * content is loaded in memory, and when loaded into a #GtkTextBuffer
	 * it takes more memory than just the content size.
	 *
	 * Set to -1 for unlimited size.
	 *
	 * Since: 1.0
	 */
	properties[PROP_MAX_SIZE] =
		g_param_spec_int64 ("max-size",
				    "Max Size",
				    "",
				    -1,
				    G_MAXINT64,
				    GTEF_FILE_CONTENT_LOADER_DEFAULT_MAX_SIZE,
				    G_PARAM_READWRITE |
				    G_PARAM_CONSTRUCT |
				    G_PARAM_STATIC_STRINGS);

	/**
	 * GtefFileLoader:chunk-size:
	 *
	 * The chunk size, in bytes. The content is loaded chunk by chunk. It
	 * permits to avoid allocating a too big contiguous memory area, as well
	 * as reporting progress information after each chunk read.
	 *
	 * A small chunk size is better when loading a remote file with a slow
	 * connection. For local files, the chunk size can be larger.
	 *
	 * Since: 1.0
	 */
	properties[PROP_CHUNK_SIZE] =
		g_param_spec_int64 ("chunk-size",
				    "Chunk Size",
				    "",
				    1,
				    G_MAXINT64,
				    GTEF_FILE_CONTENT_LOADER_DEFAULT_CHUNK_SIZE,
				    G_PARAM_READWRITE |
				    G_PARAM_CONSTRUCT |
				    G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
gtef_file_loader_init (GtefFileLoader *loader)
{
	GtefFileLoaderPrivate *priv;

	priv = gtef_file_loader_get_instance_private (loader);

	priv->detected_newline_type = GTEF_NEWLINE_TYPE_DEFAULT;
}

/**
 * gtef_file_loader_new:
 * @buffer: the #GtefBuffer to load the content into.
 * @file: the #GtefFile.
 *
 * Creates a new #GtefFileLoader object. The content is read from the #GtefFile
 * location.
 *
 * If not already done, call gtef_file_set_location() before calling this
 * constructor. The previous location is anyway not needed, because as soon as
 * the file loading begins, the @buffer is emptied. Setting the #GtefFile
 * location directly permits to update the UI, to display the good location when
 * the file is loading.
 *
 * Returns: a new #GtefFileLoader object.
 * Since: 1.0
 */
GtefFileLoader *
gtef_file_loader_new (GtefBuffer *buffer,
		      GtefFile   *file)
{
	g_return_val_if_fail (GTEF_IS_BUFFER (buffer), NULL);
	g_return_val_if_fail (GTEF_IS_FILE (file), NULL);

	return g_object_new (GTEF_TYPE_FILE_LOADER,
			     "buffer", buffer,
			     "file", file,
			     NULL);
}

/**
 * gtef_file_loader_get_buffer:
 * @loader: a #GtefFileLoader.
 *
 * Returns: (transfer none) (nullable): the #GtefBuffer to load the content
 * into.
 * Since: 1.0
 */
GtefBuffer *
gtef_file_loader_get_buffer (GtefFileLoader *loader)
{
	GtefFileLoaderPrivate *priv;

	g_return_val_if_fail (GTEF_IS_FILE_LOADER (loader), NULL);

	priv = gtef_file_loader_get_instance_private (loader);
	return priv->buffer;
}

/**
 * gtef_file_loader_get_file:
 * @loader: a #GtefFileLoader.
 *
 * Returns: (transfer none) (nullable): the #GtefFile.
 * Since: 1.0
 */
GtefFile *
gtef_file_loader_get_file (GtefFileLoader *loader)
{
	GtefFileLoaderPrivate *priv;

	g_return_val_if_fail (GTEF_IS_FILE_LOADER (loader), NULL);

	priv = gtef_file_loader_get_instance_private (loader);
	return priv->file;
}

/**
 * gtef_file_loader_get_location:
 * @loader: a #GtefFileLoader.
 *
 * Returns: (transfer none) (nullable): the #GFile to load.
 * Since: 1.0
 */
GFile *
gtef_file_loader_get_location (GtefFileLoader *loader)
{
	GtefFileLoaderPrivate *priv;

	g_return_val_if_fail (GTEF_IS_FILE_LOADER (loader), NULL);

	priv = gtef_file_loader_get_instance_private (loader);
	return priv->location;
}

/**
 * gtef_file_loader_get_max_size:
 * @loader: a #GtefFileLoader.
 *
 * Returns: the maximum content size, or -1 for unlimited.
 * Since: 1.0
 */
gint64
gtef_file_loader_get_max_size (GtefFileLoader *loader)
{
	GtefFileLoaderPrivate *priv;

	g_return_val_if_fail (GTEF_IS_FILE_LOADER (loader),
			      GTEF_FILE_CONTENT_LOADER_DEFAULT_MAX_SIZE);

	priv = gtef_file_loader_get_instance_private (loader);
	return priv->max_size;
}

/**
 * gtef_file_loader_set_max_size:
 * @loader: a #GtefFileLoader.
 * @max_size: the new maximum size, or -1 for unlimited.
 *
 * Since: 1.0
 */
void
gtef_file_loader_set_max_size (GtefFileLoader *loader,
			       gint64          max_size)
{
	GtefFileLoaderPrivate *priv;

	g_return_if_fail (GTEF_IS_FILE_LOADER (loader));
	g_return_if_fail (max_size >= -1);

	priv = gtef_file_loader_get_instance_private (loader);

	g_return_if_fail (priv->task == NULL);

	if (priv->max_size != max_size)
	{
		priv->max_size = max_size;
		g_object_notify_by_pspec (G_OBJECT (loader), properties[PROP_MAX_SIZE]);
	}
}

/**
 * gtef_file_loader_get_chunk_size:
 * @loader: a #GtefFileLoader.
 *
 * Returns: the chunk size.
 * Since: 1.0
 */
gint64
gtef_file_loader_get_chunk_size (GtefFileLoader *loader)
{
	GtefFileLoaderPrivate *priv;

	g_return_val_if_fail (GTEF_IS_FILE_LOADER (loader),
			      GTEF_FILE_CONTENT_LOADER_DEFAULT_CHUNK_SIZE);

	priv = gtef_file_loader_get_instance_private (loader);
	return priv->chunk_size;
}

/**
 * gtef_file_loader_set_chunk_size:
 * @loader: a #GtefFileLoader.
 * @chunk_size: the new chunk size.
 *
 * Since: 1.0
 */
void
gtef_file_loader_set_chunk_size (GtefFileLoader *loader,
				 gint64          chunk_size)
{
	GtefFileLoaderPrivate *priv;

	g_return_if_fail (GTEF_IS_FILE_LOADER (loader));
	g_return_if_fail (chunk_size >= 1);

	priv = gtef_file_loader_get_instance_private (loader);

	if (priv->chunk_size == chunk_size)
	{
		return;
	}

	priv->chunk_size = chunk_size;

	if (priv->task != NULL)
	{
		TaskData *task_data;

		task_data = g_task_get_task_data (priv->task);

		if (task_data->content_loader != NULL)
		{
			_gtef_file_content_loader_set_chunk_size (task_data->content_loader,
								  chunk_size);
		}
	}

	g_object_notify_by_pspec (G_OBJECT (loader), properties[PROP_CHUNK_SIZE]);
}

static void
insert_content (GtkTextBuffer *buffer,
		const gchar   *str,
		gsize          length)
{
	GtkTextIter end;
	GtkTextIter start;

	gtk_text_buffer_get_end_iter (buffer, &end);
	gtk_text_buffer_insert (buffer, &end, str, length);

	/* Keep cursor at the start, to avoid signal emissions for each chunk. */
	gtk_text_buffer_get_start_iter (buffer, &start);
	gtk_text_buffer_place_cursor (buffer, &start);
}

static void
content_converted_cb (const gchar *str,
		      gsize        length,
		      gpointer     user_data)
{
	GTask *task = G_TASK (user_data);
	GtefFileLoader *loader;
	GtefFileLoaderPrivate *priv;
	TaskData *task_data;
	gchar *my_str;
	gsize my_length;

	loader = g_task_get_source_object (task);
	priv = gtef_file_loader_get_instance_private (loader);

	task_data = g_task_get_task_data (task);

	/* I normally know what I'm doing. */
	my_str = (gchar *) str;
	my_length = length;

	/* Insert \r\n in one block. */
	if (task_data->insert_carriage_return)
	{
		if (my_str[0] == '\n')
		{
			g_assert (my_length > 0);

			insert_content (GTK_TEXT_BUFFER (priv->buffer), "\r\n", 2);
			my_str++;
			my_length--;
		}
		else
		{
			insert_content (GTK_TEXT_BUFFER (priv->buffer), "\r", 1);
		}

		task_data->insert_carriage_return = FALSE;
	}

	if (my_length == 0)
	{
		return;
	}

	if (my_str[my_length - 1] == '\r')
	{
		my_str[my_length - 1] = '\0';
		my_length--;

		/* Insert \r the next time. */
		task_data->insert_carriage_return = TRUE;
	}

	if (my_length != 0)
	{
		insert_content (GTK_TEXT_BUFFER (priv->buffer), my_str, my_length);
	}
}

static void
convert_and_insert_content (GTask *task)
{
	GtefFileLoader *loader;
	GtefFileLoaderPrivate *priv;
	TaskData *task_data;
	GtefEncodingConverter *converter = NULL;
	GQueue *content;
	GList *l;
	GError *error = NULL;

	loader = g_task_get_source_object (task);
	priv = gtef_file_loader_get_instance_private (loader);

	task_data = g_task_get_task_data (task);

	if (priv->buffer == NULL)
	{
		g_task_return_boolean (task, FALSE);
		return;
	}

	converter = _gtef_encoding_converter_new (ENCODING_CONVERTER_BUFFER_SIZE);

	_gtef_encoding_converter_set_callback (converter,
					       content_converted_cb,
					       task);

	g_assert (priv->detected_encoding != NULL);
	_gtef_encoding_converter_open (converter,
				       "UTF-8",
				       gtef_encoding_get_charset (priv->detected_encoding),
				       &error);
	if (error != NULL)
	{
		g_task_return_error (task, error);
		goto out;
	}

	content = _gtef_file_content_loader_get_content (task_data->content_loader);

	for (l = content->head; l != NULL; l = l->next)
	{
		GBytes *chunk = l->data;

		g_assert (chunk != NULL);
		g_assert (g_bytes_get_size (chunk) > 0);

		_gtef_encoding_converter_feed (converter,
					       g_bytes_get_data (chunk, NULL),
					       g_bytes_get_size (chunk),
					       &error);

		if (error != NULL)
		{
			g_task_return_error (task, error);
			goto out;
		}
	}

	_gtef_encoding_converter_close (converter, &error);
	if (error != NULL)
	{
		g_task_return_error (task, error);
		goto out;
	}

	if (task_data->insert_carriage_return)
	{
		insert_content (GTK_TEXT_BUFFER (priv->buffer), "\r", 1);
		task_data->insert_carriage_return = FALSE;
	}

	/* The order is important here: if the buffer contains only one line, we
	 * must remove the trailing newline *after* detecting the newline type.
	 */
	detect_newline_type (loader);
	remove_trailing_newline_if_needed (loader);

	g_task_return_boolean (task, TRUE);

out:
	g_clear_object (&converter);
}

static void
determine_encoding (GTask *task)
{
	GtefFileLoader *loader;
	GtefFileLoaderPrivate *priv;
	TaskData *task_data;
	uchardet_t ud;
	const gchar *charset;
	GQueue *content;
	GList *l;

	loader = g_task_get_source_object (task);
	priv = gtef_file_loader_get_instance_private (loader);

	task_data = g_task_get_task_data (task);

	ud = uchardet_new ();

	content = _gtef_file_content_loader_get_content (task_data->content_loader);

	for (l = content->head; l != NULL; l = l->next)
	{
		GBytes *chunk = l->data;

		g_assert (chunk != NULL);
		g_assert (g_bytes_get_size (chunk) > 0);

		uchardet_handle_data (ud,
				      g_bytes_get_data (chunk, NULL),
				      g_bytes_get_size (chunk));
	}

	uchardet_data_end (ud);

	/* reset() must have been called before launching the task. */
	g_assert (priv->detected_encoding == NULL);

	charset = uchardet_get_charset (ud);
	if (charset != NULL && charset[0] != '\0')
	{
		priv->detected_encoding = gtef_encoding_new (charset);
	}

	uchardet_delete (ud);

	if (priv->detected_encoding == NULL)
	{
		g_task_return_new_error (task,
					 GTEF_FILE_LOADER_ERROR,
					 GTEF_FILE_LOADER_ERROR_ENCODING_AUTO_DETECTION_FAILED,
					 _("It is not possible to detect the character encoding automatically."));
		return;
	}

	convert_and_insert_content (task);
}

static void
mount_cb (GObject      *source_object,
	  GAsyncResult *result,
	  gpointer      user_data)
{
	GFile *location = G_FILE (source_object);
	GTask *task = G_TASK (user_data);
	GtefFileLoader *loader;
	GtefFileLoaderPrivate *priv;
	GError *error = NULL;

	loader = g_task_get_source_object (task);
	priv = gtef_file_loader_get_instance_private (loader);

	g_file_mount_enclosing_volume_finish (location, result, &error);

	if (error != NULL)
	{
		g_task_return_error (task, error);
	}
	else
	{
		if (priv->file != NULL)
		{
			_gtef_file_set_mounted (priv->file);
		}

		/* Try again the previous operation. */
		load_content (task);
	}
}

static void
recover_not_mounted (GTask *task)
{
	GtefFileLoader *loader;
	GtefFileLoaderPrivate *priv;
	TaskData *task_data;
	GMountOperation *mount_operation;

	loader = g_task_get_source_object (task);
	priv = gtef_file_loader_get_instance_private (loader);

	task_data = g_task_get_task_data (task);

	mount_operation = _gtef_file_create_mount_operation (priv->file);

	task_data->tried_mount = TRUE;

	g_file_mount_enclosing_volume (priv->location,
				       G_MOUNT_MOUNT_NONE,
				       mount_operation,
				       g_task_get_cancellable (task),
				       mount_cb,
				       task);

	g_object_unref (mount_operation);
}

static void
load_content_cb (GObject      *source_object,
		 GAsyncResult *result,
		 gpointer      user_data)
{
	GtefFileContentLoader *content_loader = GTEF_FILE_CONTENT_LOADER (source_object);
	GTask *task = G_TASK (user_data);
	TaskData *task_data;
	GError *error = NULL;

	task_data = g_task_get_task_data (task);

	_gtef_file_content_loader_load_finish (content_loader, result, &error);

	if (error != NULL)
	{
		if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_MOUNTED) &&
		    !task_data->tried_mount)
		{
			recover_not_mounted (task);
			g_error_free (error);
		}
		else
		{
			g_task_return_error (task, error);
		}
	}
	else
	{
		/* Finished reading, next operation. */
		determine_encoding (task);
	}
}

static void
load_content (GTask *task)
{
	TaskData *task_data;
	GtefFileLoader *loader;
	GtefFileLoaderPrivate *priv;

	task_data = g_task_get_task_data (task);

	loader = g_task_get_source_object (task);
	priv = gtef_file_loader_get_instance_private (loader);

	g_clear_object (&task_data->content_loader);
	task_data->content_loader = _gtef_file_content_loader_new_from_file (priv->location);

	_gtef_file_content_loader_set_max_size (task_data->content_loader,
						priv->max_size);

	_gtef_file_content_loader_set_chunk_size (task_data->content_loader,
						  priv->chunk_size);

	_gtef_file_content_loader_load_async (task_data->content_loader,
					      g_task_get_priority (task),
					      g_task_get_cancellable (task),
					      task_data->progress_cb,
					      task_data->progress_cb_data,
					      NULL, /* Call the GDestroyNotify just once. */
					      load_content_cb,
					      task);
}

static void
start_loading (GTask *task)
{
	GtefFileLoader *loader;
	GtefFileLoaderPrivate *priv;

	loader = g_task_get_source_object (task);
	priv = gtef_file_loader_get_instance_private (loader);

	if (priv->buffer == NULL)
	{
		g_task_return_boolean (task, FALSE);
		return;
	}

	gtk_source_buffer_begin_not_undoable_action (GTK_SOURCE_BUFFER (priv->buffer));
	gtk_text_buffer_begin_user_action (GTK_TEXT_BUFFER (priv->buffer));

	empty_buffer (loader);

	load_content (task);
}

static void
finish_loading (GTask *task)
{
	GtefFileLoader *loader;
	GtefFileLoaderPrivate *priv;
	GtkTextIter start;

	loader = g_task_get_source_object (task);
	priv = gtef_file_loader_get_instance_private (loader);

	if (priv->buffer == NULL)
	{
		return;
	}

	gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (priv->buffer), &start);
	gtk_text_buffer_place_cursor (GTK_TEXT_BUFFER (priv->buffer), &start);

	gtk_text_buffer_end_user_action (GTK_TEXT_BUFFER (priv->buffer));
	gtk_source_buffer_end_not_undoable_action (GTK_SOURCE_BUFFER (priv->buffer));

	gtk_text_buffer_set_modified (GTK_TEXT_BUFFER (priv->buffer), FALSE);
}

static void
reset (GtefFileLoader *loader)
{
	GtefFileLoaderPrivate *priv = gtef_file_loader_get_instance_private (loader);

	gtef_encoding_free (priv->detected_encoding);
	priv->detected_encoding = NULL;

	priv->detected_newline_type = GTEF_NEWLINE_TYPE_DEFAULT;
}

/**
 * gtef_file_loader_load_async:
 * @loader: a #GtefFileLoader.
 * @io_priority: the I/O priority of the request. E.g. %G_PRIORITY_LOW,
 *   %G_PRIORITY_DEFAULT or %G_PRIORITY_HIGH.
 * @cancellable: (nullable): optional #GCancellable object, %NULL to ignore.
 * @progress_callback: (scope notified) (nullable): function to call back with
 *   progress information, or %NULL if progress information is not needed.
 * @progress_callback_data: (closure): user data to pass to @progress_callback.
 * @progress_callback_notify: (nullable): function to call on
 *   @progress_callback_data when the @progress_callback is no longer needed, or
 *   %NULL.
 * @callback: (scope async): a #GAsyncReadyCallback to call when the request is
 *   satisfied.
 * @user_data: user data to pass to @callback.
 *
 * Loads asynchronously the file content into the #GtefBuffer.
 *
 * See the #GAsyncResult documentation to know how to use this function.
 *
 * Since: 1.0
 */

/* The GDestroyNotify is needed, currently the following bug is not fixed:
 * https://bugzilla.gnome.org/show_bug.cgi?id=616044
 */
void
gtef_file_loader_load_async (GtefFileLoader        *loader,
			     gint                   io_priority,
			     GCancellable          *cancellable,
			     GFileProgressCallback  progress_callback,
			     gpointer               progress_callback_data,
			     GDestroyNotify         progress_callback_notify,
			     GAsyncReadyCallback    callback,
			     gpointer               user_data)
{
	GtefFileLoaderPrivate *priv;
	TaskData *task_data;

	g_return_if_fail (GTEF_IS_FILE_LOADER (loader));
	g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));

	priv = gtef_file_loader_get_instance_private (loader);

	if (priv->task != NULL)
	{
		g_warning ("Several load operations in parallel with the same "
			   "GtefFileLoader is not possible and doesn't make sense.");
		return;
	}

	g_return_if_fail (priv->location != NULL);

	reset (loader);

	priv->task = g_task_new (loader, cancellable, callback, user_data);
	g_task_set_priority (priv->task, io_priority);

	task_data = task_data_new ();
	g_task_set_task_data (priv->task, task_data, task_data_free);

	task_data->progress_cb = progress_callback;
	task_data->progress_cb_data = progress_callback_data;
	task_data->progress_cb_notify = progress_callback_notify;

	start_loading (priv->task);
}

/**
 * gtef_file_loader_load_finish:
 * @loader: a #GtefFileLoader.
 * @result: a #GAsyncResult.
 * @error: a #GError, or %NULL.
 *
 * Finishes a file loading started with gtef_file_loader_load_async().
 *
 * Returns: whether the content has been loaded successfully.
 * Since: 1.0
 */
gboolean
gtef_file_loader_load_finish (GtefFileLoader  *loader,
			      GAsyncResult    *result,
			      GError         **error)
{
	GtefFileLoaderPrivate *priv;
	gboolean ok;

	g_return_val_if_fail (GTEF_IS_FILE_LOADER (loader), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail (g_task_is_valid (result, loader), FALSE);

	priv = gtef_file_loader_get_instance_private (loader);

	g_return_val_if_fail (G_TASK (result) == priv->task, FALSE);

	finish_loading (priv->task);

	ok = g_task_propagate_boolean (priv->task, error);

	if (ok && priv->file != NULL)
	{
		TaskData *task_data;
		const gchar *etag;
		gboolean readonly;

		task_data = g_task_get_task_data (priv->task);

		_gtef_file_set_encoding (priv->file, priv->detected_encoding);
		_gtef_file_set_newline_type (priv->file, priv->detected_newline_type);
		_gtef_file_set_compression_type (priv->file, GTEF_COMPRESSION_TYPE_NONE);
		_gtef_file_set_externally_modified (priv->file, FALSE);
		_gtef_file_set_deleted (priv->file, FALSE);

		etag = _gtef_file_content_loader_get_etag (task_data->content_loader);
		_gtef_file_set_etag (priv->file, etag);

		readonly = _gtef_file_content_loader_get_readonly (task_data->content_loader);
		_gtef_file_set_readonly (priv->file, readonly);
	}

	g_clear_object (&priv->task);

	return ok;
}

/**
 * gtef_file_loader_get_encoding:
 * @loader: a #GtefFileLoader.
 *
 * Returns: (nullable): the detected file encoding, or %NULL.
 * Since: 2.0
 */
const GtefEncoding *
gtef_file_loader_get_encoding (GtefFileLoader *loader)
{
	GtefFileLoaderPrivate *priv;

	g_return_val_if_fail (GTEF_IS_FILE_LOADER (loader), NULL);

	priv = gtef_file_loader_get_instance_private (loader);
	return priv->detected_encoding;
}

/**
 * gtef_file_loader_get_newline_type:
 * @loader: a #GtefFileLoader.
 *
 * Returns: the detected newline type.
 * Since: 2.0
 */
GtefNewlineType
gtef_file_loader_get_newline_type (GtefFileLoader *loader)
{
	GtefFileLoaderPrivate *priv;

	g_return_val_if_fail (GTEF_IS_FILE_LOADER (loader), GTEF_NEWLINE_TYPE_LF);

	priv = gtef_file_loader_get_instance_private (loader);
	return priv->detected_newline_type;
}

/* For the unit tests. */
gint64
_gtef_file_loader_get_encoding_converter_buffer_size (void)
{
	GtefEncodingConverter *converter;
	gint64 buffer_size;

	converter = _gtef_encoding_converter_new (ENCODING_CONVERTER_BUFFER_SIZE);
	buffer_size = _gtef_encoding_converter_get_buffer_size (converter);
	g_object_unref (converter);

	return buffer_size;
}
