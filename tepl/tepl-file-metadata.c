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

#include "tepl-file-metadata.h"
#include "tepl-utils.h"

struct _TeplFileMetadataPrivate
{
	/* Keys: gchar *
	 * Values: gchar *
	 * Never NULL.
	 */
	GHashTable *hash_table;

	/* Time of last access in milliseconds since January 1, 1970 UTC.
	 * Permits to remove the oldest TeplFileMetadata's from the XML file, so
	 * that the XML file doesn't grow indefinitely.
	 */
	gint64 atime;
};

G_DEFINE_TYPE_WITH_PRIVATE (TeplFileMetadata, tepl_file_metadata, G_TYPE_OBJECT)

static void
set_current_atime (TeplFileMetadata *metadata)
{
	metadata->priv->atime = g_get_real_time () / 1000;
}

static void
tepl_file_metadata_finalize (GObject *object)
{
	TeplFileMetadata *metadata = TEPL_FILE_METADATA (object);

	g_hash_table_unref (metadata->priv->hash_table);

	G_OBJECT_CLASS (tepl_file_metadata_parent_class)->finalize (object);
}

static void
tepl_file_metadata_class_init (TeplFileMetadataClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = tepl_file_metadata_finalize;
}

static void
tepl_file_metadata_init (TeplFileMetadata *metadata)
{
	metadata->priv = tepl_file_metadata_get_instance_private (metadata);

	metadata->priv->hash_table = g_hash_table_new_full (g_str_hash,
							    g_str_equal,
							    g_free,
							    g_free);
}

TeplFileMetadata *
tepl_file_metadata_new (void)
{
	return g_object_new (TEPL_TYPE_FILE_METADATA, NULL);
}

gchar *
tepl_file_metadata_get (TeplFileMetadata *metadata,
			const gchar      *key)
{
	g_return_val_if_fail (TEPL_IS_FILE_METADATA (metadata), NULL);
	g_return_val_if_fail (tepl_utils_metadata_key_is_valid (key), NULL);

	set_current_atime (metadata);

	return g_strdup (g_hash_table_lookup (metadata->priv->hash_table, key));
}

void
tepl_file_metadata_set (TeplFileMetadata *metadata,
			const gchar      *key,
			const gchar      *value)
{
	g_return_if_fail (TEPL_IS_FILE_METADATA (metadata));
	g_return_if_fail (tepl_utils_metadata_key_is_valid (key));
	g_return_if_fail (value == NULL || g_utf8_validate (value, -1, NULL));

	set_current_atime (metadata);

	if (value == NULL)
	{
		g_hash_table_remove (metadata->priv->hash_table, key);
	}
	else
	{
		_tepl_file_metadata_insert_entry (metadata, key, value);
	}
}

/* Returns: TRUE on success. */
gboolean
_tepl_file_metadata_set_atime_str (TeplFileMetadata *metadata,
				   const gchar      *atime_str)
{
	g_return_val_if_fail (TEPL_IS_FILE_METADATA (metadata), FALSE);
	g_return_val_if_fail (atime_str != NULL, FALSE);

	return g_ascii_string_to_signed (atime_str,
					 10,
					 0, G_MAXINT64,
					 &metadata->priv->atime,
					 NULL);
}

/* See GCompareFunc. */
gint
_tepl_file_metadata_compare_atime (TeplFileMetadata *metadata1,
				   TeplFileMetadata *metadata2)
{
	g_return_val_if_fail (TEPL_IS_FILE_METADATA (metadata1), 0);
	g_return_val_if_fail (TEPL_IS_FILE_METADATA (metadata2), 0);

	if (metadata1->priv->atime < metadata2->priv->atime)
	{
		return -1;
	}
	else if (metadata1->priv->atime > metadata2->priv->atime)
	{
		return 1;
	}

	return 0;
}

void
_tepl_file_metadata_insert_entry (TeplFileMetadata *metadata,
				  const gchar      *key,
				  const gchar      *value)
{
	g_return_if_fail (TEPL_IS_FILE_METADATA (metadata));
	g_return_if_fail (tepl_utils_metadata_key_is_valid (key));
	g_return_if_fail (g_utf8_validate (value, -1, NULL));

	g_hash_table_replace (metadata->priv->hash_table,
			      g_strdup (key),
			      g_strdup (value));
}

void
_tepl_file_metadata_copy_into (TeplFileMetadata *src_metadata,
			       TeplFileMetadata *dest_metadata)
{
	GHashTableIter iter;
	gpointer key_p;
	gpointer value_p;

	g_return_if_fail (TEPL_IS_FILE_METADATA (src_metadata));
	g_return_if_fail (TEPL_IS_FILE_METADATA (dest_metadata));

	g_hash_table_iter_init (&iter, src_metadata->priv->hash_table);
	while (g_hash_table_iter_next (&iter, &key_p, &value_p))
	{
		const gchar *key = key_p;
		const gchar *value = value_p;

		_tepl_file_metadata_insert_entry (dest_metadata, key, value);
	}
}

static void
append_entries_to_string (TeplFileMetadata *metadata,
			  GString          *string)
{
	GHashTableIter iter;
	gpointer key_p;
	gpointer value_p;

	g_hash_table_iter_init (&iter, metadata->priv->hash_table);
	while (g_hash_table_iter_next (&iter, &key_p, &value_p))
	{
		const gchar *key = key_p;
		const gchar *value = value_p;
		gchar *value_escaped;

		/* No need to escape the key. */

		value_escaped = g_markup_escape_text (value, -1);

		g_string_append_printf (string,
					"  <entry key=\"%s\" value=\"%s\"/>\n",
					key,
					value_escaped);

		g_free (value_escaped);
	}
}

void
_tepl_file_metadata_append_xml_to_string (TeplFileMetadata *metadata,
					  GFile            *location,
					  GString          *string)
{
	gchar *uri;
	gchar *uri_escaped;

	g_return_if_fail (TEPL_IS_FILE_METADATA (metadata));
	g_return_if_fail (G_IS_FILE (location));
	g_return_if_fail (string != NULL);

	if (g_hash_table_size (metadata->priv->hash_table) == 0)
	{
		return;
	}

	uri = g_file_get_uri (location);
	uri_escaped = g_markup_escape_text (uri, -1);

	g_string_append_printf (string,
				" <document uri=\"%s\" atime=\"%" G_GINT64_FORMAT "\">\n",
				uri_escaped,
				metadata->priv->atime);

	append_entries_to_string (metadata, string);

	g_string_append (string, " </document>\n");

	g_free (uri);
	g_free (uri_escaped);
}
