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

#ifndef TEPL_METADATA_H
#define TEPL_METADATA_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gio/gio.h>
#include <tepl/tepl-macros.h>

G_BEGIN_DECLS

#define TEPL_TYPE_METADATA             (tepl_metadata_get_type ())
#define TEPL_METADATA(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_METADATA, TeplMetadata))
#define TEPL_METADATA_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_METADATA, TeplMetadataClass))
#define TEPL_IS_METADATA(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_METADATA))
#define TEPL_IS_METADATA_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_METADATA))
#define TEPL_METADATA_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_METADATA, TeplMetadataClass))

typedef struct _TeplMetadata         TeplMetadata;
typedef struct _TeplMetadataClass    TeplMetadataClass;
typedef struct _TeplMetadataPrivate  TeplMetadataPrivate;

struct _TeplMetadata
{
	GObject parent;

	TeplMetadataPrivate *priv;
};

struct _TeplMetadataClass
{
	GObjectClass parent_class;

	gpointer padding[12];
};

_TEPL_EXTERN
GType		tepl_metadata_get_type		(void);

_TEPL_EXTERN
TeplMetadata *	tepl_metadata_new		(void);

_TEPL_EXTERN
gchar *		tepl_metadata_get		(TeplMetadata *metadata,
						 const gchar  *key);

_TEPL_EXTERN
void		tepl_metadata_set		(TeplMetadata *metadata,
						 const gchar  *key,
						 const gchar  *value);

G_GNUC_INTERNAL
void		_tepl_metadata_foreach		(TeplMetadata *metadata,
						 GHFunc        func,
						 gpointer      user_data);

G_GNUC_INTERNAL
gboolean	_tepl_metadata_key_is_valid	(const gchar *key);

G_GNUC_INTERNAL
gboolean	_tepl_metadata_value_is_valid	(const gchar *value);

G_END_DECLS

#endif /* TEPL_METADATA_H */
