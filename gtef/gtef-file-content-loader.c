/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
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
#include "gtef-file-content-loader.h"
#include <glib/gi18n-lib.h>
#include "gtef-file-loader.h" /* For GTEF_FILE_LOADER_ERROR */

/* Just loads the content of a GFile, with a max size and a progress callback.
 * The progress callback is called after each chunk read. The chunk size can be
 * adjusted.
 * Doesn't handle/recover from errors.
 */

typedef struct _TaskData TaskData;

struct _GtefFileContentLoaderPrivate
{
	GFile *location;

	gint64 max_size;
	gint64 chunk_size;

	GTask *task;

	/* List of GBytes*. */
	GQueue *content;
};

struct _TaskData
{
	GFileInfo *info;

	GFileInputStream *file_input_stream;

	GFileProgressCallback progress_cb;
	gpointer progress_cb_data;
	GDestroyNotify progress_cb_notify;

	goffset total_bytes_read;
	goffset total_size;
};

G_DEFINE_TYPE_WITH_PRIVATE (GtefFileContentLoader, _gtef_file_content_loader, G_TYPE_OBJECT)

/* Prototypes */
static void read_next_chunk (GTask *task);

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

	g_clear_object (&task_data->info);
	g_clear_object (&task_data->file_input_stream);

	if (task_data->progress_cb_notify != NULL)
	{
		task_data->progress_cb_notify (task_data->progress_cb_data);
	}

	g_free (task_data);
}

static void
_gtef_file_content_loader_dispose (GObject *object)
{
	GtefFileContentLoader *loader = GTEF_FILE_CONTENT_LOADER (object);

	g_clear_object (&loader->priv->location);
	g_clear_object (&loader->priv->task);

	G_OBJECT_CLASS (_gtef_file_content_loader_parent_class)->dispose (object);
}

static void
_gtef_file_content_loader_finalize (GObject *object)
{
	GtefFileContentLoader *loader = GTEF_FILE_CONTENT_LOADER (object);

	if (loader->priv->content != NULL)
	{
		g_queue_free_full (loader->priv->content, (GDestroyNotify)g_bytes_unref);
	}

	G_OBJECT_CLASS (_gtef_file_content_loader_parent_class)->finalize (object);
}

static void
_gtef_file_content_loader_class_init (GtefFileContentLoaderClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = _gtef_file_content_loader_dispose;
	object_class->finalize = _gtef_file_content_loader_finalize;
}

static void
_gtef_file_content_loader_init (GtefFileContentLoader *loader)
{
	loader->priv = _gtef_file_content_loader_get_instance_private (loader);

	loader->priv->max_size = GTEF_FILE_CONTENT_LOADER_DEFAULT_MAX_SIZE;
	loader->priv->chunk_size = GTEF_FILE_CONTENT_LOADER_DEFAULT_CHUNK_SIZE;
}

GtefFileContentLoader *
_gtef_file_content_loader_new_from_file (GFile *location)
{
	GtefFileContentLoader *loader;

	g_return_val_if_fail (G_IS_FILE (location), NULL);

	loader = g_object_new (GTEF_TYPE_FILE_CONTENT_LOADER, NULL);
	loader->priv->location = g_object_ref (location);

	return loader;
}

/*
 * _gtef_file_content_loader_set_max_size:
 * @loader: a #GtefFileContentLoader.
 * @max_size: the new maximum size, or -1 for unlimited.
 */
void
_gtef_file_content_loader_set_max_size (GtefFileContentLoader *loader,
					gint64                 max_size)
{
	g_return_if_fail (GTEF_IS_FILE_CONTENT_LOADER (loader));
	g_return_if_fail (max_size >= -1);
	g_return_if_fail (loader->priv->task == NULL);

	loader->priv->max_size = max_size;
}

void
_gtef_file_content_loader_set_chunk_size (GtefFileContentLoader *loader,
					  gint64                 chunk_size)
{
	g_return_if_fail (GTEF_IS_FILE_CONTENT_LOADER (loader));
	g_return_if_fail (chunk_size >= 1);

	loader->priv->chunk_size = chunk_size;
}

static void
close_input_stream_cb (GObject      *source_object,
		       GAsyncResult *result,
		       gpointer      user_data)
{
	GInputStream *input_stream = G_INPUT_STREAM (source_object);
	GTask *task = G_TASK (user_data);
	GError *error = NULL;

	g_input_stream_close_finish (input_stream, result, &error);

	if (error != NULL)
	{
		g_task_return_error (task, error);
		return;
	}

	/* Finished! */
	g_task_return_boolean (task, TRUE);
}

static void
close_input_stream (GTask *task)
{
	TaskData *task_data;

	task_data = g_task_get_task_data (task);

	g_input_stream_close_async (G_INPUT_STREAM (task_data->file_input_stream),
				    g_task_get_priority (task),
				    g_task_get_cancellable (task),
				    close_input_stream_cb,
				    task);
}

static void
read_next_chunk_cb (GObject      *source_object,
		    GAsyncResult *result,
		    gpointer      user_data)
{
	GInputStream *input_stream = G_INPUT_STREAM (source_object);
	GTask *task = G_TASK (user_data);
	GtefFileContentLoader *loader;
	TaskData *task_data;
	GBytes *chunk;
	gsize chunk_size;
	GError *error = NULL;

	loader = g_task_get_source_object (task);
	task_data = g_task_get_task_data (task);

	chunk = g_input_stream_read_bytes_finish (input_stream, result, &error);

	if (error != NULL)
	{
		g_task_return_error (task, error);
		g_clear_pointer (&chunk, (GDestroyNotify)g_bytes_unref);
		return;
	}

	chunk_size = g_bytes_get_size (chunk);

	if (chunk_size == 0)
	{
		/* Finished reading */
		close_input_stream (task);
		g_bytes_unref (chunk);
		return;
	}

	g_queue_push_tail (loader->priv->content, chunk);
	task_data->total_bytes_read += chunk_size;

	/* Read next chunk before calling the progress_cb, because the
	 * progress_cb can take some time. If for some reason the progress_cb
	 * takes more time than reading the next chunk, the ordering will still
	 * be good, with the main event loop.
	 */
	read_next_chunk (task);

	if (task_data->progress_cb != NULL &&
	    task_data->total_size > 0)
	{
		/* It can happen, for example when another process changes the
		 * file we are currently reading (race condition).
		 * FIXME: It would maybe be better to report an error, or check
		 * at the end of the file loading if the file was not modified
		 * since the beginning of the file loading.
		 */
		if (task_data->total_size < task_data->total_bytes_read)
		{
			task_data->total_size = task_data->total_bytes_read;
		}

		task_data->progress_cb (task_data->total_bytes_read,
					task_data->total_size,
					task_data->progress_cb_data);
	}
}

static void
read_next_chunk (GTask *task)
{
	GtefFileContentLoader *loader;
	TaskData *task_data;

	loader = g_task_get_source_object (task);
	task_data = g_task_get_task_data (task);

	/* GIO provides several functions to read content from a GInputStream or
	 * a GFile. Here we want to report progress information, mainly in case
	 * the content comes from a remote file (with potentially a slow
	 * connection). Reading a local file should be fast enough to not need a
	 * progress bar. Anyway, for our use case, it seems that
	 * g_input_stream_read_async() is better than
	 * g_input_stream_read_all_async(): we can suppose that if the
	 * connection is slow, after a certain time read_async() will return
	 * earlier than read_all_async(). And the documentation says that
	 * g_input_stream_read_bytes() is like g_input_stream_read(). And having
	 * a GBytes is more convenient in our case. At the end of the day, I
	 * don't know if it makes a big difference, so it's better to favor
	 * simple code.
	 */
	g_input_stream_read_bytes_async (G_INPUT_STREAM (task_data->file_input_stream),
					 MAX (1, loader->priv->chunk_size),
					 g_task_get_priority (task),
					 g_task_get_cancellable (task),
					 read_next_chunk_cb,
					 task);
}

static void
check_file_size (GTask *task)
{
	TaskData *task_data;
	GtefFileContentLoader *loader;

	task_data = g_task_get_task_data (task);
	loader = g_task_get_source_object (task);

	if (g_file_info_has_attribute (task_data->info, G_FILE_ATTRIBUTE_STANDARD_SIZE))
	{
		task_data->total_size = g_file_info_get_size (task_data->info);

		if (loader->priv->max_size >= 0 &&
		    task_data->total_size > loader->priv->max_size)
		{
			gchar *max_size_str;

			max_size_str = g_format_size (loader->priv->max_size);

			g_task_return_new_error (task,
						 GTEF_FILE_LOADER_ERROR,
						 GTEF_FILE_LOADER_ERROR_TOO_BIG,
						 _("The file is too big. Maximum %s can be loaded."),
						 max_size_str);

			g_free (max_size_str);
			return;
		}
	}

	/* Start reading */
	read_next_chunk (task);
}

static void
query_info_cb (GObject      *source_object,
	       GAsyncResult *result,
	       gpointer      user_data)
{
	GFileInputStream *file_input_stream = G_FILE_INPUT_STREAM (source_object);
	GTask *task = G_TASK (user_data);
	TaskData *task_data;
	GError *error = NULL;

	task_data = g_task_get_task_data (task);

	g_clear_object (&task_data->info);
	task_data->info = g_file_input_stream_query_info_finish (file_input_stream, result, &error);

	if (error != NULL)
	{
		g_task_return_error (task, error);
		return;
	}

	check_file_size (task);
}

static void
query_info (GTask *task)
{
	TaskData *task_data;

	task_data = g_task_get_task_data (task);

	/* Query info on the GFileInputStream, not the GFile. We can suppose
	 * that we avoid potential races that way. If we query the info on the
	 * GFile, then the file is modified by another program, then we open the
	 * GFile to have the GFileInputStream, we would have a race condition.
	 */
	g_file_input_stream_query_info_async (task_data->file_input_stream,
					      G_FILE_ATTRIBUTE_STANDARD_SIZE,
					      g_task_get_priority (task),
					      g_task_get_cancellable (task),
					      query_info_cb,
					      task);
}

static void
open_file_cb (GObject      *source_object,
	      GAsyncResult *result,
	      gpointer      user_data)
{
	GFile *location = G_FILE (source_object);
	GTask *task = G_TASK (user_data);
	TaskData *task_data;
	GError *error = NULL;

	task_data = g_task_get_task_data (task);

	g_assert (task_data->file_input_stream == NULL);
	task_data->file_input_stream = g_file_read_finish (location, result, &error);

	if (error != NULL)
	{
		g_task_return_error (task, error);
		return;
	}

	query_info (task);
}

static void
open_file (GTask *task)
{
	GtefFileContentLoader *loader;

	loader = g_task_get_source_object (task);

	g_file_read_async (loader->priv->location,
			   g_task_get_priority (task),
			   g_task_get_cancellable (task),
			   open_file_cb,
			   task);
}

/*
 * _gtef_file_content_loader_load_async:
 * @loader: a #GtefFileContentLoader.
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
 * Loads asynchronously the content.
 */

/* The GDestroyNotify is needed, currently the following bug is not fixed:
 * https://bugzilla.gnome.org/show_bug.cgi?id=616044
 */
void
_gtef_file_content_loader_load_async (GtefFileContentLoader *loader,
				      gint                   io_priority,
				      GCancellable          *cancellable,
				      GFileProgressCallback  progress_callback,
				      gpointer               progress_callback_data,
				      GDestroyNotify         progress_callback_notify,
				      GAsyncReadyCallback    callback,
				      gpointer               user_data)
{
	TaskData *task_data;

	g_return_if_fail (GTEF_IS_FILE_CONTENT_LOADER (loader));
	g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));

	if (loader->priv->task != NULL)
	{
		g_warning ("Several load operations in parallel with the same "
			   "GtefFileContentLoader is not possible and doesn't make sense.");
		return;
	}

	loader->priv->task = g_task_new (loader, cancellable, callback, user_data);
	g_task_set_priority (loader->priv->task, io_priority);

	task_data = task_data_new ();
	g_task_set_task_data (loader->priv->task, task_data, task_data_free);

	task_data->progress_cb = progress_callback;
	task_data->progress_cb_data = progress_callback_data;
	task_data->progress_cb_notify = progress_callback_notify;

	if (loader->priv->content != NULL)
	{
		g_queue_free_full (loader->priv->content, (GDestroyNotify)g_bytes_unref);
	}

	loader->priv->content = g_queue_new ();

	open_file (loader->priv->task);
}

/*
 * _gtef_file_content_loader_load_finish:
 * @loader: a #GtefFileContentLoader.
 * @result: a #GAsyncResult.
 * @error: a #GError, or %NULL.
 *
 * Finishes a file loading started with _gtef_file_content_loader_load_async().
 *
 * If the file is too big, the %GTEF_FILE_LOADER_ERROR_TOO_BIG error is
 * returned.
 *
 * Returns: whether the content has been loaded successfully.
 */
gboolean
_gtef_file_content_loader_load_finish (GtefFileContentLoader  *loader,
				       GAsyncResult           *result,
				       GError                **error)
{
	gboolean ok;

	g_return_val_if_fail (GTEF_IS_FILE_CONTENT_LOADER (loader), FALSE);
	g_return_val_if_fail (g_task_is_valid (result, loader), FALSE);
	g_return_val_if_fail (G_TASK (result) == loader->priv->task, FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	ok = g_task_propagate_boolean (loader->priv->task, error);

	g_clear_object (&loader->priv->task);
	return ok;
}

/* Returns: (transfer none) (element-type GBytes): the content that has been
 * loaded by the last load operation on @loader.
 */
GQueue *
_gtef_file_content_loader_get_content (GtefFileContentLoader *loader)
{
	g_return_val_if_fail (GTEF_IS_FILE_CONTENT_LOADER (loader), NULL);

	if (loader->priv->content == NULL)
	{
		loader->priv->content = g_queue_new ();
	}

	return loader->priv->content;
}
