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

#define TEPL_TYPE_FILE_LOADER             (tepl_file_loader_get_type ())
#define TEPL_FILE_LOADER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_FILE_LOADER, TeplFileLoader))
#define TEPL_FILE_LOADER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_FILE_LOADER, TeplFileLoaderClass))
#define TEPL_IS_FILE_LOADER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_FILE_LOADER))
#define TEPL_IS_FILE_LOADER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_FILE_LOADER))
#define TEPL_FILE_LOADER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_FILE_LOADER, TeplFileLoaderClass))

typedef struct _TeplFileLoader         TeplFileLoader;
typedef struct _TeplFileLoaderClass    TeplFileLoaderClass;
typedef struct _TeplFileLoaderPrivate  TeplFileLoaderPrivate;

struct _TeplFileLoader
{
	GObject parent;

	TeplFileLoaderPrivate *priv;
};

struct _TeplFileLoaderClass
{
	GObjectClass parent_class;

	gpointer padding[12];
};

_TEPL_EXTERN
GType			tepl_file_loader_get_type		(void);

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
