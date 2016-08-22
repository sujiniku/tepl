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

#include "gtef-file-loader.h"
#include "gtef-buffer.h"
#include "gtef-file.h"

/**
 * SECTION:file-loader
 * @Short_description: Load a file into a GtefBuffer
 * @Title: GtefFileLoader
 */

typedef struct _GtefFileLoaderPrivate GtefFileLoaderPrivate;

struct _GtefFileLoaderPrivate
{
	/* Weak ref to the GtefBuffer. A strong ref could create a reference
	 * cycle in an application. For example a subclass of GtefBuffer can
	 * have a strong ref to the FileLoader.
	 */
	GtefBuffer *buffer;

	GFile *location;

	GTask *task;
};

enum
{
	PROP_0,
	PROP_BUFFER,
	PROP_LOCATION,
	N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

G_DEFINE_TYPE_WITH_PRIVATE (GtefFileLoader, gtef_file_loader, G_TYPE_OBJECT)

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

		case PROP_LOCATION:
			g_value_set_object (value, gtef_file_loader_get_location (loader));
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
	GtefFileLoaderPrivate *priv = gtef_file_loader_get_instance_private (GTEF_FILE_LOADER (object));

	switch (prop_id)
	{
		case PROP_BUFFER:
			g_assert (priv->buffer == NULL);
			priv->buffer = g_value_get_object (value);
			g_object_add_weak_pointer (G_OBJECT (priv->buffer),
						   (gpointer *) &priv->buffer);
			break;

		case PROP_LOCATION:
			g_assert (priv->location == NULL);
			priv->location = g_value_dup_object (value);
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

	if (priv->buffer != NULL &&
	    priv->location == NULL)
	{
		GtefFile *file;

		file = gtef_buffer_get_file (priv->buffer);
		priv->location = gtef_file_get_location (file);

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

	g_clear_object (&priv->location);
	g_clear_object (&priv->task);

	G_OBJECT_CLASS (gtef_file_loader_parent_class)->dispose (object);
}

static void
gtef_file_loader_class_init (GtefFileLoaderClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gtef_file_loader_get_property;
	object_class->set_property = gtef_file_loader_set_property;
	object_class->constructed = gtef_file_loader_constructed;
	object_class->dispose = gtef_file_loader_dispose;

	/**
	 * GtefFileLoader:buffer:
	 *
	 * The #GtefBuffer to load the contents into. The #GtefFileLoader object
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

	g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
gtef_file_loader_init (GtefFileLoader *loader)
{
}

/**
 * gtef_file_loader_new:
 * @buffer: the #GtefBuffer to load the contents into.
 *
 * Creates a new #GtefFileLoader object. The contents is read from the #GtefFile
 * location. If not already done, call gtef_file_set_location() before calling
 * this constructor. The previous location is anyway not needed, because as soon
 * as the file loading begins, the @buffer is emptied.
 *
 * Returns: a new #GtefFileLoader object.
 * Since: 1.0
 */
GtefFileLoader *
gtef_file_loader_new (GtefBuffer *buffer)
{
	g_return_val_if_fail (GTEF_IS_BUFFER (buffer), NULL);

	return g_object_new (GTEF_TYPE_FILE_LOADER,
			     "buffer", buffer,
			     NULL);
}

/**
 * gtef_file_loader_get_buffer:
 * @loader: a #GtefFileLoader.
 *
 * Returns: (transfer none) (nullable): the #GtefBuffer to load the contents
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

static void
load_contents_cb (GObject      *source_object,
		  GAsyncResult *result,
		  gpointer      user_data)
{
	GFile *location = G_FILE (source_object);
	GTask *task = G_TASK (user_data);
	GtefFileLoader *loader;
	GtefFileLoaderPrivate *priv;
	gchar *contents = NULL;
	gsize length;
	GtkTextIter start;
	GError *error = NULL;

	loader = g_task_get_source_object (task);
	priv = gtef_file_loader_get_instance_private (loader);

	g_file_load_contents_finish (location,
				     result,
				     &contents,
				     &length,
				     NULL,
				     &error);

	if (error != NULL)
	{
		g_task_return_error (task, error);
		goto out;
	}

	if (priv->buffer == NULL)
	{
		g_task_return_boolean (task, FALSE);
		goto out;
	}

	gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (priv->buffer), &start);

	gtk_text_buffer_insert (GTK_TEXT_BUFFER (priv->buffer),
				&start,
				contents,
				length);

	g_task_return_boolean (task, TRUE);

out:
	g_free (contents);
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

	g_file_load_contents_async (priv->location,
				    g_task_get_cancellable (task),
				    load_contents_cb,
				    task);
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

/**
 * gtef_file_loader_load_async:
 * @loader: a #GtefFileLoader.
 * @io_priority: the I/O priority of the request. E.g. %G_PRIORITY_LOW,
 *   %G_PRIORITY_DEFAULT or %G_PRIORITY_HIGH.
 * @cancellable: (nullable): optional #GCancellable object, %NULL to ignore.
 * @callback: (scope async): a #GAsyncReadyCallback to call when the request is
 *   satisfied.
 * @user_data: user data to pass to @callback.
 *
 * Loads asynchronously the file contents into the #GtefBuffer.
 *
 * See the #GAsyncResult documentation to know how to use this function.
 *
 * Since: 1.0
 */
void
gtef_file_loader_load_async (GtefFileLoader      *loader,
			     gint                 io_priority,
			     GCancellable        *cancellable,
			     GAsyncReadyCallback  callback,
			     gpointer             user_data)
{
	GtefFileLoaderPrivate *priv;

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

	priv->task = g_task_new (loader, cancellable, callback, user_data);
	g_task_set_priority (priv->task, io_priority);

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
 * Returns: whether the contents has been loaded successfully.
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

	g_clear_object (&priv->task);

	return ok;
}
