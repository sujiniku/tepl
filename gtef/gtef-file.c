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

#include "gtef-file.h"

/**
 * SECTION:file
 * @Short_description: On-disk representation of a GtkSourceBuffer
 * @Title: GtefFile
 *
 * #GtefFile extends #GtkSourceFile with metadata support.
 */

typedef struct _GtefFilePrivate GtefFilePrivate;

struct _GtefFilePrivate
{
	/* Never NULL */
	GFileInfo *metadata;
};

G_DEFINE_TYPE_WITH_PRIVATE (GtefFile, gtef_file, GTK_SOURCE_TYPE_FILE)

#define METADATA_PREFIX "metadata::"
#define METADATA_QUERY_ATTRIBUTES "metadata::*"

static gchar *
get_metadata_attribute_key (const gchar *key)
{
	return g_strconcat (METADATA_PREFIX, key, NULL);
}

static void
gtef_file_finalize (GObject *object)
{
	GtefFilePrivate *priv = gtef_file_get_instance_private (GTEF_FILE (object));

	g_object_unref (priv->metadata);

	G_OBJECT_CLASS (gtef_file_parent_class)->finalize (object);
}

static void
gtef_file_class_init (GtefFileClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = gtef_file_finalize;
}

static void
gtef_file_init (GtefFile *file)
{
	GtefFilePrivate *priv = gtef_file_get_instance_private (file);

	priv->metadata = g_file_info_new ();
}

/**
 * gtef_file_new:
 *
 * Returns: a new #GtefFile object.
 * Since: 1.0
 */
GtefFile *
gtef_file_new (void)
{
	return g_object_new (GTEF_TYPE_FILE, NULL);
}

/**
 * gtef_file_get_metadata:
 * @file: a #GtefFile.
 * @key: the name of the metadata.
 *
 * Gets the value of a metadata stored in the @file object memory.
 *
 * Returns: (nullable): the value of the metadata, or %NULL if the metadata
 *   doesn't exist. Free with g_free().
 * Since: 1.0
 */
gchar *
gtef_file_get_metadata (GtefFile    *file,
			const gchar *key)
{
	GtefFilePrivate *priv;
	gchar *attribute_key;
	gchar *value = NULL;

	g_return_val_if_fail (GTEF_IS_FILE (file), NULL);
	g_return_val_if_fail (key != NULL && key[0] != '\0', NULL);

	priv = gtef_file_get_instance_private (file);

	attribute_key = get_metadata_attribute_key (key);

	if (g_file_info_has_attribute (priv->metadata, attribute_key) &&
	    g_file_info_get_attribute_type (priv->metadata, attribute_key) == G_FILE_ATTRIBUTE_TYPE_STRING)
	{
		value = g_strdup (g_file_info_get_attribute_string (priv->metadata, attribute_key));
	}

	g_free (attribute_key);
	return value;
}

/**
 * gtef_file_set_metadata:
 * @file: a #GtefFile.
 * @key: the name of the metadata.
 * @value: (nullable): the value of the metadata, or %NULL to unset.
 *
 * Sets the value of a metadata. It's preferable that @key starts with a
 * namespace, to not get metadata conflicts between applications.
 *
 * This function just stores the new metadata value in the @file object memory.
 *
 * Since: 1.0
 */
void
gtef_file_set_metadata (GtefFile    *file,
			const gchar *key,
			const gchar *value)
{
	GtefFilePrivate *priv;
	gchar *attribute_key;

	g_return_if_fail (GTEF_IS_FILE (file));
	g_return_if_fail (key != NULL && key[0] != '\0');

	priv = gtef_file_get_instance_private (file);

	attribute_key = get_metadata_attribute_key (key);

	if (value != NULL)
	{
		g_file_info_set_attribute_string (priv->metadata,
						  attribute_key,
						  value);
	}
	else
	{
		/* Unset the key */
		g_file_info_set_attribute (priv->metadata,
					   attribute_key,
					   G_FILE_ATTRIBUTE_TYPE_INVALID,
					   NULL);
	}

	g_free (attribute_key);
}

/**
 * gtef_file_load_metadata:
 * @file: a #GtefFile.
 * @cancellable: (nullable): optional #GCancellable object, %NULL to ignore.
 * @error: location to a %NULL #GError, or %NULL.
 *
 * Loads synchronously the metadata from #GtkSourceFile:location.
 *
 * If the metadata are loaded successfully, this function deletes all previous
 * metadata stored in the @file object memory.
 *
 * Returns: whether the metadata was loaded successfully.
 * Since: 1.0
 */
gboolean
gtef_file_load_metadata (GtefFile      *file,
			 GCancellable  *cancellable,
			 GError       **error)
{
	GtefFilePrivate *priv;
	GFile *location;
	GFileInfo *metadata;

	g_return_val_if_fail (GTEF_IS_FILE (file), FALSE);
	g_return_val_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	priv = gtef_file_get_instance_private (file);

	location = gtk_source_file_get_location (GTK_SOURCE_FILE (file));
	if (location == NULL)
	{
		return FALSE;
	}

	metadata = g_file_query_info (location,
				      METADATA_QUERY_ATTRIBUTES,
				      G_FILE_QUERY_INFO_NONE,
				      cancellable,
				      error);

	if (metadata == NULL)
	{
		return FALSE;
	}

	g_object_unref (priv->metadata);
	priv->metadata = metadata;

	return TRUE;
}

static void
load_metadata_async_cb (GObject      *source_object,
			GAsyncResult *result,
			gpointer      user_data)
{
	GFile *location = G_FILE (source_object);
	GTask *task = G_TASK (user_data);
	GtefFile *file;
	GtefFilePrivate *priv;
	GFileInfo *metadata;
	GError *error = NULL;

	metadata = g_file_query_info_finish (location, result, &error);

	if (error != NULL)
	{
		g_task_return_error (task, error);
		g_object_unref (task);
		g_clear_object (&metadata);
		return;
	}

	if (metadata == NULL)
	{
		g_task_return_boolean (task, FALSE);
		g_object_unref (task);
		return;
	}

	file = g_task_get_source_object (task);
	priv = gtef_file_get_instance_private (file);

	g_object_unref (priv->metadata);
	priv->metadata = metadata;

	g_task_return_boolean (task, TRUE);
	g_object_unref (task);
}

/**
 * gtef_file_load_metadata_async:
 * @file: a #GtefFile.
 * @io_priority: the I/O priority of the request. E.g. %G_PRIORITY_LOW,
 *   %G_PRIORITY_DEFAULT or %G_PRIORITY_HIGH.
 * @cancellable: (nullable): optional #GCancellable object, %NULL to ignore.
 * @callback: (scope async): a #GAsyncReadyCallback to call when the request is
 *   satisfied.
 * @user_data: user data to pass to @callback.
 *
 * The asynchronous version of gtef_file_load_metadata().
 *
 * See the #GAsyncResult documentation to know how to use this function.
 *
 * Since: 1.0
 */
void
gtef_file_load_metadata_async (GtefFile            *file,
			       gint                 io_priority,
			       GCancellable        *cancellable,
			       GAsyncReadyCallback  callback,
			       gpointer             user_data)
{
	GTask *task;
	GFile *location;

	g_return_if_fail (GTEF_IS_FILE (file));
	g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));

	task = g_task_new (file, cancellable, callback, user_data);

	location = gtk_source_file_get_location (GTK_SOURCE_FILE (file));
	if (location == NULL)
	{
		g_task_return_boolean (task, FALSE);
		g_object_unref (task);
		return;
	}

	g_file_query_info_async (location,
				 METADATA_QUERY_ATTRIBUTES,
				 G_FILE_QUERY_INFO_NONE,
				 io_priority,
				 cancellable,
				 load_metadata_async_cb,
				 task);
}

/**
 * gtef_file_load_metadata_finish:
 * @file: a #GtefFile.
 * @result: a #GAsyncResult.
 * @error: location to a %NULL #GError, or %NULL.
 *
 * Finishes the metadata loading started with gtef_file_load_metadata_async().
 *
 * Returns: whether the metadata was loaded successfully.
 * Since: 1.0
 */
gboolean
gtef_file_load_metadata_finish (GtefFile      *file,
				GAsyncResult  *result,
				GError       **error)
{
	g_return_val_if_fail (GTEF_IS_FILE (file), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail (g_task_is_valid (result, file), FALSE);

	return g_task_propagate_boolean (G_TASK (result), error);
}
