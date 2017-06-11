/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef TEPL_FILE_LOADER_H
#define TEPL_FILE_LOADER_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gio/gio.h>
#include <tepl/tepl-types.h>
#include <tepl/tepl-file.h>

G_BEGIN_DECLS

#define TEPL_TYPE_FILE_LOADER (tepl_file_loader_get_type ())
G_DECLARE_DERIVABLE_TYPE (TeplFileLoader, tepl_file_loader,
			  TEPL, FILE_LOADER,
			  GObject)

#define TEPL_FILE_LOADER_ERROR tepl_file_loader_error_quark ()

/**
 * TeplFileLoaderError:
 * @TEPL_FILE_LOADER_ERROR_TOO_BIG: The file is too big.
 * @TEPL_FILE_LOADER_ERROR_ENCODING_AUTO_DETECTION_FAILED: It is not possible to
 *   detect the encoding automatically.
 *
 * An error code used with the %TEPL_FILE_LOADER_ERROR domain.
 *
 * Since: 1.0
 */
typedef enum _TeplFileLoaderError
{
	TEPL_FILE_LOADER_ERROR_TOO_BIG,
	TEPL_FILE_LOADER_ERROR_ENCODING_AUTO_DETECTION_FAILED
} TeplFileLoaderError;

struct _TeplFileLoaderClass
{
	GObjectClass parent_class;

	gpointer padding[12];
};

GQuark			tepl_file_loader_error_quark				(void);

TeplFileLoader *	tepl_file_loader_new					(TeplBuffer *buffer,
										 TeplFile   *file);

TeplBuffer *		tepl_file_loader_get_buffer				(TeplFileLoader *loader);

TeplFile *		tepl_file_loader_get_file				(TeplFileLoader *loader);

GFile *			tepl_file_loader_get_location				(TeplFileLoader *loader);

gint64			tepl_file_loader_get_max_size				(TeplFileLoader *loader);

void			tepl_file_loader_set_max_size				(TeplFileLoader *loader,
										 gint64          max_size);

gint64			tepl_file_loader_get_chunk_size				(TeplFileLoader *loader);

void			tepl_file_loader_set_chunk_size				(TeplFileLoader *loader,
										 gint64          chunk_size);

void			tepl_file_loader_load_async				(TeplFileLoader        *loader,
										 gint                   io_priority,
										 GCancellable          *cancellable,
										 GFileProgressCallback  progress_callback,
										 gpointer               progress_callback_data,
										 GDestroyNotify         progress_callback_notify,
										 GAsyncReadyCallback    callback,
										 gpointer               user_data);

gboolean		tepl_file_loader_load_finish				(TeplFileLoader  *loader,
										 GAsyncResult    *result,
										 GError         **error);

const TeplEncoding *	tepl_file_loader_get_encoding				(TeplFileLoader *loader);

TeplNewlineType		tepl_file_loader_get_newline_type			(TeplFileLoader *loader);

G_GNUC_INTERNAL
gint64			_tepl_file_loader_get_encoding_converter_buffer_size	(void);

G_END_DECLS

#endif /* TEPL_FILE_LOADER_H */
