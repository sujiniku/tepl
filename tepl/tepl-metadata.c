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

#include "tepl-metadata.h"

/**
 * SECTION:metadata
 * @Title: TeplMetadata
 * @Short_description: File metadata
 *
 * A #TeplMetadata object contains a set of file metadata as key/value pairs.
 *
 * The tepl_metadata_get() and tepl_metadata_set() functions don't load or save
 * the metadata on disk, they only access the metadata stored in the
 * #TeplMetadata object memory.
 *
 * #TeplMetadata is intended to be used alongside #TeplMetadataManager to load
 * and store the metadata on disk.
 *
 * # Values requirements
 *
 * Values must be nul-terminated UTF-8 strings.
 *
 * # Keys requirements # {#tepl-metadata-keys-requirements}
 *
 * Keys must be non-empty strings containing only:
 * - ASCII alphanumeric characters (see g_ascii_isalnum());
 * - `'-'` characters (dashes);
 * - or `'_'` characters (underscores).
 *
 * Additionally, it's preferable that keys start with a namespace, to not get
 * metadata conflicts between the application and libraries.
 *
 * Examples of valid metadata keys:
 * - `"gedit-spell-checking-language"`
 * - `"gCSVedit_column_delimiter"`
 * - `"tepl-character-encoding"`
 */

struct _TeplMetadataPrivate
{
	/* Keys: not nullable gchar *
	 * Values: nullable gchar *
	 * hash_table never NULL.
	 */
	GHashTable *hash_table;
};

G_DEFINE_TYPE_WITH_PRIVATE (TeplMetadata, tepl_metadata, G_TYPE_OBJECT)

static void
tepl_metadata_finalize (GObject *object)
{
	TeplMetadata *metadata = TEPL_METADATA (object);

	g_hash_table_unref (metadata->priv->hash_table);

	G_OBJECT_CLASS (tepl_metadata_parent_class)->finalize (object);
}

static void
tepl_metadata_class_init (TeplMetadataClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = tepl_metadata_finalize;
}

static void
tepl_metadata_init (TeplMetadata *metadata)
{
	metadata->priv = tepl_metadata_get_instance_private (metadata);

	metadata->priv->hash_table = g_hash_table_new_full (g_str_hash,
							    g_str_equal,
							    g_free,
							    g_free);
}

/**
 * tepl_metadata_new:
 *
 * Returns: a new, empty #TeplMetadata object.
 * Since: 5.0
 */
TeplMetadata *
tepl_metadata_new (void)
{
	return g_object_new (TEPL_TYPE_METADATA, NULL);
}

/**
 * tepl_metadata_get:
 * @metadata: a #TeplMetadata.
 * @key: a key.
 *
 * Gets the value of a metadata stored in the @metadata object memory.
 *
 * @key must follow [the requirements explained in the class
 * description][tepl-metadata-keys-requirements].
 *
 * Returns: (transfer full) (nullable): the associated value (a UTF-8 string),
 * or %NULL. Free with g_free() when no longer needed.
 * Since: 5.0
 */
gchar *
tepl_metadata_get (TeplMetadata *metadata,
		   const gchar  *key)
{
	g_return_val_if_fail (TEPL_IS_METADATA (metadata), NULL);
	g_return_val_if_fail (_tepl_metadata_key_is_valid (key), NULL);

	return g_strdup (g_hash_table_lookup (metadata->priv->hash_table, key));
}

/**
 * tepl_metadata_set:
 * @metadata: a #TeplMetadata.
 * @key: a key.
 * @value: (nullable): a nul-terminated UTF-8 string, or %NULL to unset the key.
 *
 * Sets or unsets @key. This function just stores the new metadata value in the
 * @metadata object memory.
 *
 * @key must follow [the requirements explained in the class
 * description][tepl-metadata-keys-requirements].
 *
 * Since: 5.0
 */
void
tepl_metadata_set (TeplMetadata *metadata,
		   const gchar  *key,
		   const gchar  *value)
{
	g_return_if_fail (TEPL_IS_METADATA (metadata));
	g_return_if_fail (_tepl_metadata_key_is_valid (key));
	g_return_if_fail (value == NULL || g_utf8_validate (value, -1, NULL));

	g_hash_table_replace (metadata->priv->hash_table,
			      g_strdup (key),
			      g_strdup (value));
}

void
_tepl_metadata_foreach (TeplMetadata *metadata,
			GHFunc        func,
			gpointer      user_data)
{
	g_return_if_fail (TEPL_IS_METADATA (metadata));

	g_hash_table_foreach (metadata->priv->hash_table, func, user_data);
}

static gboolean
key_char_is_valid (gchar ch)
{
	/* The original intention was to use the "metadata" namespace of the
	 * GFileInfo API (to use GVfs metadata).
	 *
	 * At the time of writing this, the GIO API doesn't document the
	 * requirements for valid attribute names. See the docs of
	 * g_file_query_info() for example. Clearly '*' and ',' must not be used
	 * because they serve to query several attributes. ':' is used in "::"
	 * to separate the namespace from the attribute name, I'm not sure that
	 * there can be several nested namespaces like in
	 * "metadata::gCSVedit::delimiter"; in case of doubt it's better not to
	 * support it by not allowing ':'.
	 *
	 * All in all, the following code seems like good requirements even
	 * though GVfs metadata is not used.
	 */
	return (g_ascii_isalnum (ch) || ch == '-' || ch == '_');
}

gboolean
_tepl_metadata_key_is_valid (const gchar *key)
{
	const gchar *p;

	if (key == NULL || key[0] == '\0')
	{
		return FALSE;
	}

	for (p = key; *p != '\0'; p++)
	{
		gchar cur_char = *p;

		if (!key_char_is_valid (cur_char))
		{
			return FALSE;
		}
	}

	return TRUE;
}
