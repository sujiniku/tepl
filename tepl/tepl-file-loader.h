/* SPDX-FileCopyrightText: 2016-2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_FILE_LOADER_H
#define TEPL_FILE_LOADER_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gio/gio.h>
#include <tepl/tepl-buffer.h>
#include <tepl/tepl-file.h>

G_BEGIN_DECLS

#define TEPL_TYPE_FILE_LOADER (tepl_file_loader_get_type ())
_TEPL_EXTERN
G_DECLARE_DERIVABLE_TYPE (TeplFileLoader, tepl_file_loader,
			  TEPL, FILE_LOADER,
			  GObject)

struct _TeplFileLoaderClass
{
	GObjectClass parent_class;

	gpointer padding[12];
};

_TEPL_EXTERN
TeplFileLoader *	tepl_file_loader_new			(TeplBuffer *buffer,
								 TeplFile   *file);

_TEPL_EXTERN
TeplBuffer *		tepl_file_loader_get_buffer		(TeplFileLoader *loader);

_TEPL_EXTERN
TeplFile *		tepl_file_loader_get_file		(TeplFileLoader *loader);

_TEPL_EXTERN
GFile *			tepl_file_loader_get_location		(TeplFileLoader *loader);

_TEPL_EXTERN
void			tepl_file_loader_load_async		(TeplFileLoader      *loader,
								 gint                 io_priority,
								 GCancellable        *cancellable,
								 GAsyncReadyCallback  callback,
								 gpointer             user_data);

_TEPL_EXTERN
gboolean		tepl_file_loader_load_finish		(TeplFileLoader  *loader,
								 GAsyncResult    *result,
								 GError         **error);

G_END_DECLS

#endif /* TEPL_FILE_LOADER_H */
