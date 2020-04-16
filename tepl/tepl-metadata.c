/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2020 - Sébastien Wilmet <swilmet@gnome.org>
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

#include "config.h"
#include "tepl-metadata.h"
#include "tepl-metadata-store.h"

/* Almost a drop-in replacement for, or a wrapping of, the
 * GFile-metadata-related API (just what we need) to either:
 * - Call the GFile API in case GVfs metadata is used.
 * - Use the TeplMetadataStore otherwise.
 */

/******************************************************************************/
/* Utils */

static gboolean force_using_metadata_store;

void
_tepl_metadata_set_force_using_metadata_store (gboolean value)
{
	force_using_metadata_store = value != FALSE;
}

static gboolean
use_gvfs_metadata (void)
{
	if (force_using_metadata_store)
	{
		return FALSE;
	}

#if ENABLE_GVFS_METADATA
	return TRUE;
#else
	return FALSE;
#endif
}

/******************************************************************************/
/* Simplify TeplMetadataStore is_activated/is_loaded/is_loading/load */

static void
load_metadata_store__notify_loaded_cb (TeplMetadataStore *store,
				       GParamSpec        *pspec,
				       GTask             *task)
{
	g_task_return_boolean (task, TRUE);
	g_object_unref (task);
}

static void
load_metadata_store__load_cb (GObject      *source_object,
			      GAsyncResult *result,
			      gpointer      user_data)
{
	TeplMetadataStore *store = TEPL_METADATA_STORE (source_object);
	GTask *task = G_TASK (user_data);
	GError *error = NULL;
	gboolean ok;

	ok = _tepl_metadata_store_load_finish (store, result, &error);
	if (error != NULL)
	{
		g_task_return_error (task, error);
		g_object_unref (task);
		return;
	}

	g_task_return_boolean (task, ok);
	g_object_unref (task);
}

static void
load_metadata_store_async (TeplMetadataStore   *store,
			   gint                 io_priority,
			   GCancellable        *cancellable,
			   GAsyncReadyCallback  callback,
			   gpointer             user_data)
{
	GTask *task;

	g_return_if_fail (TEPL_IS_METADATA_STORE (store));
	g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));

	task = g_task_new (store, cancellable, callback, user_data);
	g_task_set_priority (task, io_priority);

	if (!_tepl_metadata_store_is_activated (store))
	{
		g_task_return_new_error (task,
					 G_IO_ERROR,
					 G_IO_ERROR_FAILED,
					 "Failed to use the TeplMetadataStore because "
					 "tepl_metadata_store_set_store_file() has not been called.");
		g_object_unref (task);
	}
	else if (_tepl_metadata_store_is_loaded (store))
	{
		g_task_return_boolean (task, TRUE);
		g_object_unref (task);
	}
	else if (_tepl_metadata_store_is_loading (store))
	{
		g_signal_connect (store,
				  "notify::loaded",
				  G_CALLBACK (load_metadata_store__notify_loaded_cb),
				  task);
	}
	else
	{
		_tepl_metadata_store_load_async (store,
						 io_priority,
						 cancellable,
						 load_metadata_store__load_cb,
						 task);
	}
}

static gboolean
load_metadata_store_finish (TeplMetadataStore  *store,
			    GAsyncResult       *result,
			    GError            **error)
{
	g_return_val_if_fail (TEPL_IS_METADATA_STORE (store), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail (g_task_is_valid (result, store), FALSE);

	return g_task_propagate_boolean (G_TASK (result), error);
}

/******************************************************************************/
/* Query info */

static void
do_query_info (TeplMetadataStore *store,
	       GTask             *task)
{
	GFile *location = g_task_get_source_object (task);
	GFileInfo *file_info;

	file_info = _tepl_metadata_store_get_metadata_for_location (store, location);
	if (file_info == NULL)
	{
		/* See the Returns of g_file_query_info_finish(). */
		file_info = g_file_info_new ();
	}

	g_task_return_pointer (task, file_info, g_object_unref);
	g_object_unref (task);
}

static void
query_info__load_metadata_store_cb (GObject      *source_object,
				    GAsyncResult *result,
				    gpointer      user_data)
{
	TeplMetadataStore *store = TEPL_METADATA_STORE (source_object);
	GTask *task = G_TASK (user_data);
	GError *error = NULL;

	load_metadata_store_finish (store, result, &error);
	if (error != NULL)
	{
		g_task_return_error (task, error);
		g_object_unref (task);
		return;
	}

	do_query_info (store, task);
}

void
_tepl_metadata_query_info_async (GFile               *location,
				 gint                 io_priority,
				 GCancellable        *cancellable,
				 GAsyncReadyCallback  callback,
				 gpointer             user_data)
{
	GTask *task;
	TeplMetadataStore *store;

	if (use_gvfs_metadata ())
	{
		g_file_query_info_async (location,
					 "metadata::*",
					 G_FILE_QUERY_INFO_NONE,
					 io_priority,
					 cancellable,
					 callback,
					 user_data);
		return;
	}

	g_return_if_fail (G_IS_FILE (location));
	g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));

	task = g_task_new (location, cancellable, callback, user_data);
	g_task_set_priority (task, io_priority);

	store = tepl_metadata_store_get_singleton ();

	load_metadata_store_async (store,
				   io_priority,
				   cancellable,
				   query_info__load_metadata_store_cb,
				   task);
}

GFileInfo *
_tepl_metadata_query_info_finish (GFile         *location,
				  GAsyncResult  *result,
				  GError       **error)
{
	if (use_gvfs_metadata ())
	{
		return g_file_query_info_finish (location, result, error);
	}

	g_return_val_if_fail (G_IS_FILE (location), NULL);
	g_return_val_if_fail (error == NULL || *error == NULL, NULL);
	g_return_val_if_fail (g_task_is_valid (result, location), NULL);

	return g_task_propagate_pointer (G_TASK (result), error);
}

/******************************************************************************/
/* Set attributes */

static void
do_set_attributes (TeplMetadataStore *store,
		   GTask             *task)
{
	GFile *location = g_task_get_source_object (task);
	GFileInfo *file_info_to_merge = g_task_get_task_data (task);
	GFileInfo *full_file_info;
	gchar **attributes;
	gint attr_num;

	full_file_info = _tepl_metadata_store_get_metadata_for_location (store, location);

	attributes = g_file_info_list_attributes (file_info_to_merge, "metadata");
	for (attr_num = 0; attributes != NULL && attributes[attr_num] != NULL; attr_num++)
	{
		const gchar *cur_attribute = attributes[attr_num];
		GFileAttributeType attribute_type = G_FILE_ATTRIBUTE_TYPE_INVALID;
		gpointer attribute_value = NULL;

		g_file_info_get_attribute_data (file_info_to_merge,
						cur_attribute,
						&attribute_type,
						&attribute_value,
						NULL);

		if (attribute_type == G_FILE_ATTRIBUTE_TYPE_INVALID &&
		    full_file_info != NULL)
		{
			g_file_info_remove_attribute (full_file_info, cur_attribute);
		}
		if (attribute_type == G_FILE_ATTRIBUTE_TYPE_STRING)
		{
			if (full_file_info == NULL)
			{
				full_file_info = g_file_info_new ();
			}

			g_file_info_set_attribute_string (full_file_info,
							  cur_attribute,
							  attribute_value);
		}
	}

	_tepl_metadata_store_set_metadata_for_location (store, location, full_file_info);
	g_clear_object (&full_file_info);

	g_task_return_boolean (task, TRUE);
	g_object_unref (task);
}

static void
set_attributes__load_metadata_store_cb (GObject      *source_object,
					GAsyncResult *result,
					gpointer      user_data)
{
	TeplMetadataStore *store = TEPL_METADATA_STORE (source_object);
	GTask *task = G_TASK (user_data);
	GError *error = NULL;

	load_metadata_store_finish (store, result, &error);
	if (error != NULL)
	{
		g_task_return_error (task, error);
		g_object_unref (task);
		return;
	}

	do_set_attributes (store, task);
}

void
_tepl_metadata_set_attributes_async (GFile               *location,
				     GFileInfo           *file_info,
				     gint                 io_priority,
				     GCancellable        *cancellable,
				     GAsyncReadyCallback  callback,
				     gpointer             user_data)
{
	GTask *task;
	TeplMetadataStore *store;

	if (use_gvfs_metadata ())
	{
		g_file_set_attributes_async (location,
					     file_info,
					     G_FILE_QUERY_INFO_NONE,
					     io_priority,
					     cancellable,
					     callback,
					     user_data);
		return;
	}

	task = g_task_new (location, cancellable, callback, user_data);
	g_task_set_priority (task, io_priority);
	g_task_set_task_data (task, g_file_info_dup (file_info), g_object_unref);

	store = tepl_metadata_store_get_singleton ();

	load_metadata_store_async (store,
				   io_priority,
				   cancellable,
				   set_attributes__load_metadata_store_cb,
				   task);
}

gboolean
_tepl_metadata_set_attributes_finish (GFile         *location,
				      GAsyncResult  *result,
				      GError       **error)
{
	if (use_gvfs_metadata ())
	{
		return g_file_set_attributes_finish (location, result, NULL, error);
	}

	g_return_val_if_fail (G_IS_FILE (location), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail (g_task_is_valid (result, location), FALSE);

	return g_task_propagate_boolean (G_TASK (result), error);
}
