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

#include "tepl-metadata.h"

/* Almost a drop-in replacement for, or a wrapping of, the
 * GFile-metadata-related API (just what we need) to either:
 * - Call the GFile API in case GVfs metadata is used.
 * - Use the TeplMetadataStore otherwise. TODO: do it.
 */

void
_tepl_metadata_query_info_async (GFile               *location,
				 gint                 io_priority,
				 GCancellable        *cancellable,
				 GAsyncReadyCallback  callback,
				 gpointer             user_data)
{
	g_file_query_info_async (location,
				 "metadata::*",
				 G_FILE_QUERY_INFO_NONE,
				 io_priority,
				 cancellable,
				 callback,
				 user_data);
}

GFileInfo *
_tepl_metadata_query_info_finish (GFile         *location,
				  GAsyncResult  *result,
				  GError       **error)
{
	return g_file_query_info_finish (location, result, error);
}

void
_tepl_metadata_set_attributes_async (GFile               *location,
				     GFileInfo           *info,
				     gint                 io_priority,
				     GCancellable        *cancellable,
				     GAsyncReadyCallback  callback,
				     gpointer             user_data)
{
	g_file_set_attributes_async (location,
				     info,
				     G_FILE_QUERY_INFO_NONE,
				     io_priority,
				     cancellable,
				     callback,
				     user_data);
}

gboolean
_tepl_metadata_set_attributes_finish (GFile         *location,
				      GAsyncResult  *result,
				      GError       **error)
{
	return g_file_set_attributes_finish (location, result, NULL, error);
}
