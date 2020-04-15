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
 * - Use the TeplMetadataStore otherwise. TODO: do it.
 */

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

	task = g_task_new (location, cancellable, callback, user_data);
	g_task_set_priority (task, io_priority);

	store = tepl_metadata_store_get_singleton ();

	/* TODO: check if TeplMetadataStore is activated. */

	if (_tepl_metadata_store_is_loaded (store))
	{
		GFileInfo *file_info;

		file_info = _tepl_metadata_store_get_metadata_for_location (store, location);

		g_task_return_pointer (task, file_info, g_object_unref);
		g_object_unref (task);
		return;
	}
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

void
_tepl_metadata_set_attributes_async (GFile               *location,
				     GFileInfo           *info,
				     gint                 io_priority,
				     GCancellable        *cancellable,
				     GAsyncReadyCallback  callback,
				     gpointer             user_data)
{
	if (use_gvfs_metadata ())
	{
		g_file_set_attributes_async (location,
					     info,
					     G_FILE_QUERY_INFO_NONE,
					     io_priority,
					     cancellable,
					     callback,
					     user_data);
	}
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

	return FALSE;
}
