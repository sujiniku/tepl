/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
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

_TEPL_EXTERN
GType			tepl_metadata_manager_get_type		(void);

_TEPL_EXTERN
TeplMetadataManager *	tepl_metadata_manager_get_singleton	(void);

G_GNUC_INTERNAL
void			_tepl_metadata_manager_unref_singleton	(void);

_TEPL_EXTERN
void			tepl_metadata_manager_trim		(TeplMetadataManager *manager,
								 gint                 max_number_of_locations);

_TEPL_EXTERN
gboolean		tepl_metadata_manager_load_from_disk	(TeplMetadataManager  *manager,
								 GFile                *from_file,
								 GError              **error);

_TEPL_EXTERN
gboolean		tepl_metadata_manager_save_to_disk	(TeplMetadataManager  *manager,
								 GFile                *to_file,
								 gboolean              trim,
								 GError              **error);

_TEPL_EXTERN
void			tepl_metadata_manager_copy_from		(TeplMetadataManager *from_manager,
								 GFile               *for_location,
								 TeplMetadata        *to_metadata);

_TEPL_EXTERN
void			tepl_metadata_manager_merge_into	(TeplMetadataManager *into_manager,
								 GFile               *for_location,
								 TeplMetadata        *from_metadata);

G_END_DECLS

#endif /* TEPL_METADATA_MANAGER_H */
