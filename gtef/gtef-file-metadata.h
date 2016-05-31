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

#ifndef GTEF_FILE_METADATA_H
#define GTEF_FILE_METADATA_H

#if !defined (GTEF_H_INSIDE) && !defined (GTEF_COMPILATION)
#error "Only <gtef/gtef.h> can be included directly."
#endif

#include <gio/gio.h>
#include <gtef/gtef-types.h>

G_BEGIN_DECLS

#define GTEF_TYPE_FILE_METADATA (gtef_file_metadata_get_type ())
G_DECLARE_DERIVABLE_TYPE (GtefFileMetadata, gtef_file_metadata,
			  GTEF, FILE_METADATA,
			  GObject)

struct _GtefFileMetadataClass
{
	GObjectClass parent_class;

	gpointer padding[12];
};

GtefFileMetadata *	gtef_file_metadata_new				(GtefFile *file);

GtefFile *		gtef_file_metadata_get_file			(GtefFileMetadata *metadata);

gchar *			gtef_file_metadata_get				(GtefFileMetadata *metadata,
									 const gchar      *key);

void			gtef_file_metadata_set				(GtefFileMetadata *metadata,
									 const gchar      *key,
									 const gchar      *value);

gboolean		gtef_file_metadata_load				(GtefFileMetadata  *metadata,
									 GCancellable      *cancellable,
									 GError           **error);

void			gtef_file_metadata_load_async			(GtefFileMetadata    *metadata,
									 gint                 io_priority,
									 GCancellable        *cancellable,
									 GAsyncReadyCallback  callback,
									 gpointer             user_data);

gboolean		gtef_file_metadata_load_finish			(GtefFileMetadata  *metadata,
									 GAsyncResult      *result,
									 GError           **error);

gboolean		gtef_file_metadata_save				(GtefFileMetadata  *metadata,
									 GCancellable      *cancellable,
									 GError           **error);

void			gtef_file_metadata_save_async			(GtefFileMetadata    *metadata,
									 gint                 io_priority,
									 GCancellable        *cancellable,
									 GAsyncReadyCallback  callback,
									 gpointer             user_data);

gboolean		gtef_file_metadata_save_finish			(GtefFileMetadata  *metadata,
									 GAsyncResult      *result,
									 GError           **error);

G_GNUC_INTERNAL
void			_gtef_file_metadata_set_use_gvfs_metadata	(GtefFileMetadata *metadata,
									 gboolean          use_gvfs_metadata);

G_END_DECLS

#endif /* GTEF_FILE_METADATA_H */
