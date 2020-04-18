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

struct _TeplMetadataPrivate
{
	/* Keys: gchar *
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

TeplMetadata *
tepl_metadata_new (void)
{
	return g_object_new (TEPL_TYPE_METADATA, NULL);
}

gchar *
tepl_metadata_get (TeplMetadata *metadata,
		   const gchar  *key)
{
	g_return_val_if_fail (TEPL_IS_METADATA (metadata), NULL);
	g_return_val_if_fail (_tepl_metadata_key_is_valid (key), NULL);

	return g_strdup (g_hash_table_lookup (metadata->priv->hash_table, key));
}

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

static gboolean
key_char_is_valid (gchar ch)
{
	/* TODO update comment.
	 * At the time of writing this, the GIO API doesn't document the
	 * requirements for valid attribute names. See the docs of
	 * g_file_query_info() for example. Clearly '*' and ',' must not be used
	 * because they serve to query several attributes. ':' is used in "::"
	 * to separate the namespace from the attribute name, I'm not sure that
	 * there can be several nested namespaces like in
	 * "metadata::gCSVedit::delimiter"; in case of doubt it's better not to
	 * support it by not allowing ':'.
	 */
	return (g_ascii_isalnum (ch) || ch == '-' || ch == '_');
}

/*
 * _tepl_metadata_key_is_valid:
 * @metadata_key: (nullable): a string, or %NULL.
 *
 * Returns whether @metadata_key is a valid string that can be used as a
 * metadata key when using the Tepl metadata API.
 *
 * It returns %TRUE only if @metadata_key is a non-empty string containing only
 * ASCII alphanumeric characters (see g_ascii_isalnum()), `"-"` (dash) or `"_"`
 * (underscore).
 *
 * Examples of valid metadata keys:
 * - `"gedit-spell-checking-language"`
 * - `"gCSVedit_column_delimiter"`
 *
 * Returns: whether @metadata_key is valid.
 */
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
