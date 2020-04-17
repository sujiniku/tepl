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

#ifndef TEPL_FILE_METADATA_H
#define TEPL_FILE_METADATA_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gio/gio.h>

G_BEGIN_DECLS

#define TEPL_TYPE_FILE_METADATA             (tepl_file_metadata_get_type ())
#define TEPL_FILE_METADATA(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_FILE_METADATA, TeplFileMetadata))
#define TEPL_FILE_METADATA_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_FILE_METADATA, TeplFileMetadataClass))
#define TEPL_IS_FILE_METADATA(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_FILE_METADATA))
#define TEPL_IS_FILE_METADATA_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_FILE_METADATA))
#define TEPL_FILE_METADATA_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_FILE_METADATA, TeplFileMetadataClass))

typedef struct _TeplFileMetadata         TeplFileMetadata;
typedef struct _TeplFileMetadataClass    TeplFileMetadataClass;
typedef struct _TeplFileMetadataPrivate  TeplFileMetadataPrivate;

struct _TeplFileMetadata
{
	GObject parent;

	TeplFileMetadataPrivate *priv;
};

struct _TeplFileMetadataClass
{
	GObjectClass parent_class;

	gpointer padding[12];
};

GType			tepl_file_metadata_get_type			(void);

TeplFileMetadata *	tepl_file_metadata_new				(void);

gchar *			tepl_file_metadata_get				(TeplFileMetadata *metadata,
									 const gchar      *key);

void			tepl_file_metadata_set				(TeplFileMetadata *metadata,
									 const gchar      *key,
									 const gchar      *value);

G_GNUC_INTERNAL
gboolean		_tepl_file_metadata_set_atime_str		(TeplFileMetadata *metadata,
									 const gchar      *atime_str);

G_GNUC_INTERNAL
gint			_tepl_file_metadata_compare_atime		(TeplFileMetadata *metadata1,
									 TeplFileMetadata *metadata2);

G_GNUC_INTERNAL
void			_tepl_file_metadata_insert_entry		(TeplFileMetadata *metadata,
									 const gchar      *key,
									 const gchar      *value);

G_GNUC_INTERNAL
void			_tepl_file_metadata_copy_into			(TeplFileMetadata *src_metadata,
									 TeplFileMetadata *dest_metadata);

G_GNUC_INTERNAL
void			_tepl_file_metadata_append_xml_to_string	(TeplFileMetadata *metadata,
									 GFile            *location,
									 GString          *string);

G_GNUC_INTERNAL
gboolean		_tepl_file_metadata_key_is_valid		(const gchar *key);

G_END_DECLS

#endif /* TEPL_FILE_METADATA_H */
