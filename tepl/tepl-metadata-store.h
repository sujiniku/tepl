/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2020 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef TEPL_METADATA_STORE_H
#define TEPL_METADATA_STORE_H

#include <gio/gio.h>

G_BEGIN_DECLS

#define TEPL_TYPE_METADATA_STORE             (tepl_metadata_store_get_type ())
#define TEPL_METADATA_STORE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_METADATA_STORE, TeplMetadataStore))
#define TEPL_METADATA_STORE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_METADATA_STORE, TeplMetadataStoreClass))
#define TEPL_IS_METADATA_STORE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_METADATA_STORE))
#define TEPL_IS_METADATA_STORE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_METADATA_STORE))
#define TEPL_METADATA_STORE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_METADATA_STORE, TeplMetadataStoreClass))

typedef struct _TeplMetadataStore         TeplMetadataStore;
typedef struct _TeplMetadataStoreClass    TeplMetadataStoreClass;
typedef struct _TeplMetadataStorePrivate  TeplMetadataStorePrivate;

struct _TeplMetadataStore
{
	GObject parent;

	TeplMetadataStorePrivate *priv;
};

struct _TeplMetadataStoreClass
{
	GObjectClass parent_class;

	gpointer padding[12];
};

GType			tepl_metadata_store_get_type			(void);

TeplMetadataStore *	tepl_metadata_store_get_singleton		(void);

void			tepl_metadata_store_set_store_file		(TeplMetadataStore *store,
									 GFile             *store_file);

void			tepl_metadata_store_set_max_number_of_locations	(TeplMetadataStore *store,
									 guint              max_number_of_locations);

gboolean		tepl_metadata_store_save			(TeplMetadataStore  *store,
									 GCancellable       *cancellable,
									 GError            **error);

G_GNUC_INTERNAL
void			_tepl_metadata_store_unref_singleton		(void);

G_GNUC_INTERNAL
void			_tepl_metadata_store_load_async			(TeplMetadataStore   *store,
									 gint                 io_priority,
									 GCancellable        *cancellable,
									 GAsyncReadyCallback  callback,
									 gpointer             user_data);

G_GNUC_INTERNAL
gboolean		_tepl_metadata_store_load_finish		(TeplMetadataStore  *store,
									 GAsyncResult       *result,
									 GError            **error);

G_GNUC_INTERNAL
gboolean		_tepl_metadata_store_is_loaded			(TeplMetadataStore *store);

G_GNUC_INTERNAL
GFileInfo *		_tepl_metadata_store_get_metadata_for_location	(TeplMetadataStore *store,
									 GFile             *location);

G_GNUC_INTERNAL
void			_tepl_metadata_store_set_metadata_for_location	(TeplMetadataStore *store,
									 GFile             *location,
									 GFileInfo         *metadata);

G_END_DECLS

#endif /* TEPL_METADATA_STORE_H */
