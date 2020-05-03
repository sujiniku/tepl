/* SPDX-FileCopyrightText: 2016 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "config.h"
#include "tepl-file-content-loader.h"
#include <glib/gi18n-lib.h>
#include "tepl-file-content.h"
#include "tepl-file-loader.h" /* For TEPL_FILE_LOADER_ERROR */

/* Just loads the content of a GFile, with a max size and a progress callback.
 * The progress callback is called after each chunk read. The chunk size can be
 * adjusted.
 * Doesn't handle/recover from errors.
 */

typedef struct _TaskData TaskData;

struct _TeplFileContentLoaderPrivate
{
	GFile *location;

	gint64 max_size;
	gint64 chunk_size;

	GTask *task;

	GFileInfo *info;
	gchar *etag;

	TeplFileContent *content;
};

struct _TaskData
{
	GFileInputStream *file_input_stream;

	GFileProgressCallback progress_cb;
	gpointer progress_cb_data;
	GDestroyNotify progress_cb_notify;

	goffset total_bytes_read;
	goffset total_size;
};

G_DEFINE_TYPE_WITH_PRIVATE (TeplFileContentLoader, _tepl_file_content_loader, G_TYPE_OBJECT)

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

	g_clear_object (&task_data->file_input_stream);

	if (task_data->progress_cb_notify != NULL)
	{
		task_data->progress_cb_notify (task_data->progress_cb_data);
	}

	g_free (task_data);
}

static void
reset (TeplFileContentLoader *loader)
{
	g_clear_object (&loader->priv->task);
	g_clear_object (&loader->priv->info);
	g_clear_object (&loader->priv->content);

	g_free (loader->priv->etag);
	loader->priv->etag = NULL;
}

static void
_tepl_file_content_loader_dispose (GObject *object)
{
	TeplFileContentLoader *loader = TEPL_FILE_CONTENT_LOADER (object);

	reset (loader);
	g_clear_object (&loader->priv->location);

	G_OBJECT_CLASS (_tepl_file_content_loader_parent_class)->dispose (object);
}

static void
_tepl_file_content_loader_class_init (TeplFileContentLoaderClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = _tepl_file_content_loader_dispose;
}

static void
_tepl_file_content_loader_init (TeplFileContentLoader *loader)
{
	loader->priv = _tepl_file_content_loader_get_instance_private (loader);

	loader->priv->max_size = TEPL_FILE_CONTENT_LOADER_DEFAULT_MAX_SIZE;
	loader->priv->chunk_size = TEPL_FILE_CONTENT_LOADER_DEFAULT_CHUNK_SIZE;
}

TeplFileContentLoader *
_tepl_file_content_loader_new_from_file (GFile *location)
{
	TeplFileContentLoader *loader;

	g_return_val_if_fail (G_IS_FILE (location), NULL);

	loader = g_object_new (TEPL_TYPE_FILE_CONTENT_LOADER, NULL);
	loader->priv->location = g_object_ref (location);

	return loader;
}

/*
 * _tepl_file_content_loader_set_max_size:
 * @loader: a #TeplFileContentLoader.
 * @max_size: the new maximum size, or -1 for unlimited.
 */
void
_tepl_file_content_loader_set_max_size (TeplFileContentLoader *loader,
					gint64                 max_size)
{
	g_return_if_fail (TEPL_IS_FILE_CONTENT_LOADER (loader));
	g_return_if_fail (max_size >= -1);
	g_return_if_fail (loader->priv->task == NULL);

	loader->priv->max_size = max_size;
}

void
_tepl_file_content_loader_set_chunk_size (TeplFileContentLoader *loader,
					  gint64                 chunk_size)
{
	g_return_if_fail (TEPL_IS_FILE_CONTENT_LOADER (loader));
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
	TeplFileContentLoader *loader;
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

		if (chunk != NULL)
		{
			g_bytes_unref (chunk);
		}
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

	if (loader->priv->content == NULL)
	{
		loader->priv->content = _tepl_file_content_new ();
	}

	_tepl_file_content_add_chunk (loader->priv->content, chunk);
	g_bytes_unref (chunk);
	chunk = NULL;

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
	TeplFileContentLoader *loader;
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
	TeplFileContentLoader *loader;

	task_data = g_task_get_task_data (task);
	loader = g_task_get_source_object (task);

	if (g_file_info_has_attribute (loader->priv->info, G_FILE_ATTRIBUTE_STANDARD_SIZE))
	{
		task_data->total_size = g_file_info_get_size (loader->priv->info);

		if (loader->priv->max_size >= 0 &&
		    task_data->total_size > loader->priv->max_size)
		{
			gchar *max_size_str;

			max_size_str = g_format_size (loader->priv->max_size);

			g_task_return_new_error (task,
						 TEPL_FILE_LOADER_ERROR,
						 TEPL_FILE_LOADER_ERROR_TOO_BIG,
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
query_info_on_location_cb (GObject      *source_object,
			   GAsyncResult *result,
			   gpointer      user_data)
{
	GFile *location = G_FILE (source_object);
	GTask *task = G_TASK (user_data);
	TeplFileContentLoader *loader;
	GError *error = NULL;

	loader = g_task_get_source_object (task);

	g_clear_object (&loader->priv->info);
	loader->priv->info = g_file_query_info_finish (location, result, &error);

	if (error != NULL)
	{
		g_task_return_error (task, error);
		return;
	}

	check_file_size (task);
}

static void
query_info_on_location (GTask *task)
{
	TeplFileContentLoader *loader;

	loader = g_task_get_source_object (task);

	/* At least G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE cannot be queried from the
	 * GFileInputStream. Normally querying from the GFile always works (in
	 * normal conditions).
	 */
	g_file_query_info_async (loader->priv->location,
				 G_FILE_ATTRIBUTE_STANDARD_SIZE ","
				 G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE,
				 G_FILE_QUERY_INFO_NONE,
				 g_task_get_priority (task),
				 g_task_get_cancellable (task),
				 query_info_on_location_cb,
				 task);
}

static void
query_info_on_file_input_stream_cb (GObject      *source_object,
				    GAsyncResult *result,
				    gpointer      user_data)
{
	GFileInputStream *file_input_stream = G_FILE_INPUT_STREAM (source_object);
	GTask *task = G_TASK (user_data);
	TeplFileContentLoader *loader;
	GFileInfo *info;
	GError *error = NULL;

	loader = g_task_get_source_object (task);

	info = g_file_input_stream_query_info_finish (file_input_stream, result, &error);

	if (error != NULL)
	{
		g_task_return_error (task, error);
		g_clear_object (&info);
		return;
	}

	if (g_file_info_has_attribute (info, G_FILE_ATTRIBUTE_ETAG_VALUE))
	{
		g_free (loader->priv->etag);
		loader->priv->etag = g_strdup (g_file_info_get_etag (info));
	}

	query_info_on_location (task);
	g_object_unref (info);
}

static void
query_info_on_file_input_stream (GTask *task)
{
	TaskData *task_data;

	task_data = g_task_get_task_data (task);

	/* See the docs of g_file_replace(), it is apparently better to query
	 * the etag on the GFileInputStream. Maybe to avoid a race condition if
	 * another program modifies the file while we are reading it.
	 */
	g_file_input_stream_query_info_async (task_data->file_input_stream,
					      G_FILE_ATTRIBUTE_ETAG_VALUE,
					      g_task_get_priority (task),
					      g_task_get_cancellable (task),
					      query_info_on_file_input_stream_cb,
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

	query_info_on_file_input_stream (task);
}

static void
open_file (GTask *task)
{
	TeplFileContentLoader *loader;

	loader = g_task_get_source_object (task);

	g_file_read_async (loader->priv->location,
			   g_task_get_priority (task),
			   g_task_get_cancellable (task),
			   open_file_cb,
			   task);
}

/*
 * _tepl_file_content_loader_load_async:
 * @loader: a #TeplFileContentLoader.
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
_tepl_file_content_loader_load_async (TeplFileContentLoader *loader,
				      gint                   io_priority,
				      GCancellable          *cancellable,
				      GFileProgressCallback  progress_callback,
				      gpointer               progress_callback_data,
				      GDestroyNotify         progress_callback_notify,
				      GAsyncReadyCallback    callback,
				      gpointer               user_data)
{
	TaskData *task_data;

	g_return_if_fail (TEPL_IS_FILE_CONTENT_LOADER (loader));
	g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));

	if (loader->priv->task != NULL)
	{
		g_warning ("Several load operations in parallel with the same "
			   "TeplFileContentLoader is not possible and doesn't make sense.");
		return;
	}

	reset (loader);

	loader->priv->task = g_task_new (loader, cancellable, callback, user_data);
	g_task_set_priority (loader->priv->task, io_priority);

	task_data = task_data_new ();
	g_task_set_task_data (loader->priv->task, task_data, task_data_free);

	task_data->progress_cb = progress_callback;
	task_data->progress_cb_data = progress_callback_data;
	task_data->progress_cb_notify = progress_callback_notify;

	open_file (loader->priv->task);
}

/*
 * _tepl_file_content_loader_load_finish:
 * @loader: a #TeplFileContentLoader.
 * @result: a #GAsyncResult.
 * @error: a #GError, or %NULL.
 *
 * Finishes a file loading started with _tepl_file_content_loader_load_async().
 *
 * If the file is too big, the %TEPL_FILE_LOADER_ERROR_TOO_BIG error is
 * returned.
 *
 * Returns: whether the content has been loaded successfully.
 */
gboolean
_tepl_file_content_loader_load_finish (TeplFileContentLoader  *loader,
				       GAsyncResult           *result,
				       GError                **error)
{
	gboolean ok;

	g_return_val_if_fail (TEPL_IS_FILE_CONTENT_LOADER (loader), FALSE);
	g_return_val_if_fail (g_task_is_valid (result, loader), FALSE);
	g_return_val_if_fail (G_TASK (result) == loader->priv->task, FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	ok = g_task_propagate_boolean (loader->priv->task, error);

	g_clear_object (&loader->priv->task);
	return ok;
}

/*
 * Should be called only after a successful load operation.
 *
 * Returns: (transfer none): the content that has been loaded by the last load
 * operation on @loader.
 */
TeplFileContent *
_tepl_file_content_loader_get_content (TeplFileContentLoader *loader)
{
	g_return_val_if_fail (TEPL_IS_FILE_CONTENT_LOADER (loader), NULL);

	if (loader->priv->content == NULL)
	{
		loader->priv->content = _tepl_file_content_new ();
	}

	return loader->priv->content;
}

/* Should be called only after a successful load operation. */
const gchar *
_tepl_file_content_loader_get_etag (TeplFileContentLoader *loader)
{
	g_return_val_if_fail (TEPL_IS_FILE_CONTENT_LOADER (loader), NULL);

	return loader->priv->etag;
}

/* Should be called only after a successful load operation. */
gboolean
_tepl_file_content_loader_get_readonly (TeplFileContentLoader *loader)
{
	g_return_val_if_fail (TEPL_IS_FILE_CONTENT_LOADER (loader), FALSE);
	g_return_val_if_fail (loader->priv->info != NULL, FALSE);

	if (g_file_info_has_attribute (loader->priv->info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE))
	{
		return !g_file_info_get_attribute_boolean (loader->priv->info,
							   G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
	}

	return FALSE;
}
