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

#ifndef TEPL_METADATA_MANAGER_H
#define TEPL_METADATA_MANAGER_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gio/gio.h>
#include <tepl/tepl-metadata.h>

G_BEGIN_DECLS

#define TEPL_TYPE_METADATA_MANAGER             (tepl_metadata_manager_get_type ())
#define TEPL_METADATA_MANAGER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_METADATA_MANAGER, TeplMetadataManager))
#define TEPL_METADATA_MANAGER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_METADATA_MANAGER, TeplMetadataManagerClass))
#define TEPL_IS_METADATA_MANAGER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_METADATA_MANAGER))
#define TEPL_IS_METADATA_MANAGER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_METADATA_MANAGER))
#define TEPL_METADATA_MANAGER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_METADATA_MANAGER, TeplMetadataManagerClass))

typedef struct _TeplMetadataManager         TeplMetadataManager;
typedef struct _TeplMetadataManagerClass    TeplMetadataManagerClass;
typedef struct _TeplMetadataManagerPrivate  TeplMetadataManagerPrivate;

struct _TeplMetadataManager
{
	GObject parent;

	TeplMetadataManagerPrivate *priv;
};

struct _TeplMetadataManagerClass
{
	GObjectClass parent_class;

	gpointer padding[12];
};

GType			tepl_metadata_manager_get_type		(void);

TeplMetadataManager *	tepl_metadata_manager_get_singleton	(void);

G_GNUC_INTERNAL
void			_tepl_metadata_manager_unref_singleton	(void);

void			tepl_metadata_manager_trim		(TeplMetadataManager *manager,
								 gint                 max_number_of_locations);

gboolean		tepl_metadata_manager_load_from_disk	(TeplMetadataManager  *manager,
								 GFile                *from_file,
								 GError              **error);

gboolean		tepl_metadata_manager_save_to_disk	(TeplMetadataManager  *manager,
								 GFile                *to_file,
								 gboolean              trim,
								 GError              **error);

void			tepl_metadata_manager_copy_from		(TeplMetadataManager *from_manager,
								 GFile               *for_location,
								 TeplMetadata        *to_metadata);

void			tepl_metadata_manager_merge_into	(TeplMetadataManager *into_manager,
								 GFile               *for_location,
								 TeplMetadata        *from_metadata);

G_END_DECLS

#endif /* TEPL_METADATA_MANAGER_H */