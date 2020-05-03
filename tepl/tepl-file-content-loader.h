/* SPDX-FileCopyrightText: 2016 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_FILE_CONTENT_LOADER_H
#define TEPL_FILE_CONTENT_LOADER_H

#include <gio/gio.h>
#include "tepl-file-content.h"

G_BEGIN_DECLS

/* 50MB, not 50MiB because the UI shows the value in MB. */
#define TEPL_FILE_CONTENT_LOADER_DEFAULT_MAX_SIZE (50 * 1000 * 1000)

/* Should be small enough for slow network connections, to report progress. */
#define TEPL_FILE_CONTENT_LOADER_DEFAULT_CHUNK_SIZE (8 * 1024)

#define TEPL_TYPE_FILE_CONTENT_LOADER             (_tepl_file_content_loader_get_type ())
#define TEPL_FILE_CONTENT_LOADER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_FILE_CONTENT_LOADER, TeplFileContentLoader))
#define TEPL_FILE_CONTENT_LOADER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_FILE_CONTENT_LOADER, TeplFileContentLoaderClass))
#define TEPL_IS_FILE_CONTENT_LOADER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_FILE_CONTENT_LOADER))
#define TEPL_IS_FILE_CONTENT_LOADER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_FILE_CONTENT_LOADER))
#define TEPL_FILE_CONTENT_LOADER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_FILE_CONTENT_LOADER, TeplFileContentLoaderClass))

typedef struct _TeplFileContentLoader         TeplFileContentLoader;
typedef struct _TeplFileContentLoaderClass    TeplFileContentLoaderClass;
typedef struct _TeplFileContentLoaderPrivate  TeplFileContentLoaderPrivate;

struct _TeplFileContentLoader
{
	GObject parent;

	TeplFileContentLoaderPrivate *priv;
};

struct _TeplFileContentLoaderClass
{
	GObjectClass parent_class;
};

G_GNUC_INTERNAL
GType			_tepl_file_content_loader_get_type		(void);

G_GNUC_INTERNAL
TeplFileContentLoader *	_tepl_file_content_loader_new_from_file		(GFile *location);

G_GNUC_INTERNAL
void			_tepl_file_content_loader_set_max_size		(TeplFileContentLoader *loader,
									 gint64                 max_size);

G_GNUC_INTERNAL
void			_tepl_file_content_loader_set_chunk_size	(TeplFileContentLoader *loader,
									 gint64                 chunk_size);

G_GNUC_INTERNAL
void			_tepl_file_content_loader_load_async		(TeplFileContentLoader *loader,
									 gint                   io_priority,
									 GCancellable          *cancellable,
									 GFileProgressCallback  progress_callback,
									 gpointer               progress_callback_data,
									 GDestroyNotify         progress_callback_notify,
									 GAsyncReadyCallback    callback,
									 gpointer               user_data);

G_GNUC_INTERNAL
gboolean		_tepl_file_content_loader_load_finish		(TeplFileContentLoader  *loader,
									 GAsyncResult           *result,
									 GError                **error);

G_GNUC_INTERNAL
TeplFileContent *	_tepl_file_content_loader_get_content		(TeplFileContentLoader *loader);

G_GNUC_INTERNAL
const gchar *		_tepl_file_content_loader_get_etag		(TeplFileContentLoader *loader);

G_GNUC_INTERNAL
gboolean		_tepl_file_content_loader_get_readonly		(TeplFileContentLoader *loader);

G_END_DECLS

#endif /* TEPL_FILE_CONTENT_LOADER_H */
