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

#ifndef GTEF_FILE_LOADER_H
#define GTEF_FILE_LOADER_H

#if !defined (GTEF_H_INSIDE) && !defined (GTEF_COMPILATION)
#error "Only <gtef/gtef.h> can be included directly."
#endif

#include <gio/gio.h>
#include <gtef/gtef-types.h>

G_BEGIN_DECLS

#define GTEF_TYPE_FILE_LOADER (gtef_file_loader_get_type ())
G_DECLARE_DERIVABLE_TYPE (GtefFileLoader, gtef_file_loader,
			  GTEF, FILE_LOADER,
			  GObject)

#define GTEF_FILE_LOADER_ERROR gtef_file_loader_error_quark ()

/**
 * GtefFileLoaderError:
 * @GTEF_FILE_LOADER_ERROR_TOO_BIG: The file is too big.
 * @GTEF_FILE_LOADER_ERROR_ENCODING_AUTO_DETECTION_FAILED: It is not possible to
 *   detect the encoding automatically.
 *
 * An error code used with the %GTEF_FILE_LOADER_ERROR domain.
 *
 * Since: 1.0
 */
typedef enum _GtefFileLoaderError
{
	GTEF_FILE_LOADER_ERROR_TOO_BIG,
	GTEF_FILE_LOADER_ERROR_ENCODING_AUTO_DETECTION_FAILED
} GtefFileLoaderError;

struct _GtefFileLoaderClass
{
	GObjectClass parent_class;

	gpointer padding[12];
};

GQuark			gtef_file_loader_error_quark				(void);

GtefFileLoader *	gtef_file_loader_new					(GtefBuffer *buffer,
										 GtefFile   *file);

GtefBuffer *		gtef_file_loader_get_buffer				(GtefFileLoader *loader);

GtefFile *		gtef_file_loader_get_file				(GtefFileLoader *loader);

GFile *			gtef_file_loader_get_location				(GtefFileLoader *loader);

gint64			gtef_file_loader_get_max_size				(GtefFileLoader *loader);

void			gtef_file_loader_set_max_size				(GtefFileLoader *loader,
										 gint64          max_size);

gint64			gtef_file_loader_get_chunk_size				(GtefFileLoader *loader);

void			gtef_file_loader_set_chunk_size				(GtefFileLoader *loader,
										 gint64          chunk_size);

void			gtef_file_loader_load_async				(GtefFileLoader      *loader,
										 gint                 io_priority,
										 GCancellable        *cancellable,
										 GAsyncReadyCallback  callback,
										 gpointer             user_data);

gboolean		gtef_file_loader_load_finish				(GtefFileLoader  *loader,
										 GAsyncResult    *result,
										 GError         **error);

G_END_DECLS

#endif /* GTEF_FILE_LOADER_H */
