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

#ifndef TEPL_METADATA_ATTIC_H
#define TEPL_METADATA_ATTIC_H

#include <gio/gio.h>
#include "tepl-metadata.h"

G_BEGIN_DECLS

#define TEPL_TYPE_METADATA_ATTIC             (_tepl_metadata_attic_get_type ())
#define TEPL_METADATA_ATTIC(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_METADATA_ATTIC, TeplMetadataAttic))
#define TEPL_METADATA_ATTIC_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_METADATA_ATTIC, TeplMetadataAtticClass))
#define TEPL_IS_METADATA_ATTIC(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_METADATA_ATTIC))
#define TEPL_IS_METADATA_ATTIC_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_METADATA_ATTIC))
#define TEPL_METADATA_ATTIC_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_METADATA_ATTIC, TeplMetadataAtticClass))

typedef struct _TeplMetadataAttic         TeplMetadataAttic;
typedef struct _TeplMetadataAtticClass    TeplMetadataAtticClass;
typedef struct _TeplMetadataAtticPrivate  TeplMetadataAtticPrivate;

struct _TeplMetadataAttic
{
	GObject parent;

	TeplMetadataAtticPrivate *priv;
};

struct _TeplMetadataAtticClass
{
	GObjectClass parent_class;
};

G_GNUC_INTERNAL
GType			_tepl_metadata_attic_get_type			(void);

G_GNUC_INTERNAL
TeplMetadataAttic *	_tepl_metadata_attic_new			(void);

G_GNUC_INTERNAL
gboolean		_tepl_metadata_attic_set_atime_str		(TeplMetadataAttic *metadata,
									 const gchar       *atime_str);

G_GNUC_INTERNAL
gint			_tepl_metadata_attic_compare_atime		(TeplMetadataAttic *metadata1,
									 TeplMetadataAttic *metadata2);

G_GNUC_INTERNAL
void			_tepl_metadata_attic_insert_entry		(TeplMetadataAttic *metadata,
									 const gchar       *key,
									 const gchar       *value);

G_GNUC_INTERNAL
void			_tepl_metadata_attic_append_xml_to_string	(TeplMetadataAttic *metadata,
									 GFile             *location,
									 GString           *string);

G_GNUC_INTERNAL
void			_tepl_metadata_attic_copy_into			(TeplMetadataAttic *from_metadata_attic,
									 TeplMetadata      *to_metadata);

G_GNUC_INTERNAL
void			_tepl_metadata_attic_merge_into			(TeplMetadataAttic *into_metadata_attic,
									 TeplMetadata      *from_metadata);

G_END_DECLS

#endif /* TEPL_METADATA_ATTIC_H */
