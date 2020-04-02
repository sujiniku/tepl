/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2016-2020 - Sébastien Wilmet <swilmet@gnome.org>
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

#include "tepl-file-metadata.h"

/**
 * SECTION:file-metadata
 * @Short_description: File metadata
 * @Title: TeplFileMetadata
 *
 * A #TeplFileMetadata object stores the metadata of a #GFile.
 *
 * You need to load and save the #TeplMetadataStore in your application, it is
 * used as a fallback in #TeplFileMetadata in the case where GVfs metadata is
 * not supported. TODO: update.
 *
 * The tepl_file_metadata_get() and tepl_file_metadata_set() functions don't
 * load or save the metadata on disk, they only access the metadata stored in
 * the #TeplFileMetadata object memory. To load the metadata from disk, call
 * tepl_file_metadata_load_async(). Likewise, to save the
 * metadata on disk, call tepl_file_metadata_save_async().
 */

/* TODO Better test how it works with remote files, with various protocols.
 * For example with an ftp://... location, there can be the error "The specified
 * location is not mounted". In that case we can either propagate the error or
 * automatically call the TeplFile mount operation factory method.
 *
 * On Linux, is the metadata supported for all GVfs backends? (the custom
 * metadata that we set). Does it fallback to TeplMetadataStore even on Linux
 * and even when GVfs is well installed?
 */

typedef struct _TeplFileMetadataPrivate TeplFileMetadataPrivate;

struct _TeplFileMetadataPrivate
{
	/* Never NULL */
	GFileInfo *file_info;

#if 0
	guint use_gvfs_metadata : 1;
#endif
};

G_DEFINE_TYPE_WITH_PRIVATE (TeplFileMetadata, tepl_file_metadata, G_TYPE_OBJECT)

#define METADATA_PREFIX "metadata::"
#define METADATA_QUERY_ATTRIBUTES "metadata::*"

static gchar *
get_metadata_attribute_key (const gchar *key)
{
	return g_strconcat (METADATA_PREFIX, key, NULL);
}

static void
print_fallback_to_metadata_store_warning (void)
{
	g_warning_once ("GVfs metadata is not supported. Fallback to TeplMetadataStore. "
			"Either GVfs is not correctly installed or GVfs metadata are "
			"not supported on this platform. In the latter case, you should "
			"configure Tepl with -Dgvfs_metadata=false.");
}

static void
tepl_file_metadata_finalize (GObject *object)
{
	TeplFileMetadataPrivate *priv;

	priv = tepl_file_metadata_get_instance_private (TEPL_FILE_METADATA (object));

	g_object_unref (priv->file_info);

	G_OBJECT_CLASS (tepl_file_metadata_parent_class)->finalize (object);
}

static void
tepl_file_metadata_class_init (TeplFileMetadataClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = tepl_file_metadata_finalize;
}

static void
tepl_file_metadata_init (TeplFileMetadata *metadata)
{
	TeplFileMetadataPrivate *priv = tepl_file_metadata_get_instance_private (metadata);

	priv->file_info = g_file_info_new ();

#if 0
/* TODO change the #ifdef to an #if. */
#ifdef ENABLE_GVFS_METADATA
	priv->use_gvfs_metadata = TRUE;
#else
	priv->use_gvfs_metadata = FALSE;
#endif
#endif
}

/**
 * tepl_file_metadata_new:
 *
 * Returns: a new #TeplFileMetadata object.
 * Since: 5.0
 */
TeplFileMetadata *
tepl_file_metadata_new (void)
{
	return g_object_new (TEPL_TYPE_FILE_METADATA, NULL);
}

/**
 * tepl_file_metadata_get:
 * @metadata: a #TeplFileMetadata.
 * @key: the name of the metadata.
 *
 * Gets the value of a metadata stored in the @metadata object memory.
 *
 * Returns: (nullable): the value of the metadata, or %NULL if the metadata
 *   doesn't exist. Free with g_free().
 * Since: 1.0
 */
gchar *
tepl_file_metadata_get (TeplFileMetadata *metadata,
			const gchar      *key)
{
	TeplFileMetadataPrivate *priv;
	gchar *attribute_key;
	gchar *value = NULL;

	g_return_val_if_fail (TEPL_IS_FILE_METADATA (metadata), NULL);
	g_return_val_if_fail (key != NULL && key[0] != '\0', NULL);

	priv = tepl_file_metadata_get_instance_private (metadata);

	attribute_key = get_metadata_attribute_key (key);

	if (g_file_info_has_attribute (priv->file_info, attribute_key) &&
	    g_file_info_get_attribute_type (priv->file_info, attribute_key) == G_FILE_ATTRIBUTE_TYPE_STRING)
	{
		value = g_strdup (g_file_info_get_attribute_string (priv->file_info, attribute_key));
	}

	g_free (attribute_key);
	return value;
}

/**
 * tepl_file_metadata_set:
 * @metadata: a #TeplFileMetadata.
 * @key: the name of the metadata.
 * @value: (nullable): the value of the metadata, or %NULL to unset.
 *
 * Sets the value of a metadata. It's preferable that @key starts with a
 * namespace, to not get metadata conflicts between applications. For example a
 * good @key name for the gedit application is
 * `"gedit-spell-checking-language"`.
 *
 * This function just stores the new metadata value in the @metadata object
 * memory.
 *
 * Since: 1.0
 */
void
tepl_file_metadata_set (TeplFileMetadata *metadata,
			const gchar      *key,
			const gchar      *value)
{
	TeplFileMetadataPrivate *priv;
	gchar *attribute_key;

	g_return_if_fail (TEPL_IS_FILE_METADATA (metadata));
	g_return_if_fail (key != NULL && key[0] != '\0');

	priv = tepl_file_metadata_get_instance_private (metadata);

	attribute_key = get_metadata_attribute_key (key);

	if (value != NULL)
	{
		g_file_info_set_attribute_string (priv->file_info,
						  attribute_key,
						  value);
	}
	else
	{
		/* Unset the key */
		g_file_info_set_attribute (priv->file_info,
					   attribute_key,
					   G_FILE_ATTRIBUTE_TYPE_INVALID,
					   NULL);
	}

	g_free (attribute_key);
}

static void
load_metadata_async_cb (GObject      *source_object,
			GAsyncResult *result,
			gpointer      user_data)
{
	GFile *location = G_FILE (source_object);
	GTask *task = G_TASK (user_data);
	TeplFileMetadata *metadata;
	TeplFileMetadataPrivate *priv;
	GFileInfo *file_info;
	GError *error = NULL;

	metadata = g_task_get_source_object (task);
	priv = tepl_file_metadata_get_instance_private (metadata);

	file_info = g_file_query_info_finish (location, result, &error);

	if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED))
	{
		print_fallback_to_metadata_store_warning ();
		//priv->use_gvfs_metadata = FALSE;

		g_clear_error (&error);
		g_clear_object (&file_info);

		/* TODO: load from TeplMetadataStore. */
		file_info = NULL;
	}

	if (error != NULL)
	{
		g_task_return_error (task, error);
		g_object_unref (task);
		g_clear_object (&file_info);
		return;
	}

	if (file_info == NULL)
	{
		/* FIXME: return TRUE instead? */
		g_task_return_boolean (task, FALSE);
		g_object_unref (task);
		return;
	}

	g_set_object (&priv->file_info, file_info);

	g_task_return_boolean (task, TRUE);
	g_object_unref (task);
}

/**
 * tepl_file_metadata_load_async:
 * @metadata: a #TeplFileMetadata.
 * @location: a #GFile.
 * @io_priority: the I/O priority of the request. E.g. %G_PRIORITY_LOW,
 *   %G_PRIORITY_DEFAULT or %G_PRIORITY_HIGH.
 * @cancellable: (nullable): optional #GCancellable object, %NULL to ignore.
 * @callback: (scope async): a #GAsyncReadyCallback to call when the request is
 *   satisfied.
 * @user_data: user data to pass to @callback.
 *
 * Loads asynchronously the metadata for @location.
 *
 * If the metadata are loaded successfully, this function deletes all previous
 * metadata stored in the @metadata object memory.
 *
 * @location must exist on the filesystem, otherwise an error is returned.
 *
 * See the #GAsyncResult documentation to know how to use this function.
 *
 * Since: 5.0
 */
void
tepl_file_metadata_load_async (TeplFileMetadata    *metadata,
			       GFile               *location,
			       gint                 io_priority,
			       GCancellable        *cancellable,
			       GAsyncReadyCallback  callback,
			       gpointer             user_data)
{
	GTask *task;

	g_return_if_fail (TEPL_IS_FILE_METADATA (metadata));
	g_return_if_fail (G_IS_FILE (location));
	g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));

	task = g_task_new (metadata, cancellable, callback, user_data);
	g_task_set_priority (task, io_priority);

	g_file_query_info_async (location,
				 METADATA_QUERY_ATTRIBUTES,
				 G_FILE_QUERY_INFO_NONE,
				 io_priority,
				 cancellable,
				 load_metadata_async_cb,
				 task);
}

/**
 * tepl_file_metadata_load_finish:
 * @metadata: a #TeplFileMetadata.
 * @result: a #GAsyncResult.
 * @error: location to a %NULL #GError, or %NULL.
 *
 * Finishes the metadata loading started with tepl_file_metadata_load_async().
 *
 * Returns: whether the metadata was loaded successfully.
 * Since: 1.0
 */
gboolean
tepl_file_metadata_load_finish (TeplFileMetadata  *metadata,
				GAsyncResult      *result,
				GError           **error)
{
	g_return_val_if_fail (TEPL_IS_FILE_METADATA (metadata), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail (g_task_is_valid (result, metadata), FALSE);

	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
save_metadata_async_cb (GObject      *source_object,
			GAsyncResult *result,
			gpointer      user_data)
{
	GFile *location = G_FILE (source_object);
	GTask *task = G_TASK (user_data);
	GError *error = NULL;

	g_file_set_attributes_finish (location, result, NULL, &error);

	if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED))
	{
		print_fallback_to_metadata_store_warning ();
		g_clear_error (&error);
	}

	if (error != NULL)
	{
		g_task_return_error (task, error);
		g_object_unref (task);
		return;
	}

	g_task_return_boolean (task, TRUE);
	g_object_unref (task);
}

/**
 * tepl_file_metadata_save_async:
 * @metadata: a #TeplFileMetadata.
 * @location: a #GFile.
 * @io_priority: the I/O priority of the request. E.g. %G_PRIORITY_LOW,
 *   %G_PRIORITY_DEFAULT or %G_PRIORITY_HIGH.
 * @cancellable: (nullable): optional #GCancellable object, %NULL to ignore.
 * @callback: (scope async): a #GAsyncReadyCallback to call when the request is
 *   satisfied.
 * @user_data: user data to pass to @callback.
 *
 * Saves asynchronously the metadata for @location.
 *
 * @location must exist on the filesystem, otherwise an error is returned.
 *
 * See the #GAsyncResult documentation to know how to use this function.
 *
 * Since: 5.0
 */
void
tepl_file_metadata_save_async (TeplFileMetadata    *metadata,
			       GFile               *location,
			       gint                 io_priority,
			       GCancellable        *cancellable,
			       GAsyncReadyCallback  callback,
			       gpointer             user_data)
{
	TeplFileMetadataPrivate *priv;
	GTask *task;

	g_return_if_fail (TEPL_IS_FILE_METADATA (metadata));
	g_return_if_fail (G_IS_FILE (location));
	g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));

	priv = tepl_file_metadata_get_instance_private (metadata);

	task = g_task_new (metadata, cancellable, callback, user_data);
	g_task_set_priority (task, io_priority);

	g_file_set_attributes_async (location,
				     priv->file_info,
				     G_FILE_QUERY_INFO_NONE,
				     io_priority,
				     cancellable,
				     save_metadata_async_cb,
				     task);
}

/**
 * tepl_file_metadata_save_finish:
 * @metadata: a #TeplFileMetadata.
 * @result: a #GAsyncResult.
 * @error: location to a %NULL #GError, or %NULL.
 *
 * Finishes the metadata saving started with tepl_file_metadata_save_async().
 *
 * Returns: whether the metadata was saved successfully.
 * Since: 1.0
 */
gboolean
tepl_file_metadata_save_finish (TeplFileMetadata  *metadata,
				GAsyncResult      *result,
				GError           **error)
{
	g_return_val_if_fail (TEPL_IS_FILE_METADATA (metadata), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail (g_task_is_valid (result, metadata), FALSE);

	return g_task_propagate_boolean (G_TASK (result), error);
}

#if 0
/* For unit tests. */
void
_tepl_file_metadata_set_use_gvfs_metadata (TeplFileMetadata *metadata,
					   gboolean          use_gvfs_metadata)
{
	TeplFileMetadataPrivate *priv;

	g_return_if_fail (TEPL_IS_FILE_METADATA (metadata));

	priv = tepl_file_metadata_get_instance_private (metadata);

	priv->use_gvfs_metadata = use_gvfs_metadata != FALSE;
}
#endif
