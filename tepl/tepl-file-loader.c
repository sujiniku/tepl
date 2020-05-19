/* SPDX-FileCopyrightText: 2016-2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "config.h"
#include "tepl-file-loader.h"
#include <glib/gi18n-lib.h>

/**
 * SECTION:file-loader
 * @Title: TeplFileLoader
 * @Short_description: Load a file into a #TeplBuffer
 * @See_also: #TeplFile, #TeplFileSaver
 *
 * A #TeplFileLoader object permits to load the content of a #GFile into a
 * #TeplBuffer.
 *
 * A file loader should be used only for one load operation, including errors
 * handling. If an error occurs, you can reconfigure the loader and relaunch the
 * operation with tepl_file_loader_load_async().
 *
 * Running a #TeplFileLoader is an undoable action for the #TeplBuffer. That is,
 * gtk_source_buffer_begin_not_undoable_action() and
 * gtk_source_buffer_end_not_undoable_action() are called, which delete the
 * undo/redo history.
 *
 * After a file loading, the buffer is reset to the content provided by the
 * #GFile, so the buffer is set as “unmodified”, that is,
 * gtk_text_buffer_set_modified() is called with %FALSE.
 */

struct _TeplFileLoaderPrivate
{
	/* Weak ref to the TeplBuffer. A strong ref could create a reference
	 * cycle in an application. For example a subclass of TeplBuffer can
	 * have a strong ref to the FileLoader.
	 */
	TeplBuffer *buffer;

	/* Weak ref to the TeplFile. A strong ref could create a reference
	 * cycle in an application. For example a subclass of TeplFile can
	 * have a strong ref to the FileLoader.
	 */
	TeplFile *file;

	GFile *location;

	guint is_loading : 1;
};

enum
{
	PROP_0,
	PROP_BUFFER,
	PROP_FILE,
	PROP_LOCATION,
	N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

G_DEFINE_TYPE_WITH_PRIVATE (TeplFileLoader, tepl_file_loader, G_TYPE_OBJECT)

static void
tepl_file_loader_get_property (GObject    *object,
			       guint       prop_id,
			       GValue     *value,
			       GParamSpec *pspec)
{
	TeplFileLoader *loader = TEPL_FILE_LOADER (object);

	switch (prop_id)
	{
		case PROP_BUFFER:
			g_value_set_object (value, tepl_file_loader_get_buffer (loader));
			break;

		case PROP_FILE:
			g_value_set_object (value, tepl_file_loader_get_file (loader));
			break;

		case PROP_LOCATION:
			g_value_set_object (value, tepl_file_loader_get_location (loader));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_file_loader_set_property (GObject      *object,
			       guint         prop_id,
			       const GValue *value,
			       GParamSpec   *pspec)
{
	TeplFileLoader *loader = TEPL_FILE_LOADER (object);

	switch (prop_id)
	{
		case PROP_BUFFER:
			g_assert (loader->priv->buffer == NULL);
			g_set_weak_pointer (&loader->priv->buffer, g_value_get_object (value));
			break;

		case PROP_FILE:
			g_assert (loader->priv->file == NULL);
			g_set_weak_pointer (&loader->priv->file, g_value_get_object (value));
			break;

		case PROP_LOCATION:
			g_assert (loader->priv->location == NULL);
			loader->priv->location = g_value_dup_object (value);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_file_loader_constructed (GObject *object)
{
	TeplFileLoader *loader = TEPL_FILE_LOADER (object);

	G_OBJECT_CLASS (tepl_file_loader_parent_class)->constructed (object);

	if (loader->priv->file != NULL &&
	    loader->priv->location == NULL)
	{
		loader->priv->location = tepl_file_get_location (loader->priv->file);

		if (loader->priv->location != NULL)
		{
			g_object_ref (loader->priv->location);
		}
		else
		{
			g_warning ("TeplFileLoader: the TeplFile location is NULL. "
				   "Call tepl_file_set_location() before creating the FileLoader.");
		}
	}
}

static void
tepl_file_loader_dispose (GObject *object)
{
	TeplFileLoader *loader = TEPL_FILE_LOADER (object);

	g_clear_weak_pointer (&loader->priv->buffer);
	g_clear_weak_pointer (&loader->priv->file);
	g_clear_object (&loader->priv->location);

	G_OBJECT_CLASS (tepl_file_loader_parent_class)->dispose (object);
}

static void
tepl_file_loader_class_init (TeplFileLoaderClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = tepl_file_loader_get_property;
	object_class->set_property = tepl_file_loader_set_property;
	object_class->constructed = tepl_file_loader_constructed;
	object_class->dispose = tepl_file_loader_dispose;

	/**
	 * TeplFileLoader:buffer:
	 *
	 * The #TeplBuffer to load the content into. The #TeplFileLoader object
	 * has a weak reference to the buffer.
	 *
	 * Since: 1.0
	 */
	properties[PROP_BUFFER] =
		g_param_spec_object ("buffer",
				     "buffer",
				     "",
				     TEPL_TYPE_BUFFER,
				     G_PARAM_READWRITE |
				     G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS);

	/**
	 * TeplFileLoader:file:
	 *
	 * The #TeplFile. The #TeplFileLoader object has a weak
	 * reference to the file.
	 *
	 * Since: 1.0
	 */
	properties[PROP_FILE] =
		g_param_spec_object ("file",
				     "file",
				     "",
				     TEPL_TYPE_FILE,
				     G_PARAM_READWRITE |
				     G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS);

	/**
	 * TeplFileLoader:location:
	 *
	 * The #GFile to load. By default the location is taken from the
	 * #TeplFile at construction time.
	 *
	 * Since: 1.0
	 */
	properties[PROP_LOCATION] =
		g_param_spec_object ("location",
				     "location",
				     "",
				     G_TYPE_FILE,
				     G_PARAM_READWRITE |
				     G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
tepl_file_loader_init (TeplFileLoader *loader)
{
	loader->priv = tepl_file_loader_get_instance_private (loader);
}

/**
 * tepl_file_loader_new:
 * @buffer: the #TeplBuffer to load the content into.
 * @file: the #TeplFile.
 *
 * Creates a new #TeplFileLoader object. The content is read from the #TeplFile
 * location.
 *
 * If not already done, call tepl_file_set_location() before calling this
 * constructor. The previous location is anyway not needed, because as soon as
 * the file loading begins, the @buffer is emptied. Setting the #TeplFile
 * location directly permits to update the UI, to display the good location when
 * the file is loading.
 *
 * Returns: a new #TeplFileLoader object.
 * Since: 1.0
 */
TeplFileLoader *
tepl_file_loader_new (TeplBuffer *buffer,
		      TeplFile   *file)
{
	g_return_val_if_fail (TEPL_IS_BUFFER (buffer), NULL);
	g_return_val_if_fail (TEPL_IS_FILE (file), NULL);

	return g_object_new (TEPL_TYPE_FILE_LOADER,
			     "buffer", buffer,
			     "file", file,
			     NULL);
}

/**
 * tepl_file_loader_get_buffer:
 * @loader: a #TeplFileLoader.
 *
 * Returns: (transfer none) (nullable): the #TeplBuffer to load the content
 * into.
 * Since: 1.0
 */
TeplBuffer *
tepl_file_loader_get_buffer (TeplFileLoader *loader)
{
	g_return_val_if_fail (TEPL_IS_FILE_LOADER (loader), NULL);

	return loader->priv->buffer;
}

/**
 * tepl_file_loader_get_file:
 * @loader: a #TeplFileLoader.
 *
 * Returns: (transfer none) (nullable): the #TeplFile.
 * Since: 1.0
 */
TeplFile *
tepl_file_loader_get_file (TeplFileLoader *loader)
{
	g_return_val_if_fail (TEPL_IS_FILE_LOADER (loader), NULL);

	return loader->priv->file;
}

/**
 * tepl_file_loader_get_location:
 * @loader: a #TeplFileLoader.
 *
 * Returns: (transfer none) (nullable): the #GFile to load.
 * Since: 1.0
 */
GFile *
tepl_file_loader_get_location (TeplFileLoader *loader)
{
	g_return_val_if_fail (TEPL_IS_FILE_LOADER (loader), NULL);

	return loader->priv->location;
}

static void
load_contents_cb (GObject      *source_object,
		  GAsyncResult *result,
		  gpointer      user_data)
{
	GFile *location = G_FILE (source_object);
	GTask *task = G_TASK (user_data);
	TeplFileLoader *loader = g_task_get_source_object (task);
	gchar *content = NULL;
	gsize content_length = 0;
	GError *error = NULL;

	g_file_load_contents_finish (location,
				     result,
				     &content,
				     &content_length,
				     NULL,
				     &error);

	if (error != NULL)
	{
		g_task_return_error (task, error);
		g_object_unref (task);
		goto out;
	}

	if (!g_utf8_validate_len (content, content_length, NULL))
	{
		g_task_return_new_error (task,
					 G_IO_ERROR,
					 G_IO_ERROR_INVALID_DATA,
					 _("The content must be encoded with the UTF-8 character encoding."));
		g_object_unref (task);
		goto out;
	}

	if (loader->priv->buffer != NULL)
	{
		GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (loader->priv->buffer);
		GtkTextIter start;

		gtk_text_buffer_set_text (text_buffer, content, content_length);

		gtk_text_buffer_get_start_iter (text_buffer, &start);
		gtk_text_buffer_place_cursor (text_buffer, &start);
	}

	g_task_return_boolean (task, TRUE);
	g_object_unref (task);

out:
	g_free (content);
}

/**
 * tepl_file_loader_load_async:
 * @loader: a #TeplFileLoader.
 * @io_priority: the I/O priority of the request. E.g. %G_PRIORITY_LOW,
 *   %G_PRIORITY_DEFAULT or %G_PRIORITY_HIGH.
 * @cancellable: (nullable): optional #GCancellable object, %NULL to ignore.
 * @callback: (scope async): a #GAsyncReadyCallback to call when the request is
 *   satisfied.
 * @user_data: user data to pass to @callback.
 *
 * Loads asynchronously the file content into the #TeplBuffer.
 *
 * See the #GAsyncResult documentation to know how to use this function.
 *
 * Since: 5.0
 */
void
tepl_file_loader_load_async (TeplFileLoader      *loader,
			     gint                 io_priority,
			     GCancellable        *cancellable,
			     GAsyncReadyCallback  callback,
			     gpointer             user_data)
{
	GTask *task;

	g_return_if_fail (TEPL_IS_FILE_LOADER (loader));
	g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));
	g_return_if_fail (!loader->priv->is_loading);

	loader->priv->is_loading = TRUE;

	task = g_task_new (loader, cancellable, callback, user_data);
	g_task_set_priority (task, io_priority);

	if (loader->priv->buffer == NULL ||
	    loader->priv->file == NULL ||
	    loader->priv->location == NULL)
	{
		g_task_return_boolean (task, FALSE);
		g_object_unref (task);
		return;
	}

	gtk_source_buffer_begin_not_undoable_action (GTK_SOURCE_BUFFER (loader->priv->buffer));
	gtk_text_buffer_set_text (GTK_TEXT_BUFFER (loader->priv->buffer), "", -1);
	gtk_text_buffer_set_modified (GTK_TEXT_BUFFER (loader->priv->buffer), FALSE);

	g_file_load_contents_async (loader->priv->location,
				    cancellable,
				    load_contents_cb,
				    task);
}

/**
 * tepl_file_loader_load_finish:
 * @loader: a #TeplFileLoader.
 * @result: a #GAsyncResult.
 * @error: a #GError, or %NULL.
 *
 * Finishes a file loading started with tepl_file_loader_load_async().
 *
 * Returns: whether the content has been loaded successfully.
 * Since: 1.0
 */
gboolean
tepl_file_loader_load_finish (TeplFileLoader  *loader,
			      GAsyncResult    *result,
			      GError         **error)
{
	g_return_val_if_fail (TEPL_IS_FILE_LOADER (loader), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail (g_task_is_valid (result, loader), FALSE);

	if (loader->priv->buffer != NULL)
	{
		gtk_source_buffer_end_not_undoable_action (GTK_SOURCE_BUFFER (loader->priv->buffer));
		gtk_text_buffer_set_modified (GTK_TEXT_BUFFER (loader->priv->buffer), FALSE);
	}

	loader->priv->is_loading = FALSE;
	return g_task_propagate_boolean (G_TASK (result), error);
}
