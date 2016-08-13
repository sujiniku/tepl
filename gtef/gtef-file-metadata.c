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
#include "gtef-file-metadata.h"
#include <glib/gi18n-lib.h>
#include "gtef-file.h"
#include "gtef-metadata-manager.h"

/**
 * SECTION:file-metadata
 * @Short_description: File metadata
 * @Title: GtefFileMetadata
 *
 * A #GtefFileMetadata object stores the metadata of a #GtefFile. You need to
 * call gtef_metadata_manager_init() and gtef_metadata_manager_shutdown() in
 * your application, in case GVfs metadata are not supported.
 *
 * gtef_file_metadata_get() and gtef_file_metadata_set() don't load or save the
 * metadata on disk. They only access the metadata stored in the
 * #GtefFileMetadata object memory. To load the metadata from disk, call
 * gtef_file_metadata_load() or its async variant. Likewise, to save the
 * metadata on disk, call gtef_file_metadata_save() or its async variant. When
 * loading or saving metadata, the file at #GtkSourceFile:location, if
 * non-%NULL, must exist on the filesystem, otherwise an error is returned.
 *
 * When the #GtkSourceFile:location changes, the metadata are still kept in the
 * #GtefFileMetadata object memory. But the metadata are
 * <emphasis>not</emphasis> automatically saved for the new location.
 */

/* TODO Better test how it works with remote files, with various protocols.
 * For example with an ftp://... location, there can be the error "The specified
 * location is not mounted". In that case we can either propagate the error or
 * automatically call the GtkSourceFile mount operation factory method.
 *
 * On Linux, is the metadata supported for all GVfs backends? (the custom
 * metadata that we set). Does it fallback to the metadata manager even on
 * Linux?
 */

typedef struct _GtefFileMetadataPrivate GtefFileMetadataPrivate;

struct _GtefFileMetadataPrivate
{
	/* Weak ref */
	GtefFile *file;

	/* Never NULL */
	GFileInfo *file_info;

	guint use_gvfs_metadata : 1;
};

enum
{
	PROP_0,
	PROP_FILE,
	N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

G_DEFINE_TYPE_WITH_PRIVATE (GtefFileMetadata, gtef_file_metadata, G_TYPE_OBJECT)

#define METADATA_PREFIX "metadata::"
#define METADATA_QUERY_ATTRIBUTES "metadata::*"

static gchar *
get_metadata_attribute_key (const gchar *key)
{
	return g_strconcat (METADATA_PREFIX, key, NULL);
}

static void
print_fallback_to_metadata_manager_warning (void)
{
	static gboolean warning_printed = FALSE;

	if (G_LIKELY (warning_printed))
	{
		return;
	}

	g_warning ("GVfs metadata is not supported. Fallback to GtefMetadataManager. "
		   "Either GVfs is not correctly installed or GVfs metadata are "
		   "not supported on this platform. In the latter case, you should "
		   "configure Gtef with --disable-gvfs-metadata.");

	warning_printed = TRUE;
}

static void
set_file (GtefFileMetadata *metadata,
	  GtefFile         *file)
{
	GtefFileMetadataPrivate *priv = gtef_file_metadata_get_instance_private (metadata);

	g_return_if_fail (GTEF_IS_FILE (file));

	g_assert (priv->file == NULL);
	priv->file = file;

	g_object_add_weak_pointer (G_OBJECT (priv->file),
				   (gpointer *) &priv->file);

	g_object_notify_by_pspec (G_OBJECT (metadata), properties[PROP_FILE]);
}

static void
gtef_file_metadata_get_property (GObject    *object,
				 guint       prop_id,
				 GValue     *value,
				 GParamSpec *pspec)
{
	GtefFileMetadata *metadata = GTEF_FILE_METADATA (object);

	switch (prop_id)
	{
		case PROP_FILE:
			g_value_set_object (value, gtef_file_metadata_get_file (metadata));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gtef_file_metadata_set_property (GObject      *object,
				 guint         prop_id,
				 const GValue *value,
				 GParamSpec   *pspec)
{
	GtefFileMetadata *metadata = GTEF_FILE_METADATA (object);

	switch (prop_id)
	{
		case PROP_FILE:
			set_file (metadata, g_value_get_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gtef_file_metadata_dispose (GObject *object)
{
	GtefFileMetadataPrivate *priv;

	priv = gtef_file_metadata_get_instance_private (GTEF_FILE_METADATA (object));

	if (priv->file != NULL)
	{
		g_object_remove_weak_pointer (G_OBJECT (priv->file),
					      (gpointer *) &priv->file);

		priv->file = NULL;
	}

	G_OBJECT_CLASS (gtef_file_metadata_parent_class)->dispose (object);
}

static void
gtef_file_metadata_finalize (GObject *object)
{
	GtefFileMetadataPrivate *priv;

	priv = gtef_file_metadata_get_instance_private (GTEF_FILE_METADATA (object));

	g_object_unref (priv->file_info);

	G_OBJECT_CLASS (gtef_file_metadata_parent_class)->finalize (object);
}

static void
gtef_file_metadata_class_init (GtefFileMetadataClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gtef_file_metadata_get_property;
	object_class->set_property = gtef_file_metadata_set_property;
	object_class->dispose = gtef_file_metadata_dispose;
	object_class->finalize = gtef_file_metadata_finalize;

	/**
	 * GtefFileMetadata:file:
	 *
	 * The #GtefFile that the metadata belong to.
	 *
	 * Since: 1.0
	 */
	properties[PROP_FILE] =
		g_param_spec_object ("file",
				     "File",
				     "",
				     GTEF_TYPE_FILE,
				     G_PARAM_READWRITE |
				     G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
gtef_file_metadata_init (GtefFileMetadata *metadata)
{
	GtefFileMetadataPrivate *priv = gtef_file_metadata_get_instance_private (metadata);

	priv->file_info = g_file_info_new ();

#ifdef ENABLE_GVFS_METADATA
	priv->use_gvfs_metadata = TRUE;
#else
	priv->use_gvfs_metadata = FALSE;
#endif
}

/**
 * gtef_file_metadata_new:
 * @file: the #GtefFile that the metadata will belong to.
 *
 * Returns: a new #GtefFileMetadata object.
 * Since: 1.0
 */
GtefFileMetadata *
gtef_file_metadata_new (GtefFile *file)
{
	g_return_val_if_fail (GTEF_IS_FILE (file), NULL);

	return g_object_new (GTEF_TYPE_FILE_METADATA,
			     "file", file,
			     NULL);
}

/**
 * gtef_file_metadata_get_file:
 * @metadata: a #GtefFileMetadata object.
 *
 * Returns: (transfer none): the #GtefFile that the metadata belong to.
 * Since: 1.0
 */
GtefFile *
gtef_file_metadata_get_file (GtefFileMetadata *metadata)
{
	GtefFileMetadataPrivate *priv;

	g_return_val_if_fail (GTEF_IS_FILE_METADATA (metadata), NULL);

	priv = gtef_file_metadata_get_instance_private (metadata);
	return priv->file;
}

/**
 * gtef_file_metadata_get:
 * @metadata: a #GtefFileMetadata.
 * @key: the name of the metadata.
 *
 * Gets the value of a metadata stored in the @metadata object memory.
 *
 * Returns: (nullable): the value of the metadata, or %NULL if the metadata
 *   doesn't exist. Free with g_free().
 * Since: 1.0
 */
gchar *
gtef_file_metadata_get (GtefFileMetadata *metadata,
			const gchar      *key)
{
	GtefFileMetadataPrivate *priv;
	gchar *attribute_key;
	gchar *value = NULL;

	g_return_val_if_fail (GTEF_IS_FILE_METADATA (metadata), NULL);
	g_return_val_if_fail (key != NULL && key[0] != '\0', NULL);

	priv = gtef_file_metadata_get_instance_private (metadata);

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
 * gtef_file_metadata_set:
 * @metadata: a #GtefFileMetadata.
 * @key: the name of the metadata.
 * @value: (nullable): the value of the metadata, or %NULL to unset.
 *
 * Sets the value of a metadata. It's preferable that @key starts with a
 * namespace, to not get metadata conflicts between applications.
 *
 * This function just stores the new metadata value in the @metadata object
 * memory.
 *
 * Since: 1.0
 */
void
gtef_file_metadata_set (GtefFileMetadata *metadata,
			const gchar      *key,
			const gchar      *value)
{
	GtefFileMetadataPrivate *priv;
	gchar *attribute_key;

	g_return_if_fail (GTEF_IS_FILE_METADATA (metadata));
	g_return_if_fail (key != NULL && key[0] != '\0');

	priv = gtef_file_metadata_get_instance_private (metadata);

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

/**
 * gtef_file_metadata_load:
 * @metadata: a #GtefFileMetadata.
 * @cancellable: (nullable): optional #GCancellable object, %NULL to ignore.
 * @error: location to a %NULL #GError, or %NULL.
 *
 * Loads synchronously the metadata from #GtkSourceFile:location. The loaded
 * metadata values can then be accessed with gtef_file_metadata_get().
 *
 * If the metadata are loaded successfully, this function deletes all previous
 * metadata stored in the @metadata object memory.
 *
 * The file at #GtkSourceFile:location, if non-%NULL, must exist on the
 * filesystem, otherwise an error is returned.
 *
 * If #GtkSourceFile:location is %NULL, %FALSE is simply returned.
 *
 * Returns: whether the metadata was loaded successfully.
 * Since: 1.0
 */
gboolean
gtef_file_metadata_load (GtefFileMetadata  *metadata,
			 GCancellable      *cancellable,
			 GError           **error)
{
	GtefFileMetadataPrivate *priv;
	GFile *location;
	GFileInfo *file_info;

	g_return_val_if_fail (GTEF_IS_FILE_METADATA (metadata), FALSE);
	g_return_val_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	priv = gtef_file_metadata_get_instance_private (metadata);

	if (priv->file == NULL)
	{
		return FALSE;
	}

	location = gtk_source_file_get_location (GTK_SOURCE_FILE (priv->file));
	if (location == NULL)
	{
		return FALSE;
	}

	if (priv->use_gvfs_metadata)
	{
		GError *my_error = NULL;

		file_info = g_file_query_info (location,
					       METADATA_QUERY_ATTRIBUTES,
					       G_FILE_QUERY_INFO_NONE,
					       cancellable,
					       &my_error);

		if (g_error_matches (my_error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED))
		{
			print_fallback_to_metadata_manager_warning ();
			priv->use_gvfs_metadata = FALSE;

			g_clear_error (&my_error);
			g_clear_object (&file_info);
		}
		else if (my_error != NULL)
		{
			g_propagate_error (error, my_error);
			my_error = NULL;
		}
	}

	if (!priv->use_gvfs_metadata)
	{
		file_info = _gtef_metadata_manager_get_all_metadata_for_location (location);
	}

	if (file_info == NULL)
	{
		return FALSE;
	}

	g_object_unref (priv->file_info);
	priv->file_info = file_info;

	return TRUE;
}

static void
load_metadata_async_cb (GObject      *source_object,
			GAsyncResult *result,
			gpointer      user_data)
{
	GFile *location = G_FILE (source_object);
	GTask *task = G_TASK (user_data);
	GtefFileMetadata *metadata;
	GtefFileMetadataPrivate *priv;
	GFileInfo *file_info;
	GError *error = NULL;

	metadata = g_task_get_source_object (task);
	priv = gtef_file_metadata_get_instance_private (metadata);

	file_info = g_file_query_info_finish (location, result, &error);

	if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED))
	{
		print_fallback_to_metadata_manager_warning ();
		priv->use_gvfs_metadata = FALSE;

		g_clear_error (&error);
		g_clear_object (&metadata);

		file_info = _gtef_metadata_manager_get_all_metadata_for_location (location);
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
		g_task_return_boolean (task, FALSE);
		g_object_unref (task);
		return;
	}

	g_object_unref (priv->file_info);
	priv->file_info = file_info;

	g_task_return_boolean (task, TRUE);
	g_object_unref (task);
}

/**
 * gtef_file_metadata_load_async:
 * @metadata: a #GtefFileMetadata.
 * @io_priority: the I/O priority of the request. E.g. %G_PRIORITY_LOW,
 *   %G_PRIORITY_DEFAULT or %G_PRIORITY_HIGH.
 * @cancellable: (nullable): optional #GCancellable object, %NULL to ignore.
 * @callback: (scope async): a #GAsyncReadyCallback to call when the request is
 *   satisfied.
 * @user_data: user data to pass to @callback.
 *
 * The asynchronous version of gtef_file_metadata_load().
 *
 * If the metadata is loaded from the metadata manager (i.e. not with GVfs),
 * this function loads the metadata synchronously. A future version might fix
 * this.
 *
 * See the #GAsyncResult documentation to know how to use this function.
 *
 * Since: 1.0
 */
void
gtef_file_metadata_load_async (GtefFileMetadata    *metadata,
			       gint                 io_priority,
			       GCancellable        *cancellable,
			       GAsyncReadyCallback  callback,
			       gpointer             user_data)
{
	GtefFileMetadataPrivate *priv;
	GTask *task;
	GFile *location;

	g_return_if_fail (GTEF_IS_FILE_METADATA (metadata));
	g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));

	priv = gtef_file_metadata_get_instance_private (metadata);

	task = g_task_new (metadata, cancellable, callback, user_data);

	if (priv->file == NULL)
	{
		g_task_return_boolean (task, FALSE);
		g_object_unref (task);
		return;
	}

	location = gtk_source_file_get_location (GTK_SOURCE_FILE (priv->file));
	if (location == NULL)
	{
		g_task_return_boolean (task, FALSE);
		g_object_unref (task);
		return;
	}

	if (priv->use_gvfs_metadata)
	{
		g_file_query_info_async (location,
					 METADATA_QUERY_ATTRIBUTES,
					 G_FILE_QUERY_INFO_NONE,
					 io_priority,
					 cancellable,
					 load_metadata_async_cb,
					 task);
	}
	else
	{
		gboolean ok;

		ok = gtef_file_metadata_load (metadata, cancellable, NULL);
		g_task_return_boolean (task, ok);
		g_object_unref (task);
	}
}

/**
 * gtef_file_metadata_load_finish:
 * @metadata: a #GtefFileMetadata.
 * @result: a #GAsyncResult.
 * @error: location to a %NULL #GError, or %NULL.
 *
 * Finishes the metadata loading started with gtef_file_metadata_load_async().
 *
 * Returns: whether the metadata was loaded successfully.
 * Since: 1.0
 */
gboolean
gtef_file_metadata_load_finish (GtefFileMetadata  *metadata,
				GAsyncResult      *result,
				GError           **error)
{
	g_return_val_if_fail (GTEF_IS_FILE_METADATA (metadata), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail (g_task_is_valid (result, metadata), FALSE);

	return g_task_propagate_boolean (G_TASK (result), error);
}

/**
 * gtef_file_metadata_save:
 * @metadata: a #GtefFileMetadata.
 * @cancellable: (nullable): optional #GCancellable object, %NULL to ignore.
 * @error: location to a %NULL #GError, or %NULL.
 *
 * Saves synchronously the metadata for #GtkSourceFile:location.
 *
 * The file at #GtkSourceFile:location, if non-%NULL, must exist on the
 * filesystem, otherwise an error is returned.
 *
 * If #GtkSourceFile:location is %NULL, %FALSE is simply returned.
 *
 * Returns: whether the metadata was saved successfully.
 * Since: 1.0
 */
gboolean
gtef_file_metadata_save (GtefFileMetadata  *metadata,
			 GCancellable      *cancellable,
			 GError           **error)
{
	GtefFileMetadataPrivate *priv;
	GFile *location;

	g_return_val_if_fail (GTEF_IS_FILE_METADATA (metadata), FALSE);
	g_return_val_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	priv = gtef_file_metadata_get_instance_private (metadata);

	if (priv->file == NULL)
	{
		return FALSE;
	}

	location = gtk_source_file_get_location (GTK_SOURCE_FILE (priv->file));
	if (location == NULL)
	{
		return FALSE;
	}

	if (priv->use_gvfs_metadata)
	{
		GError *my_error = NULL;
		gboolean ok;

		ok = g_file_set_attributes_from_info (location,
						      priv->file_info,
						      G_FILE_QUERY_INFO_NONE,
						      cancellable,
						      &my_error);

		if (g_error_matches (my_error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED))
		{
			print_fallback_to_metadata_manager_warning ();
			priv->use_gvfs_metadata = FALSE;

			g_clear_error (&my_error);
		}
		else if (my_error != NULL)
		{
			g_propagate_error (error, my_error);
			return ok;
		}
		else
		{
			return ok;
		}
	}

	g_assert (!priv->use_gvfs_metadata);

	_gtef_metadata_manager_set_metadata_for_location (location, priv->file_info);

	return TRUE;
}

static void
save_metadata_async_cb (GObject      *source_object,
			GAsyncResult *result,
			gpointer      user_data)
{
	GFile *location = G_FILE (source_object);
	GTask *task = G_TASK (user_data);
	GtefFileMetadata *metadata;
	GtefFileMetadataPrivate *priv;
	GError *error = NULL;

	metadata = g_task_get_source_object (task);
	priv = gtef_file_metadata_get_instance_private (metadata);

	g_file_set_attributes_finish (location, result, NULL, &error);

	if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED))
	{
		print_fallback_to_metadata_manager_warning ();
		priv->use_gvfs_metadata = FALSE;

		g_clear_error (&error);

		_gtef_metadata_manager_set_metadata_for_location (location, priv->file_info);
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
 * gtef_file_metadata_save_async:
 * @metadata: a #GtefFileMetadata.
 * @io_priority: the I/O priority of the request. E.g. %G_PRIORITY_LOW,
 *   %G_PRIORITY_DEFAULT or %G_PRIORITY_HIGH.
 * @cancellable: (nullable): optional #GCancellable object, %NULL to ignore.
 * @callback: (scope async): a #GAsyncReadyCallback to call when the request is
 *   satisfied.
 * @user_data: user data to pass to @callback.
 *
 * The asynchronous version of gtef_file_metadata_save().
 *
 * If the metadata is saved with the metadata manager (i.e. not with GVfs), this
 * function saves the metadata synchronously. A future version might fix this.
 *
 * See the #GAsyncResult documentation to know how to use this function.
 *
 * Since: 1.0
 */
void
gtef_file_metadata_save_async (GtefFileMetadata    *metadata,
			       gint                 io_priority,
			       GCancellable        *cancellable,
			       GAsyncReadyCallback  callback,
			       gpointer             user_data)
{
	GtefFileMetadataPrivate *priv;
	GTask *task;
	GFile *location;

	g_return_if_fail (GTEF_IS_FILE_METADATA (metadata));
	g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));

	priv = gtef_file_metadata_get_instance_private (metadata);

	task = g_task_new (metadata, cancellable, callback, user_data);

	if (priv->file == NULL)
	{
		g_task_return_boolean (task, FALSE);
		g_object_unref (task);
		return;
	}

	location = gtk_source_file_get_location (GTK_SOURCE_FILE (priv->file));
	if (location == NULL)
	{
		g_task_return_boolean (task, FALSE);
		g_object_unref (task);
		return;
	}

	if (priv->use_gvfs_metadata)
	{
		g_file_set_attributes_async (location,
					     priv->file_info,
					     G_FILE_QUERY_INFO_NONE,
					     io_priority,
					     cancellable,
					     save_metadata_async_cb,
					     task);
	}
	else
	{
		_gtef_metadata_manager_set_metadata_for_location (location, priv->file_info);
		g_task_return_boolean (task, TRUE);
		g_object_unref (task);
	}
}

/**
 * gtef_file_metadata_save_finish:
 * @metadata: a #GtefFileMetadata.
 * @result: a #GAsyncResult.
 * @error: location to a %NULL #GError, or %NULL.
 *
 * Finishes the metadata saving started with gtef_file_metadata_save_async().
 *
 * Returns: whether the metadata was saved successfully.
 * Since: 1.0
 */
gboolean
gtef_file_metadata_save_finish (GtefFileMetadata  *metadata,
				GAsyncResult      *result,
				GError           **error)
{
	g_return_val_if_fail (GTEF_IS_FILE_METADATA (metadata), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail (g_task_is_valid (result, metadata), FALSE);

	return g_task_propagate_boolean (G_TASK (result), error);
}

/* For unit tests. */
void
_gtef_file_metadata_set_use_gvfs_metadata (GtefFileMetadata *metadata,
					   gboolean          use_gvfs_metadata)
{
	GtefFileMetadataPrivate *priv;

	g_return_if_fail (GTEF_IS_FILE_METADATA (metadata));

	priv = gtef_file_metadata_get_instance_private (metadata);

	priv->use_gvfs_metadata = use_gvfs_metadata != FALSE;
}
