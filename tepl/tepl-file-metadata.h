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

#ifndef TEPL_FILE_METADATA_H
#define TEPL_FILE_METADATA_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gio/gio.h>

G_BEGIN_DECLS

#define TEPL_TYPE_FILE_METADATA (tepl_file_metadata_get_type ())
G_DECLARE_DERIVABLE_TYPE (TeplFileMetadata, tepl_file_metadata,
			  TEPL, FILE_METADATA,
			  GObject)

struct _TeplFileMetadataClass
{
	GObjectClass parent_class;

	gpointer padding[12];
};

TeplFileMetadata *	tepl_file_metadata_new				(void);

gchar *			tepl_file_metadata_get				(TeplFileMetadata *metadata,
									 const gchar      *key);

void			tepl_file_metadata_set				(TeplFileMetadata *metadata,
									 const gchar      *key,
									 const gchar      *value);

void			tepl_file_metadata_load_async			(TeplFileMetadata    *metadata,
									 GFile               *location,
									 gint                 io_priority,
									 GCancellable        *cancellable,
									 GAsyncReadyCallback  callback,
									 gpointer             user_data);

gboolean		tepl_file_metadata_load_finish			(TeplFileMetadata  *metadata,
									 GAsyncResult      *result,
									 GError           **error);

void			tepl_file_metadata_save_async			(TeplFileMetadata    *metadata,
									 GFile               *location,
									 gboolean             save_as,
									 gint                 io_priority,
									 GCancellable        *cancellable,
									 GAsyncReadyCallback  callback,
									 gpointer             user_data);

gboolean		tepl_file_metadata_save_finish			(TeplFileMetadata  *metadata,
									 GAsyncResult      *result,
									 GError           **error);

#if 0
G_GNUC_INTERNAL
void			_tepl_file_metadata_set_use_gvfs_metadata	(TeplFileMetadata *metadata,
									 gboolean          use_gvfs_metadata);
#endif

G_END_DECLS

#endif /* TEPL_FILE_METADATA_H */
