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
#include "tepl-metadata.h"
#include "tepl-utils.h"

/**
 * SECTION:file-metadata
 * @Short_description: File metadata
 * @Title: TeplFileMetadata
 *
 * A #TeplFileMetadata object stores the metadata of a #GFile, as key/value
 * pairs. Key names must be valid according to
 * tepl_utils_metadata_key_is_valid(); see also tepl_file_metadata_set(). Values
 * must be nul-terminated UTF-8 strings.
 *
 * You need to load and save the #TeplMetadataStore in your application, it is
 * used as a fallback in #TeplFileMetadata in the case where GVfs metadata is
 * not supported. TODO: update.
 *
 * The tepl_file_metadata_get() and tepl_file_metadata_set() functions don't
 * load or save the metadata on disk, they only access the metadata stored in
 * the #TeplFileMetadata object memory. To load the metadata from disk or from
 * the #TeplMetadataStore, call tepl_file_metadata_load_async(). Likewise, to
 * save the metadata on disk or to the #TeplMetadataStore, call
 * tepl_file_metadata_save_async().
 *
 * # Separation of concerns
 *
 * To implement file metadata in an application, one strategy is to separate
 * concerns:
 * - Individual features or plugins call tepl_file_metadata_get() and
 *   tepl_file_metadata_set().
 * - The code that manages file loading and saving takes care of calling
 *   tepl_file_metadata_load_async() and tepl_file_metadata_save_async() at
 *   appropriate times (see the following use-cases as inspiration), and
 *   orchestrates or signals individual features or plugins to
 *   activate/deactivate themselves also at appropriate times.
 *
 * # Application use-cases
 *
 * This section documents some use-cases that applications may want to support.
 * Pointers are given to know how to integrate #TeplFileMetadata in your
 * application to support the use-cases.
 *
 * ## Storing settings in memory for unsaved documents
 *
 * User story:
 * 1. I create a new document in gedit. Note that the document is still unsaved,
 *    so the #GFile is %NULL at this point in time.
 * 2. I enable the spell-checking plugin and I configure the spell-checking
 *    language for the new document.
 * 3. Then I <emphasis>disable the spell-checking plugin</emphasis>. The feature
 *    is completely disabled from the application.
 * 4. Then I re-enable the spell-checking plugin.
 * 5. ==> I want that the spell-checking language setting is restored for the new
 *    document (the new document is still unsaved).
 *
 * #TeplFileMetadata supports well this user story: a #TeplFileMetadata object
 * can be associated with the #GtkTextBuffer of the new document, then the
 * spell-checking feature can set and retrieve the settings with
 * tepl_file_metadata_set() and tepl_file_metadata_get().
 *
 * ## Saving a new document and re-opening the file
 *
 * User story:
 * 1. I create a new document in gedit. Note that the document is still unsaved,
 *    so the #GFile is %NULL at this point in time.
 * 2. I configure the spell-checking language for the new document.
 * 3. I disable the spell-checking plugin.
 * 4. I save the document to a new file (the #GFile location never existed
 *    before with my user account).
 * 5. Then I close the document.
 * 6. (Optional) I close the gedit application and re-launch it.
 * 7. Then I re-open the document.
 * 8. I re-enable the spell-checking plugin.
 * 9. ==> I want that the spell-checking language setting is restored for the
 *    document.
 *
 * At step 2, tepl_file_metadata_set() is called. Just after step 4,
 * tepl_file_metadata_save_async() is called (it must be done after saving the
 * document's content, because the #GFile needs to exist when saving metadata
 * with GVfs). During step 7, tepl_file_metadata_load_async() is called. At step
 * 8, tepl_file_metadata_get() is called by the spell-checking plugin.
 *
 * ## File revert/reload
 *
 * User story:
 * 1. I open a file in a text editor. The metadata for that file is loaded.
 * 2. I re-configure the character encoding and line ending type.
 * 3. Then I change my mind, I made a mistake and I want to revert the changes.
 *    So I click on the button to revert/reload the file.
 * 4. ==> I want the old configuration back.
 *
 * At step 3, tepl_file_metadata_load_async() needs to be called, which deletes
 * all previous metadata stored in the #TeplFileMetadata object memory. So at
 * step 4 the old configuration is correctly restored.
 *
 * ## Save as
 *
 * For the 'Save as' feature of a text editor, to save a document to a new
 * #GFile location, there are two cases:
 * 1. The new #GFile location didn't exist, a new file is created.
 * 2. The new #GFile location already existed and is replaced.
 *
 * But from the point of view of metadata handling, the two cases can be
 * simplified to only case 2, because for case 1 the new #GFile location may
 * have existed in the past and the metadata for it was not deleted.
 *
 * What we want in all cases is to first delete any metadata for the new #GFile
 * location, then save <emphasis>all</emphasis> the metadata belonging to our
 * document that we are saving.
 *
 * This is supported by tepl_file_metadata_save_async() with the @save_as
 * parameter.
 *
 * ## Opening a second time the same file in the same application
 *
 * User story:
 * 1. I open a file in gedit.
 * 2. I configure the spell-checking language and character encoding.
 * 3. I open a new gedit window and I open the same file again in it.
 * 4. ==> I want the same spell-checking settings and character encoding.
 * 5. Then I change the spell-checking language in the first opened document.
 * 6. ==> I <emphasis>don't</emphasis> want the setting to be automatically
 *    synchronized in the other document. Because I may want to spell-check
 *    certain paragraphs in another language.
 *
 * To support this - and assuming that each document has a separate
 * #GtkTextBuffer/#TeplFileMetadata pair - step 3 should proceed as follows:
 * first save the metadata of the first document with
 * tepl_file_metadata_save_async() and wait that the operation is finished, then
 * load the metadata for the second document (which has the same #GFile
 * location).
 *
 * For step 6, the two #TeplFileMetadata objects are not synchronized, the
 * metadata are just saved when the respective document is saved.
 *
 * ## Opening the same file in another application - Shared metadata
 *
 * User story:
 * 1. I open a file in Text Editor A.
 * 2. I configure the character encoding and spell-checking language.
 * 3. I open the same file in Text Editor B.
 * 4. ==> When opening the file in Text Editor B, I want the same character
 *    encoding as configured in step 2 from Text Editor A. The spell-checking is
 *    specific to Text Editor A and the language should be saved only when
 *    saving the document (see also the previous, related user story).
 *
 * So there is a desire to save only a subset of a #TeplFileMetadata. Namely at
 * step 2, the character encoding - once correctly configured to view the file -
 * should be saved <emphasis>directly</emphasis>, while for other kind of
 * metadata it is better to save them when the document is saved.
 *
 * FIXME: currently not well supported by #TeplFileMetadata, need to add
 * `save_subset_async/finish()` taking an array or list of keys to save.
 */

/* API design - additional notes:
 *
 * The values must be valid UTF-8 strings, not arbitrary byte string because
 * G_FILE_ATTRIBUTE_TYPE_STRING is used. And also because it's convenient to
 * have UTF-8 strings, in case they are displayed in the UI with GTK. Note that
 * there is also G_FILE_ATTRIBUTE_TYPE_BYTE_STRING.
 */

typedef struct _TeplFileMetadataPrivate TeplFileMetadataPrivate;

struct _TeplFileMetadataPrivate
{
	/* Never NULL. Contains all metadata that was loaded with
	 * tepl_file_metadata_load_async() or set with tepl_file_metadata_set().
	 */
	GFileInfo *file_info_all;

	/* Can be NULL. Contains the metadata that has been modified by calling
	 * tepl_file_metadata_set(), but which has not yet been saved.
	 */
	GFileInfo *file_info_modified;

	guint is_saving : 1;
};

G_DEFINE_TYPE_WITH_PRIVATE (TeplFileMetadata, tepl_file_metadata, G_TYPE_OBJECT)

static gchar *
get_metadata_attribute_key (const gchar *key)
{
	return g_strconcat ("metadata::", key, NULL);
}

static void
print_gvfs_metadata_not_supported_warning (void)
{
	g_warning_once ("GVfs metadata is not supported. Either GVfs is not correctly "
			"installed or GVfs metadata is not supported on this platform. "
			"In the latter case, you should configure the Tepl library with "
			"-Dgvfs_metadata=false.");
}

static void
tepl_file_metadata_finalize (GObject *object)
{
	TeplFileMetadataPrivate *priv;

	priv = tepl_file_metadata_get_instance_private (TEPL_FILE_METADATA (object));

	g_object_unref (priv->file_info_all);
	g_clear_object (&priv->file_info_modified);

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

	priv->file_info_all = g_file_info_new ();
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
 * @key must meet the requirements of tepl_utils_metadata_key_is_valid().
 *
 * Returns: (nullable): the value of the metadata as a UTF-8 string, or %NULL if
 *   the metadata doesn't exist. Free with g_free().
 * Since: 1.0
 */
gchar *
tepl_file_metadata_get (TeplFileMetadata *metadata,
			const gchar      *key)
{
	TeplFileMetadataPrivate *priv;
	gchar *attribute_key;
	const gchar *value = NULL;

	g_return_val_if_fail (TEPL_IS_FILE_METADATA (metadata), NULL);
	g_return_val_if_fail (tepl_utils_metadata_key_is_valid (key), NULL);

	priv = tepl_file_metadata_get_instance_private (metadata);

	attribute_key = get_metadata_attribute_key (key);

	if (g_file_info_has_attribute (priv->file_info_all, attribute_key) &&
	    g_file_info_get_attribute_type (priv->file_info_all, attribute_key) == G_FILE_ATTRIBUTE_TYPE_STRING)
	{
		value = g_file_info_get_attribute_string (priv->file_info_all, attribute_key);
	}

	g_free (attribute_key);

	if (value != NULL && !g_utf8_validate (value, -1, NULL))
	{
		value = NULL;
		g_warn_if_reached ();
	}

	return g_strdup (value);
}

/**
 * tepl_file_metadata_set:
 * @metadata: a #TeplFileMetadata.
 * @key: the name of the metadata.
 * @value: (nullable): the value of the metadata as a UTF-8 string, or %NULL to
 *   unset.
 *
 * Sets the value of a metadata. This function just stores the new metadata
 * value in the @metadata object memory.
 *
 * @key must meet the requirements of tepl_utils_metadata_key_is_valid().
 * Additionally, it's preferable that @key starts with a namespace, to not get
 * metadata conflicts between applications. For example a good @key name for the
 * gedit application is `"gedit-spell-checking-language"`.
 *
 * @value, if non-%NULL, must be a valid nul-terminated UTF-8 string.
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
	g_return_if_fail (tepl_utils_metadata_key_is_valid (key));
	g_return_if_fail (value == NULL || g_utf8_validate (value, -1, NULL));

	priv = tepl_file_metadata_get_instance_private (metadata);
	g_return_if_fail (!priv->is_saving);

	if (priv->file_info_modified == NULL)
	{
		priv->file_info_modified = g_file_info_new ();
	}

	attribute_key = get_metadata_attribute_key (key);

	if (value != NULL)
	{
		g_file_info_set_attribute_string (priv->file_info_all,
						  attribute_key,
						  value);

		g_file_info_set_attribute_string (priv->file_info_modified,
						  attribute_key,
						  value);
	}
	else
	{
		g_file_info_remove_attribute (priv->file_info_all, attribute_key);

		/* Unset the key. If we call g_file_info_remove_attribute() on
		 * priv->file_info_modified, then when calling
		 * tepl_file_metadata_save_async(save_as=false), the metadata
		 * attribute will not get removed, it would just be ignored
		 * (since it would not be there in the GFileInfo anymore).
		 */
		g_file_info_set_attribute (priv->file_info_modified,
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
	TeplFileMetadata *metadata = g_task_get_source_object (task);
	TeplFileMetadataPrivate *priv = tepl_file_metadata_get_instance_private (metadata);
	GFileInfo *loaded_file_info;
	GError *error = NULL;

	loaded_file_info = _tepl_metadata_query_info_finish (location, result, &error);

	if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED))
	{
		print_gvfs_metadata_not_supported_warning ();
	}

	if (error != NULL)
	{
		g_task_return_error (task, error);
		g_object_unref (task);
		g_clear_object (&loaded_file_info);
		return;
	}

	if (loaded_file_info != NULL)
	{
		g_object_unref (priv->file_info_all);
		priv->file_info_all = loaded_file_info;

		g_clear_object (&priv->file_info_modified);
	}

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
	TeplFileMetadataPrivate *priv;
	GTask *task;

	g_return_if_fail (TEPL_IS_FILE_METADATA (metadata));
	g_return_if_fail (G_IS_FILE (location));
	g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));

	priv = tepl_file_metadata_get_instance_private (metadata);
	g_return_if_fail (!priv->is_saving);

	task = g_task_new (metadata, cancellable, callback, user_data);
	g_task_set_priority (task, io_priority);

	_tepl_metadata_query_info_async (location,
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
set_attributes_cb (GObject      *source_object,
		   GAsyncResult *result,
		   gpointer      user_data)
{
	GFile *location = G_FILE (source_object);
	GTask *task = G_TASK (user_data);
	TeplFileMetadata *metadata = g_task_get_source_object (task);
	TeplFileMetadataPrivate *priv = tepl_file_metadata_get_instance_private (metadata);
	GError *error = NULL;

	_tepl_metadata_set_attributes_finish (location, result, &error);

	if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED))
	{
		print_gvfs_metadata_not_supported_warning ();
	}

	if (error != NULL)
	{
		g_task_return_error (task, error);
		g_object_unref (task);
		return;
	}

	g_clear_object (&priv->file_info_modified);

	g_task_return_boolean (task, TRUE);
	g_object_unref (task);
}

static void
save_as__query_all_previous_metadata_cb (GObject      *source_object,
					 GAsyncResult *result,
					 gpointer      user_data)
{
	GFile *location = G_FILE (source_object);
	GTask *task = G_TASK (user_data);
	TeplFileMetadata *metadata = g_task_get_source_object (task);
	TeplFileMetadataPrivate *priv = tepl_file_metadata_get_instance_private (metadata);
	GFileInfo *file_info;
	GError *error = NULL;
	gchar **attributes;
	gint attr_num;

	file_info = _tepl_metadata_query_info_finish (location, result, &error);

	if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED))
	{
		print_gvfs_metadata_not_supported_warning ();
	}

	if (error != NULL)
	{
		/* Ignore error because the purpose is to unset the previous
		 * metadata. If we get an error here we are unable to unset the
		 * metadata, so just do nothing about it.
		 * If we get again an error in save_as__set_attributes_cb(), the
		 * error will be returned there.
		 */
		g_clear_error (&error);
		g_clear_object (&file_info);
	}

	if (file_info == NULL)
	{
		file_info = g_file_info_new ();
		/* After that, take the same code path, so that that code path
		 * is better tested and is written in a general way.
		 * Even though since file_info is empty, it would be possible to
		 * directly call _tepl_metadata_set_attributes_async() with
		 * priv->file_info_all.
		 */
	}

	attributes = g_file_info_list_attributes (file_info, "metadata");
	for (attr_num = 0; attributes != NULL && attributes[attr_num] != NULL; attr_num++)
	{
		const gchar *cur_attribute = attributes[attr_num];

		/* Unset. */
		g_file_info_set_attribute (file_info,
					   cur_attribute,
					   G_FILE_ATTRIBUTE_TYPE_INVALID,
					   NULL);
	}
	g_strfreev (attributes);

	attributes = g_file_info_list_attributes (priv->file_info_all, "metadata");
	for (attr_num = 0; attributes != NULL && attributes[attr_num] != NULL; attr_num++)
	{
		const gchar *cur_attribute = attributes[attr_num];
		GFileAttributeType attribute_type = G_FILE_ATTRIBUTE_TYPE_INVALID;
		gpointer attribute_value = NULL;

		g_file_info_get_attribute_data (priv->file_info_all,
						cur_attribute,
						&attribute_type,
						&attribute_value,
						NULL);

		g_file_info_set_attribute (file_info,
					   cur_attribute,
					   attribute_type,
					   attribute_value);
	}
	g_strfreev (attributes);

	_tepl_metadata_set_attributes_async (location,
					     file_info,
					     g_task_get_priority (task),
					     g_task_get_cancellable (task),
					     set_attributes_cb, /* Common callback with normal save. */
					     task);
	g_object_unref (file_info);
}

static void
start_to_save_as (GTask *task)
{
	GFile *location = g_task_get_task_data (task);

	_tepl_metadata_query_info_async (location,
					 g_task_get_priority (task),
					 g_task_get_cancellable (task),
					 save_as__query_all_previous_metadata_cb,
					 task);
}

static void
start_to_save_modified_metadata (GTask *task)
{
	TeplFileMetadata *metadata = g_task_get_source_object (task);
	TeplFileMetadataPrivate *priv = tepl_file_metadata_get_instance_private (metadata);
	GFile *location = g_task_get_task_data (task);

	if (priv->file_info_modified == NULL)
	{
		g_task_return_boolean (task, TRUE);
		g_object_unref (task);
		return;
	}

	_tepl_metadata_set_attributes_async (location,
					     priv->file_info_modified,
					     g_task_get_priority (task),
					     g_task_get_cancellable (task),
					     set_attributes_cb, /* Common callback with save_as. */
					     task);
}

/**
 * tepl_file_metadata_save_async:
 * @metadata: a #TeplFileMetadata.
 * @location: a #GFile.
 * @save_as: whether it's for a 'save as' operation.
 * @io_priority: the I/O priority of the request. E.g. %G_PRIORITY_LOW,
 *   %G_PRIORITY_DEFAULT or %G_PRIORITY_HIGH.
 * @cancellable: (nullable): optional #GCancellable object, %NULL to ignore.
 * @callback: (scope async): a #GAsyncReadyCallback to call when the request is
 *   satisfied.
 * @user_data: user data to pass to @callback.
 *
 * Saves asynchronously the metadata for @location. Note that the metadata is
 * not saved <emphasis>to</emphasis> @location, the metadata is saved elsewhere,
 * <emphasis>for</emphasis> @location.
 *
 * @location must exist on the filesystem, otherwise an error is returned.
 *
 * If @save_as is %FALSE, only the <emphasis>modified</emphasis> metadata is
 * saved. A call to tepl_file_metadata_set() marks that metadata as modified. A
 * successful call to tepl_file_metadata_load_async() deletes all previous
 * metadata stored in the #TeplFileMetadata object, including modified metadata.
 * A successful call to tepl_file_metadata_save_async() marks the modified
 * metadata as saved, so those metadata will no longer be marked as modified
 * (but will still be part of #TeplFileMetadata).
 *
 * If @save_as is %TRUE, this function:
 * 1. Erases all previously stored metadata for @location.
 * 2. Then saves <emphasis>all</emphasis> the metadata of @metadata for
 *    @location.
 *
 * @save_as can be set to %TRUE in two different situations: (1) save a new
 * document for the first time; (2) open a file, possibly modify it, then save
 * it to another location. In both cases, a #GFile needs to be chosen by the
 * user, and if it replaces an existing file, the user needs to confirm to
 * overwrite it.
 *
 * See the #GAsyncResult documentation to know how to use this function.
 *
 * Since: 5.0
 */
void
tepl_file_metadata_save_async (TeplFileMetadata    *metadata,
			       GFile               *location,
			       gboolean             save_as,
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
	g_return_if_fail (!priv->is_saving);

	priv->is_saving = TRUE;

	task = g_task_new (metadata, cancellable, callback, user_data);
	g_task_set_priority (task, io_priority);
	g_task_set_task_data (task, g_object_ref (location), g_object_unref);

	if (save_as)
	{
		start_to_save_as (task);
	}
	else
	{
		start_to_save_modified_metadata (task);
	}
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
	TeplFileMetadataPrivate *priv;

	g_return_val_if_fail (TEPL_IS_FILE_METADATA (metadata), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail (g_task_is_valid (result, metadata), FALSE);

	priv = tepl_file_metadata_get_instance_private (metadata);
	priv->is_saving = FALSE;

	return g_task_propagate_boolean (G_TASK (result), error);
}
