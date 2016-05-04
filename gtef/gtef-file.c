/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "gtef-file.h"

/**
 * SECTION:file
 * @Short_description: On-disk representation of a GtkSourceBuffer
 * @Title: GtefFile
 *
 * #GtefFile extends #GtkSourceFile with metadata support.
 */

typedef struct _GtefFilePrivate GtefFilePrivate;

struct _GtefFilePrivate
{
	/* Never NULL */
	GFileInfo *metadata;
};

G_DEFINE_TYPE_WITH_PRIVATE (GtefFile, gtef_file, GTK_SOURCE_TYPE_FILE)

#define METADATA_PREFIX "metadata::"

static gchar *
get_metadata_attribute_key (const gchar *key)
{
	return g_strconcat (METADATA_PREFIX, key, NULL);
}

static void
gtef_file_finalize (GObject *object)
{
	GtefFilePrivate *priv = gtef_file_get_instance_private (GTEF_FILE (object));

	g_object_unref (priv->metadata);

	G_OBJECT_CLASS (gtef_file_parent_class)->finalize (object);
}

static void
gtef_file_class_init (GtefFileClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = gtef_file_finalize;
}

static void
gtef_file_init (GtefFile *file)
{
	GtefFilePrivate *priv = gtef_file_get_instance_private (file);

	priv->metadata = g_file_info_new ();
}

/**
 * gtef_file_new:
 *
 * Returns: a new #GtefFile object.
 * Since: 1.0
 */
GtefFile *
gtef_file_new (void)
{
	return g_object_new (GTEF_TYPE_FILE, NULL);
}

/**
 * gtef_file_get_metadata:
 * @file: a #GtefFile.
 * @key: the name of the metadata.
 *
 * Gets the value of a metadata stored in the @file object memory.
 *
 * Returns: (nullable): the value of the metadata, or %NULL if the metadata
 *   doesn't exist. Free with g_free().
 * Since: 1.0
 */
gchar *
gtef_file_get_metadata (GtefFile    *file,
			const gchar *key)
{
	GtefFilePrivate *priv;
	gchar *attribute_key;
	gchar *value = NULL;

	g_return_val_if_fail (GTEF_IS_FILE (file), NULL);
	g_return_val_if_fail (key != NULL && key[0] != '\0', NULL);

	priv = gtef_file_get_instance_private (file);

	attribute_key = get_metadata_attribute_key (key);

	if (g_file_info_has_attribute (priv->metadata, attribute_key) &&
	    g_file_info_get_attribute_type (priv->metadata, attribute_key) == G_FILE_ATTRIBUTE_TYPE_STRING)
	{
		value = g_strdup (g_file_info_get_attribute_string (priv->metadata, attribute_key));
	}

	g_free (attribute_key);
	return value;
}

/**
 * gtef_file_set_metadata:
 * @file: a #GtefFile.
 * @key: the name of the metadata.
 * @value: (nullable): the value of the metadata, or %NULL to unset.
 *
 * Sets the value of a metadata. It's preferable that @key starts with a
 * namespace, to not get metadata conflicts between applications.
 *
 * This function just stores the new metadata value in the @file object memory.
 *
 * Since: 1.0
 */
void
gtef_file_set_metadata (GtefFile    *file,
			const gchar *key,
			const gchar *value)
{
	GtefFilePrivate *priv;
	gchar *attribute_key;

	g_return_if_fail (GTEF_IS_FILE (file));
	g_return_if_fail (key != NULL && key[0] != '\0');

	priv = gtef_file_get_instance_private (file);

	attribute_key = get_metadata_attribute_key (key);

	if (value != NULL)
	{
		g_file_info_set_attribute_string (priv->metadata,
						  attribute_key,
						  value);
	}
	else
	{
		/* Unset the key */
		g_file_info_set_attribute (priv->metadata,
					   attribute_key,
					   G_FILE_ATTRIBUTE_TYPE_INVALID,
					   NULL);
	}

	g_free (attribute_key);
}
