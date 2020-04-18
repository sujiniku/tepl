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

#include "tepl-metadata-attic.h"

struct _TeplMetadataAtticPrivate
{
	/* Keys: not nullable gchar *
	 * Values: not nullable gchar *
	 * hash_table never NULL.
	 *
	 * Note that unlike TeplMetadata, here the values must never be NULL.
	 */
	GHashTable *hash_table;

	/* Time of last access in milliseconds since January 1, 1970 UTC.
	 * Useful for tepl_metadata_manager_trim().
	 */
	gint64 atime;
};

G_DEFINE_TYPE_WITH_PRIVATE (TeplMetadataAttic, _tepl_metadata_attic, G_TYPE_OBJECT)

static void
set_current_atime (TeplMetadataAttic *metadata)
{
	metadata->priv->atime = g_get_real_time () / 1000;
}

static void
_tepl_metadata_attic_finalize (GObject *object)
{
	TeplMetadataAttic *metadata = TEPL_METADATA_ATTIC (object);

	g_hash_table_unref (metadata->priv->hash_table);

	G_OBJECT_CLASS (_tepl_metadata_attic_parent_class)->finalize (object);
}

static void
_tepl_metadata_attic_class_init (TeplMetadataAtticClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = _tepl_metadata_attic_finalize;
}

static void
_tepl_metadata_attic_init (TeplMetadataAttic *metadata)
{
	metadata->priv = _tepl_metadata_attic_get_instance_private (metadata);

	metadata->priv->hash_table = g_hash_table_new_full (g_str_hash,
							    g_str_equal,
							    g_free,
							    g_free);
}

TeplMetadataAttic *
_tepl_metadata_attic_new (void)
{
	return g_object_new (TEPL_TYPE_METADATA_ATTIC, NULL);
}

/* Returns: TRUE on success. */
gboolean
_tepl_metadata_attic_set_atime_str (TeplMetadataAttic *metadata,
				    const gchar       *atime_str)
{
	g_return_val_if_fail (TEPL_IS_METADATA_ATTIC (metadata), FALSE);
	g_return_val_if_fail (atime_str != NULL, FALSE);

	return g_ascii_string_to_signed (atime_str,
					 10,
					 0, G_MAXINT64,
					 &metadata->priv->atime,
					 NULL);
}

/* See GCompareFunc. */
gint
_tepl_metadata_attic_compare_atime (TeplMetadataAttic *metadata1,
				    TeplMetadataAttic *metadata2)
{
	g_return_val_if_fail (TEPL_IS_METADATA_ATTIC (metadata1), 0);
	g_return_val_if_fail (TEPL_IS_METADATA_ATTIC (metadata2), 0);

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
_tepl_metadata_attic_insert_entry (TeplMetadataAttic *metadata,
				   const gchar       *key,
				   const gchar       *value)
{
	g_return_if_fail (TEPL_IS_METADATA_ATTIC (metadata));
	g_return_if_fail (_tepl_metadata_key_is_valid (key));
	g_return_if_fail (_tepl_metadata_value_is_valid (value));

	g_hash_table_replace (metadata->priv->hash_table,
			      g_strdup (key),
			      g_strdup (value));
}

static void
append_entries_to_string (TeplMetadataAttic *metadata,
			  GString           *string)
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
_tepl_metadata_attic_append_xml_to_string (TeplMetadataAttic *metadata,
					   GFile             *location,
					   GString           *string)
{
	gchar *uri;
	gchar *uri_escaped;

	g_return_if_fail (TEPL_IS_METADATA_ATTIC (metadata));
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

void
_tepl_metadata_attic_copy_from (TeplMetadataAttic *from_metadata_attic,
				TeplMetadata      *to_metadata)
{
	GHashTableIter iter;
	gpointer key_p;
	gpointer value_p;

	g_return_if_fail (TEPL_IS_METADATA_ATTIC (from_metadata_attic));
	g_return_if_fail (TEPL_IS_METADATA (to_metadata));

	g_hash_table_iter_init (&iter, from_metadata_attic->priv->hash_table);
	while (g_hash_table_iter_next (&iter, &key_p, &value_p))
	{
		const gchar *key = key_p;
		const gchar *value = value_p;

		tepl_metadata_set (to_metadata, key, value);
	}

	set_current_atime (from_metadata_attic);
}

static void
merge_into__from_metadata_foreach_cb (gpointer key_p,
				      gpointer value_p,
				      gpointer user_data)
{
	const gchar *key = key_p;
	const gchar *value = value_p; /* Can be NULL. */
	TeplMetadataAttic *into_metadata_attic = TEPL_METADATA_ATTIC (user_data);

	if (value != NULL)
	{
		_tepl_metadata_attic_insert_entry (into_metadata_attic, key, value);
	}
	else
	{
		/* Unset. */
		g_hash_table_remove (into_metadata_attic->priv->hash_table, key);
	}
}

void
_tepl_metadata_attic_merge_into (TeplMetadataAttic *into_metadata_attic,
				 TeplMetadata      *from_metadata)
{
	g_return_if_fail (TEPL_IS_METADATA_ATTIC (into_metadata_attic));
	g_return_if_fail (TEPL_IS_METADATA (from_metadata));

	_tepl_metadata_foreach (from_metadata,
				merge_into__from_metadata_foreach_cb,
				into_metadata_attic);

	set_current_atime (into_metadata_attic);
}
