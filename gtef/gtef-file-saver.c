/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2005-2007 - Paolo Borelli and Paolo Maggi
 * Copyright 2007 - Steve Frécinaux
 * Copyright 2008 - Jesse van den Kieboom
 * Copyright 2014, 2016 - Sébastien Wilmet
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
#include "gtef-file-saver.h"
#include <glib/gi18n-lib.h>
#include "gtef-file.h"
#include "gtef-buffer-input-stream.h"
#include "gtef-buffer.h"
#include "gtef-enum-types.h"

/**
 * SECTION:file-saver
 * @Short_description: Save a GtefBuffer into a file
 * @Title: GtefFileSaver
 * @See_also: #GtefFile
 *
 * A #GtefFileSaver object permits to save a #GtefBuffer into a #GFile.
 *
 * A file saver should be used only for one save operation, including errors
 * handling. If an error occurs, you can reconfigure the saver and relaunch the
 * operation with gtef_file_saver_save_async().
 *
 * #GtefFileSaver is a non-modified fork of #GtkSourceFileSaver. See the
 * description of #GtefFile for more background on why a fork was needed.
 */

/* The code has been written initially in gedit (GeditDocumentSaver).
 * It uses a GtefBufferInputStream as input, create converter(s) if needed
 * for the encoding and the compression, and write the contents to a
 * GOutputStream (the file).
 */

#if 0
#define DEBUG(x) (x)
#else
#define DEBUG(x)
#endif

#define WRITE_CHUNK_SIZE 8192

enum
{
	PROP_0,
	PROP_BUFFER,
	PROP_FILE,
	PROP_LOCATION,
	PROP_ENCODING,
	PROP_NEWLINE_TYPE,
	PROP_COMPRESSION_TYPE,
	PROP_FLAGS
};

struct _GtefFileSaverPrivate
{
	/* Weak ref to the GtkSourceBuffer. A strong ref could create a
	 * reference cycle in an application. For example a subclass of
	 * GtkSourceBuffer can have a strong ref to the FileSaver.
	 */
	GtkSourceBuffer *source_buffer;

	/* Weak ref to the GtefFile. A strong ref could create a reference
	 * cycle in an application. For example a subclass of GtefFile can
	 * have a strong ref to the FileSaver.
	 */
	GtefFile *file;

	GFile *location;

	const GtkSourceEncoding *encoding;
	GtefNewlineType newline_type;
	GtefCompressionType compression_type;
	GtefFileSaverFlags flags;

	GTask *task;
};

typedef struct _TaskData TaskData;
struct _TaskData
{
	GFileOutputStream *file_output_stream;

	/* The output_stream contains the file_output_stream, plus the required
	 * converter(s) for the encoding and the compression type.
	 * input_stream and output_stream cannot be spliced directly, because:
	 * (1) We need to call the progress callback.
	 * (2) Sync methods must be used for the input stream, and async
	 *     methods for the output stream.
	 */
	GtefBufferInputStream *input_stream;
	GOutputStream *output_stream;

	goffset total_size;
	GFileProgressCallback progress_cb;
	gpointer progress_cb_data;
	GDestroyNotify progress_cb_notify;

	/* This field is used when cancelling the output stream: an error occurs
	 * and is stored in this field, the output stream is cancelled
	 * asynchronously, and then the error is reported to the task.
	 */
	GError *error;

	gssize chunk_bytes_read;
	gssize chunk_bytes_written;
	gchar chunk_buffer[WRITE_CHUNK_SIZE];

	guint tried_mount : 1;
};

G_DEFINE_TYPE_WITH_PRIVATE (GtefFileSaver, gtef_file_saver, G_TYPE_OBJECT)

static void read_file_chunk (GTask *task);
static void write_file_chunk (GTask *task);
static void recover_not_mounted (GTask *task);

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

	g_clear_object (&task_data->file_output_stream);
	g_clear_object (&task_data->input_stream);
	g_clear_object (&task_data->output_stream);
	g_clear_error (&task_data->error);

	if (task_data->progress_cb_notify != NULL)
	{
		task_data->progress_cb_notify (task_data->progress_cb_data);
	}

	g_free (task_data);
}

static void
gtef_file_saver_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
	GtefFileSaver *saver = GTEF_FILE_SAVER (object);

	switch (prop_id)
	{
		case PROP_BUFFER:
			g_assert (saver->priv->source_buffer == NULL);
			saver->priv->source_buffer = g_value_get_object (value);
			g_object_add_weak_pointer (G_OBJECT (saver->priv->source_buffer),
						   (gpointer *)&saver->priv->source_buffer);
			break;

		case PROP_FILE:
			g_assert (saver->priv->file == NULL);
			saver->priv->file = g_value_get_object (value);
			g_object_add_weak_pointer (G_OBJECT (saver->priv->file),
						   (gpointer *)&saver->priv->file);
			break;

		case PROP_LOCATION:
			g_assert (saver->priv->location == NULL);
			saver->priv->location = g_value_dup_object (value);
			break;

		case PROP_ENCODING:
			gtef_file_saver_set_encoding (saver, g_value_get_boxed (value));
			break;

		case PROP_NEWLINE_TYPE:
			gtef_file_saver_set_newline_type (saver, g_value_get_enum (value));
			break;

		case PROP_COMPRESSION_TYPE:
			gtef_file_saver_set_compression_type (saver, g_value_get_enum (value));
			break;

		case PROP_FLAGS:
			gtef_file_saver_set_flags (saver, g_value_get_flags (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gtef_file_saver_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
	GtefFileSaver *saver = GTEF_FILE_SAVER (object);

	switch (prop_id)
	{
		case PROP_BUFFER:
			g_value_set_object (value, saver->priv->source_buffer);
			break;

		case PROP_FILE:
			g_value_set_object (value, saver->priv->file);
			break;

		case PROP_LOCATION:
			g_value_set_object (value, saver->priv->location);
			break;

		case PROP_ENCODING:
			g_value_set_boxed (value, saver->priv->encoding);
			break;

		case PROP_NEWLINE_TYPE:
			g_value_set_enum (value, saver->priv->newline_type);
			break;

		case PROP_COMPRESSION_TYPE:
			g_value_set_enum (value, saver->priv->compression_type);
			break;

		case PROP_FLAGS:
			g_value_set_flags (value, saver->priv->flags);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gtef_file_saver_dispose (GObject *object)
{
	GtefFileSaver *saver = GTEF_FILE_SAVER (object);

	if (saver->priv->source_buffer != NULL)
	{
		g_object_remove_weak_pointer (G_OBJECT (saver->priv->source_buffer),
					      (gpointer *)&saver->priv->source_buffer);

		saver->priv->source_buffer = NULL;
	}

	if (saver->priv->file != NULL)
	{
		g_object_remove_weak_pointer (G_OBJECT (saver->priv->file),
					      (gpointer *)&saver->priv->file);

		saver->priv->file = NULL;
	}

	g_clear_object (&saver->priv->location);
	g_clear_object (&saver->priv->task);

	G_OBJECT_CLASS (gtef_file_saver_parent_class)->dispose (object);
}

static void
gtef_file_saver_constructed (GObject *object)
{
	GtefFileSaver *saver = GTEF_FILE_SAVER (object);

	if (saver->priv->file != NULL)
	{
		const GtkSourceEncoding *encoding;
		GtefNewlineType newline_type;
		GtefCompressionType compression_type;

		encoding = gtef_file_get_encoding (saver->priv->file);
		gtef_file_saver_set_encoding (saver, encoding);

		newline_type = gtef_file_get_newline_type (saver->priv->file);
		gtef_file_saver_set_newline_type (saver, newline_type);

		compression_type = gtef_file_get_compression_type (saver->priv->file);
		gtef_file_saver_set_compression_type (saver, compression_type);

		if (saver->priv->location == NULL)
		{
			saver->priv->location = gtef_file_get_location (saver->priv->file);

			if (saver->priv->location != NULL)
			{
				g_object_ref (saver->priv->location);
			}
			else
			{
				g_warning ("GtefFileSaver: the GtefFile's location is NULL. "
					   "Use gtef_file_saver_new_with_target().");
			}
		}
	}

	G_OBJECT_CLASS (gtef_file_saver_parent_class)->constructed (object);
}

static void
gtef_file_saver_class_init (GtefFileSaverClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = gtef_file_saver_dispose;
	object_class->set_property = gtef_file_saver_set_property;
	object_class->get_property = gtef_file_saver_get_property;
	object_class->constructed = gtef_file_saver_constructed;

	/**
	 * GtefFileSaver:buffer:
	 *
	 * The #GtefBuffer to save. The #GtefFileSaver object has a weak
	 * reference to the buffer.
	 *
	 * Since: 1.0
	 */
	g_object_class_install_property (object_class,
					 PROP_BUFFER,
					 g_param_spec_object ("buffer",
							      "GtefBuffer",
							      "",
							      GTK_SOURCE_TYPE_BUFFER,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY |
							      G_PARAM_STATIC_STRINGS));

	/**
	 * GtefFileSaver:file:
	 *
	 * The #GtefFile. The #GtefFileSaver object has a weak
	 * reference to the file.
	 *
	 * Since: 1.0
	 */
	g_object_class_install_property (object_class,
					 PROP_FILE,
					 g_param_spec_object ("file",
							      "GtefFile",
							      "",
							      GTEF_TYPE_FILE,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY |
							      G_PARAM_STATIC_STRINGS));

	/**
	 * GtefFileSaver:location:
	 *
	 * The #GFile where to save the buffer. By default the location is taken
	 * from the #GtefFile at construction time.
	 *
	 * Since: 1.0
	 */
	g_object_class_install_property (object_class,
					 PROP_LOCATION,
					 g_param_spec_object ("location",
							      "Location",
							      "",
							      G_TYPE_FILE,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY |
							      G_PARAM_STATIC_STRINGS));

	/**
	 * GtefFileSaver:encoding:
	 *
	 * The file's encoding.
	 *
	 * Since: 1.0
	 */
	g_object_class_install_property (object_class,
					 PROP_ENCODING,
					 g_param_spec_boxed ("encoding",
							     "Encoding",
							     "",
							     GTK_SOURCE_TYPE_ENCODING,
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT |
							     G_PARAM_STATIC_STRINGS));

	/**
	 * GtefFileSaver:newline-type:
	 *
	 * The newline type.
	 *
	 * Since: 1.0
	 */
	g_object_class_install_property (object_class,
					 PROP_NEWLINE_TYPE,
					 g_param_spec_enum ("newline-type",
					                    "Newline type",
							    "",
					                    GTK_SOURCE_TYPE_NEWLINE_TYPE,
					                    GTEF_NEWLINE_TYPE_LF,
					                    G_PARAM_READWRITE |
					                    G_PARAM_CONSTRUCT |
							    G_PARAM_STATIC_STRINGS));

	/**
	 * GtefFileSaver:compression-type:
	 *
	 * The compression type.
	 *
	 * Since: 1.0
	 */
	g_object_class_install_property (object_class,
					 PROP_COMPRESSION_TYPE,
					 g_param_spec_enum ("compression-type",
					                    "Compression type",
					                    "",
					                    GTK_SOURCE_TYPE_COMPRESSION_TYPE,
					                    GTEF_COMPRESSION_TYPE_NONE,
					                    G_PARAM_READWRITE |
					                    G_PARAM_CONSTRUCT |
							    G_PARAM_STATIC_STRINGS));

	/**
	 * GtefFileSaver:flags:
	 *
	 * File saving flags.
	 *
	 * Since: 1.0
	 */
	g_object_class_install_property (object_class,
					 PROP_FLAGS,
					 g_param_spec_flags ("flags",
							     "Flags",
							     "",
							     GTEF_TYPE_FILE_SAVER_FLAGS,
							     GTEF_FILE_SAVER_FLAGS_NONE,
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT |
							     G_PARAM_STATIC_STRINGS));
}

static void
gtef_file_saver_init (GtefFileSaver *saver)
{
	saver->priv = gtef_file_saver_get_instance_private (saver);
}

/* BEGIN NOTE:
 *
 * This fixes an issue in GOutputStream that applies the atomic replace save
 * strategy. The stream moves the written file to the original file when the
 * stream is closed. However, there is no way currently to tell the stream that
 * the save should be aborted (there could be a conversion error). The patch
 * explicitly closes the output stream in all these cases with a GCancellable in
 * the cancelled state, causing the output stream to close, but not move the
 * file. This makes use of an implementation detail in the local file stream
 * and should be properly fixed by adding the appropriate API in GIO. Until
 * then, at least we prevent data corruption for now.
 *
 * Relevant bug reports:
 *
 * Bug 615110 - write file ignore encoding errors (gedit)
 * https://bugzilla.gnome.org/show_bug.cgi?id=615110
 *
 * Bug 602412 - g_file_replace does not restore original file when there is
 *              errors while writing (glib/gio)
 * https://bugzilla.gnome.org/show_bug.cgi?id=602412
 */
static void
cancel_output_stream_ready_cb (GObject      *source_object,
			       GAsyncResult *result,
			       gpointer      user_data)
{
	GOutputStream *output_stream = G_OUTPUT_STREAM (source_object);
	GTask *task = G_TASK (user_data);
	TaskData *task_data;

	task_data = g_task_get_task_data (task);

	g_output_stream_close_finish (output_stream, result, NULL);

	if (task_data->error != NULL)
	{
		GError *error = task_data->error;
		task_data->error = NULL;
		g_task_return_error (task, error);
	}
	else
	{
		g_task_return_boolean (task, FALSE);
	}
}

static void
cancel_output_stream (GTask *task)
{
	TaskData *task_data;
	GCancellable *cancellable;

	DEBUG ({
	       g_print ("Cancel output stream\n");
	});

	task_data = g_task_get_task_data (task);

	cancellable = g_cancellable_new ();
	g_cancellable_cancel (cancellable);

	g_output_stream_close_async (task_data->output_stream,
				     g_task_get_priority (task),
				     cancellable,
				     cancel_output_stream_ready_cb,
				     task);

	g_object_unref (cancellable);
}

/*
 * END NOTE
 */

static void
close_output_stream_cb (GObject      *source_object,
			GAsyncResult *result,
			gpointer      user_data)
{
	GOutputStream *output_stream = G_OUTPUT_STREAM (source_object);
	GTask *task = G_TASK (user_data);
	GError *error = NULL;

	DEBUG ({
	       g_print ("%s\n", G_STRFUNC);
	});

	g_output_stream_close_finish (output_stream, result, &error);

	if (error != NULL)
	{
		DEBUG ({
		       g_print ("Closing stream error: %s\n", error->message);
		});

		g_task_return_error (task, error);
		return;
	}

	/* Finished! */
	g_task_return_boolean (task, TRUE);
}

static void
write_complete (GTask *task)
{
	TaskData *task_data;
	GError *error = NULL;

	DEBUG ({
	       g_print ("Close input stream\n");
	});

	task_data = g_task_get_task_data (task);

	g_input_stream_close (G_INPUT_STREAM (task_data->input_stream),
			      g_task_get_cancellable (task),
			      &error);

	if (error != NULL)
	{
		DEBUG ({
		       g_print ("Closing input stream error: %s\n", error->message);
		});

		g_clear_error (&task_data->error);
		task_data->error = error;
		cancel_output_stream (task);
		return;
	}

	DEBUG ({
	       g_print ("Close output stream\n");
	});

	g_output_stream_close_async (task_data->output_stream,
				     g_task_get_priority (task),
				     g_task_get_cancellable (task),
				     close_output_stream_cb,
				     task);
}

static void
write_file_chunk_cb (GObject      *source_object,
		     GAsyncResult *result,
		     gpointer      user_data)
{
	GOutputStream *output_stream = G_OUTPUT_STREAM (source_object);
	GTask *task = G_TASK (user_data);
	TaskData *task_data;
	gssize bytes_written;
	GError *error = NULL;

	DEBUG ({
	       g_print ("%s\n", G_STRFUNC);
	});

	task_data = g_task_get_task_data (task);

	bytes_written = g_output_stream_write_finish (output_stream, result, &error);

	DEBUG ({
	       g_print ("Written: %" G_GSSIZE_FORMAT "\n", bytes_written);
	});

	if (error != NULL)
	{
		DEBUG ({
		       g_print ("Write error: %s\n", error->message);
		});

		g_clear_error (&task_data->error);
		task_data->error = error;
		cancel_output_stream (task);
		return;
	}

	task_data->chunk_bytes_written += bytes_written;

	/* Write again */
	if (task_data->chunk_bytes_written < task_data->chunk_bytes_read)
	{
		write_file_chunk (task);
		return;
	}

	if (task_data->progress_cb != NULL)
	{
		gsize total_chars_written;

		total_chars_written = _gtef_buffer_input_stream_tell (task_data->input_stream);

		task_data->progress_cb (total_chars_written,
					task_data->total_size,
					task_data->progress_cb_data);
	}

	read_file_chunk (task);
}

static void
write_file_chunk (GTask *task)
{
	TaskData *task_data;

	DEBUG ({
	       g_print ("%s\n", G_STRFUNC);
	});

	task_data = g_task_get_task_data (task);

	g_output_stream_write_async (task_data->output_stream,
				     task_data->chunk_buffer + task_data->chunk_bytes_written,
				     task_data->chunk_bytes_read - task_data->chunk_bytes_written,
				     g_task_get_priority (task),
				     g_task_get_cancellable (task),
				     write_file_chunk_cb,
				     task);
}

static void
read_file_chunk (GTask *task)
{
	TaskData *task_data;
	GError *error = NULL;

	DEBUG ({
	       g_print ("%s\n", G_STRFUNC);
	});

	task_data = g_task_get_task_data (task);

	task_data->chunk_bytes_written = 0;

	/* We use sync methods on doc stream since it is in memory. Using async
	 * would be racy and we could end up with invalid iters.
	 */
	task_data->chunk_bytes_read = g_input_stream_read (G_INPUT_STREAM (task_data->input_stream),
							   task_data->chunk_buffer,
							   WRITE_CHUNK_SIZE,
							   g_task_get_cancellable (task),
							   &error);

	if (error != NULL)
	{
		g_clear_error (&task_data->error);
		task_data->error = error;
		cancel_output_stream (task);
		return;
	}

	/* Check if we finished reading and writing. */
	if (task_data->chunk_bytes_read == 0)
	{
		write_complete (task);
		return;
	}

	write_file_chunk (task);
}

static void
replace_file_cb (GObject      *source_object,
		 GAsyncResult *result,
		 gpointer      user_data)
{
	GFile *location = G_FILE (source_object);
	GTask *task = G_TASK (user_data);
	GtefFileSaver *saver;
	TaskData *task_data;
	GOutputStream *output_stream;
	GError *error = NULL;

	DEBUG ({
	       g_print ("%s\n", G_STRFUNC);
	});

	saver = g_task_get_source_object (task);
	task_data = g_task_get_task_data (task);

	g_clear_object (&task_data->file_output_stream);
	task_data->file_output_stream = g_file_replace_finish (location, result, &error);

	if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_MOUNTED) &&
	    !task_data->tried_mount)
	{
		recover_not_mounted (task);
		g_error_free (error);
		return;
	}
	else if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_WRONG_ETAG))
	{
		g_task_return_new_error (task,
					 GTEF_FILE_SAVER_ERROR,
					 GTEF_FILE_SAVER_ERROR_EXTERNALLY_MODIFIED,
					 _("The file is externally modified."));
		g_error_free (error);
		return;
	}
	else if (error != NULL)
	{
		DEBUG ({
		       g_print ("Opening file failed: %s\n", error->message);
		});

		g_task_return_error (task, error);
		return;
	}

	if (saver->priv->compression_type == GTEF_COMPRESSION_TYPE_GZIP)
	{
		GZlibCompressor *compressor;

		DEBUG ({
		       g_print ("Use gzip compressor\n");
		});

		compressor = g_zlib_compressor_new (G_ZLIB_COMPRESSOR_FORMAT_GZIP, -1);

		output_stream = g_converter_output_stream_new (G_OUTPUT_STREAM (task_data->file_output_stream),
							       G_CONVERTER (compressor));

		g_object_unref (compressor);
	}
	else
	{
		output_stream = G_OUTPUT_STREAM (task_data->file_output_stream);
		g_object_ref (output_stream);
	}

	/* FIXME: manage converter error? */

	DEBUG ({
	       g_print ("Encoding charset: %s\n",
			gtk_source_encoding_get_charset (saver->priv->encoding));
	});

	if (saver->priv->encoding != gtk_source_encoding_get_utf8 ())
	{
		GCharsetConverter *converter;

		converter = g_charset_converter_new (gtk_source_encoding_get_charset (saver->priv->encoding),
						     "UTF-8",
						     NULL);

		g_clear_object (&task_data->output_stream);
		task_data->output_stream = g_converter_output_stream_new (output_stream,
									  G_CONVERTER (converter));

		g_object_unref (converter);
		g_object_unref (output_stream);
	}
	else
	{
		g_clear_object (&task_data->output_stream);
		task_data->output_stream = G_OUTPUT_STREAM (output_stream);
	}

	task_data->total_size = _gtef_buffer_input_stream_get_total_size (task_data->input_stream);

	DEBUG ({
	       g_print ("Total number of characters: %" G_GINT64_FORMAT "\n", task_data->total_size);
	});

	read_file_chunk (task);
}

static void
begin_write (GTask *task)
{
	GtefFileSaver *saver;
	gboolean create_backup;
	const gchar *etag;

	saver = g_task_get_source_object (task);

	create_backup = (saver->priv->flags & GTEF_FILE_SAVER_FLAGS_CREATE_BACKUP) != 0;

	DEBUG ({
	       g_print ("Start replacing file contents\n");
	       g_print ("Make backup: %s\n", create_backup ? "yes" : "no");
	});

	if (saver->priv->flags & GTEF_FILE_SAVER_FLAGS_IGNORE_MODIFICATION_TIME)
	{
		etag = NULL;
	}
	else
	{
		etag = _gtef_file_get_etag (saver->priv->file);
	}

	g_file_replace_async (saver->priv->location,
			      etag,
			      create_backup,
			      G_FILE_CREATE_NONE,
			      g_task_get_priority (task),
			      g_task_get_cancellable (task),
			      replace_file_cb,
			      task);
}

static void
mount_cb (GObject      *source_object,
	  GAsyncResult *result,
	  gpointer      user_data)
{
	GFile *location = G_FILE (source_object);
	GTask *task = G_TASK (user_data);
	GtefFileSaver *saver;
	GError *error = NULL;

	DEBUG ({
	       g_print ("%s\n", G_STRFUNC);
	});

	saver = g_task_get_source_object (task);

	g_file_mount_enclosing_volume_finish (location, result, &error);

	if (error != NULL)
	{
		g_task_return_error (task, error);
		return;
	}

	if (saver->priv->file != NULL)
	{
		_gtef_file_set_mounted (saver->priv->file);
	}

	begin_write (task);
}

static void
recover_not_mounted (GTask *task)
{
	GtefFileSaver *saver;
	TaskData *task_data;
	GMountOperation *mount_operation;

	saver = g_task_get_source_object (task);
	task_data = g_task_get_task_data (task);

	mount_operation = _gtef_file_create_mount_operation (saver->priv->file);

	DEBUG ({
	       g_print ("%s\n", G_STRFUNC);
	});

	task_data->tried_mount = TRUE;

	g_file_mount_enclosing_volume (saver->priv->location,
				       G_MOUNT_MOUNT_NONE,
				       mount_operation,
				       g_task_get_cancellable (task),
				       mount_cb,
				       task);

	g_object_unref (mount_operation);
}

GQuark
gtef_file_saver_error_quark (void)
{
	static GQuark quark = 0;

	if (G_UNLIKELY (quark == 0))
	{
		quark = g_quark_from_static_string ("gtk-source-file-saver-error");
	}

	return quark;
}

/**
 * gtef_file_saver_new:
 * @buffer: the #GtefBuffer to save.
 * @file: the #GtefFile.
 *
 * Creates a new #GtefFileSaver object. The @buffer will be saved to the
 * #GtefFile's location.
 *
 * This constructor is suitable for a simple "save" operation, when the @file
 * already contains a non-%NULL #GtefFile:location.
 *
 * Returns: a new #GtefFileSaver object.
 * Since: 1.0
 */
GtefFileSaver *
gtef_file_saver_new (GtefBuffer *buffer,
		     GtefFile   *file)
{
	g_return_val_if_fail (GTEF_IS_BUFFER (buffer), NULL);
	g_return_val_if_fail (GTEF_IS_FILE (file), NULL);

	return g_object_new (GTEF_TYPE_FILE_SAVER,
			     "buffer", buffer,
			     "file", file,
			     NULL);
}

/**
 * gtef_file_saver_new_with_target:
 * @buffer: the #GtefBuffer to save.
 * @file: the #GtefFile.
 * @target_location: the #GFile where to save the buffer to.
 *
 * Creates a new #GtefFileSaver object with a target location. When the
 * file saving is finished successfully, @target_location is set to the @file's
 * #GtefFile:location property. If an error occurs, the previous valid
 * location is still available in #GtefFile.
 *
 * This constructor adds %GTEF_FILE_SAVER_FLAGS_IGNORE_MODIFICATION_TIME to the
 * #GtefFileSaver:flags property.
 *
 * This constructor is suitable for a "save as" operation, or for saving a new
 * buffer for the first time.
 *
 * Returns: a new #GtefFileSaver object.
 * Since: 1.0
 */
GtefFileSaver *
gtef_file_saver_new_with_target (GtefBuffer *buffer,
				 GtefFile   *file,
				 GFile      *target_location)
{
	g_return_val_if_fail (GTEF_IS_BUFFER (buffer), NULL);
	g_return_val_if_fail (GTEF_IS_FILE (file), NULL);
	g_return_val_if_fail (G_IS_FILE (target_location), NULL);

	return g_object_new (GTEF_TYPE_FILE_SAVER,
			     "buffer", buffer,
			     "file", file,
			     "location", target_location,
			     "flags", GTEF_FILE_SAVER_FLAGS_IGNORE_MODIFICATION_TIME,
			     NULL);
}

/**
 * gtef_file_saver_get_buffer:
 * @saver: a #GtefFileSaver.
 *
 * Returns: (transfer none): the #GtefBuffer to save.
 * Since: 1.0
 */
GtefBuffer *
gtef_file_saver_get_buffer (GtefFileSaver *saver)
{
	g_return_val_if_fail (GTEF_IS_FILE_SAVER (saver), NULL);

	return GTEF_BUFFER (saver->priv->source_buffer);
}

/**
 * gtef_file_saver_get_file:
 * @saver: a #GtefFileSaver.
 *
 * Returns: (transfer none): the #GtefFile.
 * Since: 1.0
 */
GtefFile *
gtef_file_saver_get_file (GtefFileSaver *saver)
{
	g_return_val_if_fail (GTEF_IS_FILE_SAVER (saver), NULL);

	return saver->priv->file;
}

/**
 * gtef_file_saver_get_location:
 * @saver: a #GtefFileSaver.
 *
 * Returns: (transfer none): the #GFile where to save the buffer to.
 * Since: 1.0
 */
GFile *
gtef_file_saver_get_location (GtefFileSaver *saver)
{
	g_return_val_if_fail (GTEF_IS_FILE_SAVER (saver), NULL);

	return saver->priv->location;
}

/**
 * gtef_file_saver_set_encoding:
 * @saver: a #GtefFileSaver.
 * @encoding: (allow-none): the new encoding, or %NULL for UTF-8.
 *
 * Sets the encoding. If @encoding is %NULL, the UTF-8 encoding will be set.
 * By default the encoding is taken from the #GtefFile.
 *
 * Since: 1.0
 */
void
gtef_file_saver_set_encoding (GtefFileSaver           *saver,
			      const GtkSourceEncoding *encoding)
{
	g_return_if_fail (GTEF_IS_FILE_SAVER (saver));
	g_return_if_fail (saver->priv->task == NULL);

	if (encoding == NULL)
	{
		encoding = gtk_source_encoding_get_utf8 ();
	}

	if (saver->priv->encoding != encoding)
	{
		saver->priv->encoding = encoding;
		g_object_notify (G_OBJECT (saver), "encoding");
	}
}

/**
 * gtef_file_saver_get_encoding:
 * @saver: a #GtefFileSaver.
 *
 * Returns: the encoding.
 * Since: 1.0
 */
const GtkSourceEncoding *
gtef_file_saver_get_encoding (GtefFileSaver *saver)
{
	g_return_val_if_fail (GTEF_IS_FILE_SAVER (saver), NULL);

	return saver->priv->encoding;
}

/**
 * gtef_file_saver_set_newline_type:
 * @saver: a #GtefFileSaver.
 * @newline_type: the new newline type.
 *
 * Sets the newline type. By default the newline type is taken from the
 * #GtefFile.
 *
 * Since: 1.0
 */
void
gtef_file_saver_set_newline_type (GtefFileSaver   *saver,
				  GtefNewlineType  newline_type)
{
	g_return_if_fail (GTEF_IS_FILE_SAVER (saver));
	g_return_if_fail (saver->priv->task == NULL);

	if (saver->priv->newline_type != newline_type)
	{
		saver->priv->newline_type = newline_type;
		g_object_notify (G_OBJECT (saver), "newline-type");
	}
}

/**
 * gtef_file_saver_get_newline_type:
 * @saver: a #GtefFileSaver.
 *
 * Returns: the newline type.
 * Since: 1.0
 */
GtefNewlineType
gtef_file_saver_get_newline_type (GtefFileSaver *saver)
{
	g_return_val_if_fail (GTEF_IS_FILE_SAVER (saver), GTEF_NEWLINE_TYPE_DEFAULT);

	return saver->priv->newline_type;
}

/**
 * gtef_file_saver_set_compression_type:
 * @saver: a #GtefFileSaver.
 * @compression_type: the new compression type.
 *
 * Sets the compression type. By default the compression type is taken from the
 * #GtefFile.
 *
 * Since: 1.0
 */
void
gtef_file_saver_set_compression_type (GtefFileSaver       *saver,
				      GtefCompressionType  compression_type)
{
	g_return_if_fail (GTEF_IS_FILE_SAVER (saver));
	g_return_if_fail (saver->priv->task == NULL);

	if (saver->priv->compression_type != compression_type)
	{
		saver->priv->compression_type = compression_type;
		g_object_notify (G_OBJECT (saver), "compression-type");
	}
}

/**
 * gtef_file_saver_get_compression_type:
 * @saver: a #GtefFileSaver.
 *
 * Returns: the compression type.
 * Since: 1.0
 */
GtefCompressionType
gtef_file_saver_get_compression_type (GtefFileSaver *saver)
{
	g_return_val_if_fail (GTEF_IS_FILE_SAVER (saver), GTEF_COMPRESSION_TYPE_NONE);

	return saver->priv->compression_type;
}

/**
 * gtef_file_saver_set_flags:
 * @saver: a #GtefFileSaver.
 * @flags: the new flags.
 *
 * Since: 1.0
 */
void
gtef_file_saver_set_flags (GtefFileSaver      *saver,
			   GtefFileSaverFlags  flags)
{
	g_return_if_fail (GTEF_IS_FILE_SAVER (saver));
	g_return_if_fail (saver->priv->task == NULL);

	if (saver->priv->flags != flags)
	{
		saver->priv->flags = flags;
		g_object_notify (G_OBJECT (saver), "flags");
	}
}

/**
 * gtef_file_saver_get_flags:
 * @saver: a #GtefFileSaver.
 *
 * Returns: the flags.
 * Since: 1.0
 */
GtefFileSaverFlags
gtef_file_saver_get_flags (GtefFileSaver *saver)
{
	g_return_val_if_fail (GTEF_IS_FILE_SAVER (saver), GTEF_FILE_SAVER_FLAGS_NONE);

	return saver->priv->flags;
}

/**
 * gtef_file_saver_save_async:
 * @saver: a #GtefFileSaver.
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
 * Saves asynchronously the buffer into the file. See the #GAsyncResult
 * documentation to know how to use this function.
 *
 * Since: 1.0
 */

/* The GDestroyNotify is needed, currently the following bug is not fixed:
 * https://bugzilla.gnome.org/show_bug.cgi?id=616044
 */
void
gtef_file_saver_save_async (GtefFileSaver         *saver,
			    gint                   io_priority,
			    GCancellable          *cancellable,
			    GFileProgressCallback  progress_callback,
			    gpointer               progress_callback_data,
			    GDestroyNotify         progress_callback_notify,
			    GAsyncReadyCallback    callback,
			    gpointer               user_data)
{
	TaskData *task_data;
	gboolean check_invalid_chars;
	gboolean implicit_trailing_newline;

	g_return_if_fail (GTEF_IS_FILE_SAVER (saver));
	g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));
	g_return_if_fail (saver->priv->task == NULL);

	saver->priv->task = g_task_new (saver, cancellable, callback, user_data);
	g_task_set_priority (saver->priv->task, io_priority);

	task_data = task_data_new ();
	g_task_set_task_data (saver->priv->task, task_data, task_data_free);

	task_data->progress_cb = progress_callback;
	task_data->progress_cb_data = progress_callback_data;
	task_data->progress_cb_notify = progress_callback_notify;

	if (saver->priv->source_buffer == NULL ||
	    saver->priv->file == NULL ||
	    saver->priv->location == NULL)
	{
		g_task_return_boolean (saver->priv->task, FALSE);
		return;
	}

	check_invalid_chars = (saver->priv->flags & GTEF_FILE_SAVER_FLAGS_IGNORE_INVALID_CHARS) == 0;

	if (check_invalid_chars && _gtef_buffer_has_invalid_chars (GTEF_BUFFER (saver->priv->source_buffer)))
	{
		g_task_return_new_error (saver->priv->task,
					 GTEF_FILE_SAVER_ERROR,
					 GTEF_FILE_SAVER_ERROR_INVALID_CHARS,
					 _("The buffer contains invalid characters."));
		return;
	}

	DEBUG ({
	       g_print ("Start saving\n");
	});

	implicit_trailing_newline = gtk_source_buffer_get_implicit_trailing_newline (saver->priv->source_buffer);

	/* The BufferInputStream has a strong reference to the buffer.
	 * We create the BufferInputStream here so we are sure that the
	 * buffer will not be destroyed during the file saving.
	 */
	task_data->input_stream = _gtef_buffer_input_stream_new (GTK_TEXT_BUFFER (saver->priv->source_buffer),
								 saver->priv->newline_type,
								 implicit_trailing_newline);

	begin_write (saver->priv->task);
}

/**
 * gtef_file_saver_save_finish:
 * @saver: a #GtefFileSaver.
 * @result: a #GAsyncResult.
 * @error: a #GError, or %NULL.
 *
 * Finishes a file saving started with gtef_file_saver_save_async().
 *
 * If the file has been saved successfully, the following #GtefFile
 * properties will be updated: the location, the encoding, the newline type and
 * the compression type.
 *
 * gtk_text_buffer_set_modified() is called with %FALSE if the file has been
 * saved successfully.
 *
 * Returns: whether the file was saved successfully.
 * Since: 1.0
 */
gboolean
gtef_file_saver_save_finish (GtefFileSaver  *saver,
			     GAsyncResult   *result,
			     GError        **error)
{
	gboolean ok;

	g_return_val_if_fail (GTEF_IS_FILE_SAVER (saver), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail (g_task_is_valid (result, saver), FALSE);

	ok = g_task_propagate_boolean (G_TASK (result), error);

	if (ok && saver->priv->file != NULL)
	{
		TaskData *task_data;
		gchar *new_etag;

		gtef_file_set_location (saver->priv->file,
					saver->priv->location);

		_gtef_file_set_encoding (saver->priv->file,
					 saver->priv->encoding);

		_gtef_file_set_newline_type (saver->priv->file,
					     saver->priv->newline_type);

		_gtef_file_set_compression_type (saver->priv->file,
						 saver->priv->compression_type);

		_gtef_file_set_externally_modified (saver->priv->file, FALSE);
		_gtef_file_set_deleted (saver->priv->file, FALSE);
		_gtef_file_set_readonly (saver->priv->file, FALSE);

		task_data = g_task_get_task_data (G_TASK (result));
		new_etag = g_file_output_stream_get_etag (task_data->file_output_stream);
		_gtef_file_set_etag (saver->priv->file, new_etag);
		g_free (new_etag);
	}

	if (ok && saver->priv->source_buffer != NULL)
	{
		gtk_text_buffer_set_modified (GTK_TEXT_BUFFER (saver->priv->source_buffer),
					      FALSE);
	}

	g_clear_object (&saver->priv->task);

	return ok;
}
