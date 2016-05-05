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

#ifndef GTEF_FILE_H
#define GTEF_FILE_H

#if !defined (GTEF_H_INSIDE) && !defined (GTEF_COMPILATION)
#error "Only <gtef/gtef.h> can be included directly."
#endif

#include <gtksourceview/gtksource.h>

G_BEGIN_DECLS

#define GTEF_TYPE_FILE (gtef_file_get_type ())
G_DECLARE_DERIVABLE_TYPE (GtefFile, gtef_file,
			  GTEF, FILE,
			  GtkSourceFile)

struct _GtefFileClass
{
	GtkSourceFileClass parent_class;

	gpointer padding[12];
};

GtefFile *	gtef_file_new				(void);

gchar *		gtef_file_get_metadata			(GtefFile    *file,
							 const gchar *key);

void		gtef_file_set_metadata			(GtefFile    *file,
							 const gchar *key,
							 const gchar *value);

gboolean	gtef_file_load_metadata			(GtefFile      *file,
							 GCancellable  *cancellable,
							 GError       **error);

void		gtef_file_load_metadata_async		(GtefFile            *file,
							 gint                 io_priority,
							 GCancellable        *cancellable,
							 GAsyncReadyCallback  callback,
							 gpointer             user_data);

gboolean	gtef_file_load_metadata_finish		(GtefFile      *file,
							 GAsyncResult  *result,
							 GError       **error);

gboolean	gtef_file_save_metadata			(GtefFile      *file,
							 GCancellable  *cancellable,
							 GError       **error);

void		gtef_file_save_metadata_async		(GtefFile            *file,
							 gint                 io_priority,
							 GCancellable        *cancellable,
							 GAsyncReadyCallback  callback,
							 gpointer             user_data);

gboolean	gtef_file_save_metadata_finish		(GtefFile      *file,
							 GAsyncResult  *result,
							 GError       **error);

G_GNUC_INTERNAL
void		_gtef_file_set_use_gvfs_metadata	(GtefFile *file,
							 gboolean  use_gvfs_metadata);

G_END_DECLS

#endif /* GTEF_FILE_H */
