/* Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_FILE_LOADER_H
#define TEPL_FILE_LOADER_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gio/gio.h>
#include <tepl/tepl-buffer.h>
#include <tepl/tepl-encoding.h>
#include <tepl/tepl-file.h>

G_BEGIN_DECLS

#define TEPL_TYPE_FILE_LOADER (tepl_file_loader_get_type ())
_TEPL_EXTERN
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

_TEPL_EXTERN
GQuark			tepl_file_loader_error_quark				(void);

_TEPL_EXTERN
TeplFileLoader *	tepl_file_loader_new					(TeplBuffer *buffer,
										 TeplFile   *file);

_TEPL_EXTERN
TeplBuffer *		tepl_file_loader_get_buffer				(TeplFileLoader *loader);

_TEPL_EXTERN
TeplFile *		tepl_file_loader_get_file				(TeplFileLoader *loader);

_TEPL_EXTERN
GFile *			tepl_file_loader_get_location				(TeplFileLoader *loader);

_TEPL_EXTERN
gint64			tepl_file_loader_get_max_size				(TeplFileLoader *loader);

_TEPL_EXTERN
void			tepl_file_loader_set_max_size				(TeplFileLoader *loader,
										 gint64          max_size);

_TEPL_EXTERN
gint64			tepl_file_loader_get_chunk_size				(TeplFileLoader *loader);

_TEPL_EXTERN
void			tepl_file_loader_set_chunk_size				(TeplFileLoader *loader,
										 gint64          chunk_size);

_TEPL_EXTERN
void			tepl_file_loader_load_async				(TeplFileLoader        *loader,
										 gint                   io_priority,
										 GCancellable          *cancellable,
										 GFileProgressCallback  progress_callback,
										 gpointer               progress_callback_data,
										 GDestroyNotify         progress_callback_notify,
										 GAsyncReadyCallback    callback,
										 gpointer               user_data);

_TEPL_EXTERN
gboolean		tepl_file_loader_load_finish				(TeplFileLoader  *loader,
										 GAsyncResult    *result,
										 GError         **error);

_TEPL_EXTERN
const TeplEncoding *	tepl_file_loader_get_encoding				(TeplFileLoader *loader);

_TEPL_EXTERN
TeplNewlineType		tepl_file_loader_get_newline_type			(TeplFileLoader *loader);

G_END_DECLS

#endif /* TEPL_FILE_LOADER_H */
