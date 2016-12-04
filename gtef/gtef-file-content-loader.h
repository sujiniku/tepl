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

#ifndef GTEF_FILE_CONTENT_LOADER_H
#define GTEF_FILE_CONTENT_LOADER_H

#include <gio/gio.h>

G_BEGIN_DECLS

/* 50MB, not 50MiB because the UI shows the value in MB. */
#define GTEF_FILE_CONTENT_LOADER_DEFAULT_MAX_SIZE (50 * 1000 * 1000)

/* Should be small enough for slow network connections, to report progress. */
#define GTEF_FILE_CONTENT_LOADER_DEFAULT_CHUNK_SIZE (8 * 1024)

#define GTEF_TYPE_FILE_CONTENT_LOADER             (_gtef_file_content_loader_get_type ())
#define GTEF_FILE_CONTENT_LOADER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTEF_TYPE_FILE_CONTENT_LOADER, GtefFileContentLoader))
#define GTEF_FILE_CONTENT_LOADER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GTEF_TYPE_FILE_CONTENT_LOADER, GtefFileContentLoaderClass))
#define GTEF_IS_FILE_CONTENT_LOADER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTEF_TYPE_FILE_CONTENT_LOADER))
#define GTEF_IS_FILE_CONTENT_LOADER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GTEF_TYPE_FILE_CONTENT_LOADER))
#define GTEF_FILE_CONTENT_LOADER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GTEF_TYPE_FILE_CONTENT_LOADER, GtefFileContentLoaderClass))

typedef struct _GtefFileContentLoader         GtefFileContentLoader;
typedef struct _GtefFileContentLoaderClass    GtefFileContentLoaderClass;
typedef struct _GtefFileContentLoaderPrivate  GtefFileContentLoaderPrivate;

struct _GtefFileContentLoader
{
	GObject parent;

	GtefFileContentLoaderPrivate *priv;
};

struct _GtefFileContentLoaderClass
{
	GObjectClass parent_class;
};

G_GNUC_INTERNAL
GType			_gtef_file_content_loader_get_type		(void);

G_GNUC_INTERNAL
GtefFileContentLoader *	_gtef_file_content_loader_new_from_file		(GFile *location);

G_GNUC_INTERNAL
void			_gtef_file_content_loader_set_max_size		(GtefFileContentLoader *loader,
									 gint64                 max_size);

G_GNUC_INTERNAL
void			_gtef_file_content_loader_set_chunk_size	(GtefFileContentLoader *loader,
									 gint64                 chunk_size);

G_GNUC_INTERNAL
void			_gtef_file_content_loader_load_async		(GtefFileContentLoader *loader,
									 gint                   io_priority,
									 GCancellable          *cancellable,
									 GFileProgressCallback  progress_callback,
									 gpointer               progress_callback_data,
									 GDestroyNotify         progress_callback_notify,
									 GAsyncReadyCallback    callback,
									 gpointer               user_data);

G_GNUC_INTERNAL
gboolean		_gtef_file_content_loader_load_finish		(GtefFileContentLoader  *loader,
									 GAsyncResult           *result,
									 GError                **error);

G_GNUC_INTERNAL
GQueue *		_gtef_file_content_loader_get_content		(GtefFileContentLoader *loader);

G_GNUC_INTERNAL
gboolean		_gtef_file_content_loader_get_readonly		(GtefFileContentLoader *loader);

G_END_DECLS

#endif /* GTEF_FILE_CONTENT_LOADER_H */
