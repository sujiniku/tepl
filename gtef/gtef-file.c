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

#include "config.h"
#include "gtef-file.h"
#include <glib/gi18n-lib.h>
#include "gtef-metadata-manager.h"

/**
 * SECTION:file
 * @Short_description: On-disk representation of a GtefBuffer
 * @Title: GtefFile
 *
 * #GtefFile extends #GtkSourceFile with metadata support, among other smaller
 * things. You need to call gtef_metadata_manager_init() and
 * gtef_metadata_manager_shutdown() in your application, in case GVfs metadata
 * are not supported.
 *
 * gtef_file_get_metadata() and gtef_file_set_metadata() don't load or save the
 * metadata on disk. They only access the metadata stored in the #GtefFile
 * object memory. To load the metadata from disk, call gtef_file_load_metadata()
 * or its async variant. Likewise, to save the metadata on disk, call
 * gtef_file_save_metadata() or its async variant. When loading or saving
 * metadata, the file at #GtkSourceFile:location, if non-%NULL, must exist on
 * the filesystem, otherwise an error is returned.
 *
 * When the #GtkSourceFile:location changes, the metadata are still kept in the
 * #GtefFile object memory. But the metadata are <emphasis>not</emphasis>
 * automatically saved for the new location.
 */

/* TODO Better test how it works with remote files, with various protocols.
 * For example with an ftp://... location, there can be the error "The specified
 * location is not mounted". In that case we can either propagate the error or
 * automatically call the GtkSourceFile mount operation factory method.
 *
 * On Linux, is the metadata supported for all GVfs backends? (the custom
 * metadata that we set). Can it fallback to the metadata manager even on Linux?
 */

typedef struct _GtefFilePrivate GtefFilePrivate;

struct _GtefFilePrivate
{
	/* Never NULL */
	GFileInfo *metadata;

	gchar *short_name;

	guint use_gvfs_metadata : 1;
};

enum
{
	PROP_0,
	PROP_SHORT_NAME,
	N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

G_DEFINE_TYPE_WITH_PRIVATE (GtefFile, gtef_file, GTK_SOURCE_TYPE_FILE)

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
gtef_file_get_property (GObject    *object,
			guint       prop_id,
			GValue     *value,
			GParamSpec *pspec)
{
	GtefFile *file = GTEF_FILE (object);

	switch (prop_id)
	{
		case PROP_SHORT_NAME:
			g_value_set_string (value, gtef_file_get_short_name (file));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gtef_file_set_property (GObject      *object,
			guint         prop_id,
			const GValue *value,
			GParamSpec   *pspec)
{
	switch (prop_id)
	{
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gtef_file_finalize (GObject *object)
{
	GtefFilePrivate *priv = gtef_file_get_instance_private (GTEF_FILE (object));

	g_object_unref (priv->metadata);
	g_free (priv->short_name);

	G_OBJECT_CLASS (gtef_file_parent_class)->finalize (object);
}

static void
gtef_file_class_init (GtefFileClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gtef_file_get_property;
	object_class->set_property = gtef_file_set_property;
	object_class->finalize = gtef_file_finalize;

	/**
	 * GtefFile:short-name:
	 *
	 * The file short name. See gtef_file_get_short_name().
	 *
	 * Since: 1.0
	 */
	properties[PROP_SHORT_NAME] =
		g_param_spec_string ("short-name",
				     "Short Name",
				     "",
				     NULL,
				     G_PARAM_READABLE |
				     G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
query_display_name_cb (GObject      *source_object,
		       GAsyncResult *result,
		       gpointer      user_data)
{
	GFile *location = G_FILE (source_object);
	GtefFile *file = GTEF_FILE (user_data);
	GtefFilePrivate *priv;
	GFileInfo *info;
	GError *error = NULL;

	priv = gtef_file_get_instance_private (file);

	info = g_file_query_info_finish (location, result, &error);

	if (error != NULL)
	{
		/* TODO short-name fallback when the file doesn't exist. */
		if (!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND))
		{
			g_warning ("Error when querying file information: %s", error->message);
		}

		g_clear_error (&error);
		goto out;
	}

	g_free (priv->short_name);
	priv->short_name = g_strdup (g_file_info_get_display_name (info));

	g_object_notify_by_pspec (G_OBJECT (file), properties[PROP_SHORT_NAME]);

out:
	g_clear_object (&info);

	/* Async operation finished */
	g_object_unref (file);
}

static void
update_short_name (GtefFile *file)
{
	GtefFilePrivate *priv;
	GFile *location;

	priv = gtef_file_get_instance_private (file);

	location = gtk_source_file_get_location (GTK_SOURCE_FILE (file));

	if (location == NULL)
	{
		g_free (priv->short_name);
		priv->short_name = g_strdup (_("Untitled File"));

		g_object_notify_by_pspec (G_OBJECT (file), properties[PROP_SHORT_NAME]);
	}
	else
	{
		g_file_query_info_async (location,
					 G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
					 G_FILE_QUERY_INFO_NONE,
					 G_PRIORITY_DEFAULT,
					 NULL,
					 query_display_name_cb,
					 g_object_ref (file));
	}
}

static void
location_notify_cb (GtefFile   *file,
		    GParamSpec *pspec,
		    gpointer    user_data)
{
	update_short_name (file);
}

static void
gtef_file_init (GtefFile *file)
{
	GtefFilePrivate *priv = gtef_file_get_instance_private (file);

	priv->metadata = g_file_info_new ();

#ifdef ENABLE_GVFS_METADATA
	priv->use_gvfs_metadata = TRUE;
#else
	priv->use_gvfs_metadata = FALSE;
#endif

	update_short_name (file);
	g_signal_connect (file,
			  "notify::location",
			  G_CALLBACK (location_notify_cb),
			  NULL);
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
 * gtef_file_get_short_name:
 * @file: a #GtefFile.
 *
 * Gets the @file short name. If the #GtkSourceFile:location isn't %NULL,
 * returns its display-name (see #G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME).
 * Otherwise returns "Untitled File".
 *
 * Returns: the @file short name.
 * Since: 1.0
 */
const gchar *
gtef_file_get_short_name (GtefFile *file)
{
	GtefFilePrivate *priv;

	g_return_val_if_fail (GTEF_IS_FILE (file), NULL);

	priv = gtef_file_get_instance_private (file);
	return priv->short_name;
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
 * Loads synchronously the metadata from #GtkSourceFile:location. The loaded
 * metadata values can then be accessed with gtef_file_get_metadata().
 *
 * If the metadata are loaded successfully, this function deletes all previous
 * metadata stored in the @file object memory.
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

	if (priv->use_gvfs_metadata)
	{
		GError *my_error = NULL;

		metadata = g_file_query_info (location,
					      METADATA_QUERY_ATTRIBUTES,
					      G_FILE_QUERY_INFO_NONE,
					      cancellable,
					      &my_error);

		if (g_error_matches (my_error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED))
		{
			print_fallback_to_metadata_manager_warning ();
			priv->use_gvfs_metadata = FALSE;

			g_clear_error (&my_error);
			g_clear_object (&metadata);
		}
		else if (my_error != NULL)
		{
			g_propagate_error (error, my_error);
			my_error = NULL;
		}
	}

	if (!priv->use_gvfs_metadata)
	{
		metadata = _gtef_metadata_manager_get_all_metadata_for_location (location);
	}

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

	file = g_task_get_source_object (task);
	priv = gtef_file_get_instance_private (file);

	metadata = g_file_query_info_finish (location, result, &error);

	if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED))
	{
		print_fallback_to_metadata_manager_warning ();
		priv->use_gvfs_metadata = FALSE;

		g_clear_error (&error);
		g_clear_object (&metadata);

		metadata = _gtef_metadata_manager_get_all_metadata_for_location (location);
	}

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
 * If the metadata is loaded from the metadata manager (i.e. not with GVfs),
 * this function loads the metadata synchronously. A future version might fix
 * this.
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
	GtefFilePrivate *priv;
	GTask *task;
	GFile *location;

	g_return_if_fail (GTEF_IS_FILE (file));
	g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));

	priv = gtef_file_get_instance_private (file);

	task = g_task_new (file, cancellable, callback, user_data);

	location = gtk_source_file_get_location (GTK_SOURCE_FILE (file));
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

		ok = gtef_file_load_metadata (file, cancellable, NULL);
		g_task_return_boolean (task, ok);
		g_object_unref (task);
	}
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

/**
 * gtef_file_save_metadata:
 * @file: a #GtefFile.
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
gtef_file_save_metadata (GtefFile      *file,
			 GCancellable  *cancellable,
			 GError       **error)
{
	GtefFilePrivate *priv;
	GFile *location;

	g_return_val_if_fail (GTEF_IS_FILE (file), FALSE);
	g_return_val_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	priv = gtef_file_get_instance_private (file);

	location = gtk_source_file_get_location (GTK_SOURCE_FILE (file));
	if (location == NULL)
	{
		return FALSE;
	}

	if (priv->use_gvfs_metadata)
	{
		GError *my_error = NULL;
		gboolean ok;

		ok = g_file_set_attributes_from_info (location,
						      priv->metadata,
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

	_gtef_metadata_manager_set_metadata_for_location (location, priv->metadata);

	return TRUE;
}

static void
save_metadata_async_cb (GObject      *source_object,
			GAsyncResult *result,
			gpointer      user_data)
{
	GFile *location = G_FILE (source_object);
	GTask *task = G_TASK (user_data);
	GtefFile *file;
	GtefFilePrivate *priv;
	GError *error = NULL;

	file = g_task_get_source_object (task);
	priv = gtef_file_get_instance_private (file);

	g_file_set_attributes_finish (location, result, NULL, &error);

	if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED))
	{
		print_fallback_to_metadata_manager_warning ();
		priv->use_gvfs_metadata = FALSE;

		g_clear_error (&error);

		_gtef_metadata_manager_set_metadata_for_location (location, priv->metadata);
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
 * gtef_file_save_metadata_async:
 * @file: a #GtefFile.
 * @io_priority: the I/O priority of the request. E.g. %G_PRIORITY_LOW,
 *   %G_PRIORITY_DEFAULT or %G_PRIORITY_HIGH.
 * @cancellable: (nullable): optional #GCancellable object, %NULL to ignore.
 * @callback: (scope async): a #GAsyncReadyCallback to call when the request is
 *   satisfied.
 * @user_data: user data to pass to @callback.
 *
 * The asynchronous version of gtef_file_save_metadata().
 *
 * If the metadata is saved with the metadata manager (i.e. not with GVfs), this
 * function saves the metadata synchronously. A future version might fix this.
 *
 * See the #GAsyncResult documentation to know how to use this function.
 *
 * Since: 1.0
 */
void
gtef_file_save_metadata_async (GtefFile            *file,
			       gint                 io_priority,
			       GCancellable        *cancellable,
			       GAsyncReadyCallback  callback,
			       gpointer             user_data)
{
	GtefFilePrivate *priv;
	GTask *task;
	GFile *location;

	g_return_if_fail (GTEF_IS_FILE (file));
	g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));

	priv = gtef_file_get_instance_private (file);

	task = g_task_new (file, cancellable, callback, user_data);

	location = gtk_source_file_get_location (GTK_SOURCE_FILE (file));
	if (location == NULL)
	{
		g_task_return_boolean (task, FALSE);
		g_object_unref (task);
		return;
	}

	if (priv->use_gvfs_metadata)
	{
		g_file_set_attributes_async (location,
					     priv->metadata,
					     G_FILE_QUERY_INFO_NONE,
					     io_priority,
					     cancellable,
					     save_metadata_async_cb,
					     task);
	}
	else
	{
		_gtef_metadata_manager_set_metadata_for_location (location, priv->metadata);
		g_task_return_boolean (task, TRUE);
		g_object_unref (task);
	}
}

/**
 * gtef_file_save_metadata_finish:
 * @file: a #GtefFile.
 * @result: a #GAsyncResult.
 * @error: location to a %NULL #GError, or %NULL.
 *
 * Finishes the metadata saving started with gtef_file_save_metadata_async().
 *
 * Returns: whether the metadata was saved successfully.
 * Since: 1.0
 */
gboolean
gtef_file_save_metadata_finish (GtefFile      *file,
				GAsyncResult  *result,
				GError       **error)
{
	g_return_val_if_fail (GTEF_IS_FILE (file), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail (g_task_is_valid (result, file), FALSE);

	return g_task_propagate_boolean (G_TASK (result), error);
}

/* For unit tests. */
void
_gtef_file_set_use_gvfs_metadata (GtefFile *file,
				  gboolean  use_gvfs_metadata)
{
	GtefFilePrivate *priv;

	g_return_if_fail (GTEF_IS_FILE (file));

	priv = gtef_file_get_instance_private (file);

	priv->use_gvfs_metadata = use_gvfs_metadata != FALSE;
}
