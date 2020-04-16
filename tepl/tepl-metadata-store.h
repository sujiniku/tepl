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

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gio/gio.h>
#include <tepl/tepl-file-metadata.h>

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

GType			tepl_metadata_store_get_type		(void);

TeplMetadataStore *	tepl_metadata_store_get_singleton	(void);

G_GNUC_INTERNAL
void			_tepl_metadata_store_unref_singleton	(void);

void			tepl_metadata_store_trim		(TeplMetadataStore *store,
								 gint               max_number_of_locations);

gboolean		tepl_metadata_store_load		(TeplMetadataStore  *store,
								 GFile              *from_file,
								 GError            **error);

gboolean		tepl_metadata_store_save		(TeplMetadataStore  *store,
								 GFile              *to_file,
								 gboolean            trim,
								 GError            **error);

void			tepl_metadata_store_load_file_metadata	(TeplMetadataStore *store,
								 GFile             *location,
								 TeplFileMetadata  *file_metadata);

void			tepl_metadata_store_save_file_metadata	(TeplMetadataStore *store,
								 GFile             *location,
								 TeplFileMetadata  *file_metadata);

G_END_DECLS

#endif /* TEPL_METADATA_STORE_H */
