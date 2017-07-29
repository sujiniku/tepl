/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2003-2007 - Paolo Maggi
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
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

/**
 * SECTION:metadata-manager
 * @Short_description: Metadata support on platforms that don't support GVfs metadata
 * @Title: TeplMetadataManager
 *
 * The metadata manager permits to save/load metadata on platforms that don't
 * support GVfs metadata, like (at the time of writing) Windows.
 */

/* This code comes from gedit.
 * A better implementation would be to use SQLite.
 */

#include "tepl-metadata-manager.h"
#include <libxml/xmlreader.h>
#include <string.h>

#define MAX_ITEMS 50

typedef struct _TeplMetadataManager TeplMetadataManager;

typedef struct _Item Item;

struct _Item
{
	/* Time of last access in seconds since January 1, 1970 UTC. */
	gint64 atime;

	GHashTable *values;
};

struct _TeplMetadataManager
{
	guint timeout_id;

	GHashTable *items;

	gchar *metadata_path;

	/* It is true if the file has been read. */
	guint values_loaded : 1;

	guint unit_test_mode : 1;
};

static gboolean tepl_metadata_manager_save (gpointer data);

static TeplMetadataManager *tepl_metadata_manager = NULL;

#define METADATA_PREFIX "metadata::"

static gchar *
get_metadata_attribute_key (const gchar *key)
{
	return g_strconcat (METADATA_PREFIX, key, NULL);
}

static void
item_free (gpointer data)
{
	Item *item;

	g_return_if_fail (data != NULL);

	item = (Item *)data;

	if (item->values != NULL)
		g_hash_table_destroy (item->values);

	g_free (item);
}

static void
tepl_metadata_manager_arm_timeout (void)
{
	if (tepl_metadata_manager->unit_test_mode)
	{
		tepl_metadata_manager_save (NULL);
		return;
	}

	if (tepl_metadata_manager->timeout_id == 0)
	{
		tepl_metadata_manager->timeout_id =
			g_timeout_add_seconds_full (G_PRIORITY_DEFAULT_IDLE,
						    2,
						    (GSourceFunc)tepl_metadata_manager_save,
						    NULL,
						    NULL);
	}
}

/**
 * tepl_metadata_manager_init:
 * @metadata_path: the filename where the metadata is stored.
 *
 * This function initializes the metadata manager.
 *
 * The @metadata_path must be different for each process. It is advised for your
 * application to rely on #GApplication process uniqueness.
 *
 * A good place to store the metadata is in a sub-directory of the user data
 * directory. See g_get_user_data_dir().
 *
 * Since: 1.0
 */
void
tepl_metadata_manager_init (const gchar *metadata_path)
{
	if (tepl_metadata_manager != NULL)
	{
		return;
	}

	tepl_metadata_manager = g_new0 (TeplMetadataManager, 1);

	tepl_metadata_manager->values_loaded = FALSE;

	tepl_metadata_manager->items =
		g_hash_table_new_full (g_str_hash,
				       g_str_equal,
				       g_free,
				       item_free);

	tepl_metadata_manager->metadata_path = g_strdup (metadata_path);

	tepl_metadata_manager->unit_test_mode = FALSE;
}

/**
 * tepl_metadata_manager_shutdown:
 *
 * This function saves synchronously metadata if they need to be saved, and
 * frees the internal data of the metadata manager.
 *
 * See also tepl_finalize(), which calls this function.
 *
 * Since: 1.0
 */
void
tepl_metadata_manager_shutdown (void)
{
	if (tepl_metadata_manager == NULL)
		return;

	if (tepl_metadata_manager->timeout_id != 0)
	{
		g_source_remove (tepl_metadata_manager->timeout_id);
		tepl_metadata_manager->timeout_id = 0;
		tepl_metadata_manager_save (NULL);
	}

	if (tepl_metadata_manager->items != NULL)
		g_hash_table_destroy (tepl_metadata_manager->items);

	g_free (tepl_metadata_manager->metadata_path);

	g_free (tepl_metadata_manager);
	tepl_metadata_manager = NULL;
}

static void
parseItem (xmlDocPtr doc, xmlNodePtr cur)
{
	Item *item;

	xmlChar *uri;
	xmlChar *atime;

	if (xmlStrcmp (cur->name, (const xmlChar *)"document") != 0)
			return;

	uri = xmlGetProp (cur, (const xmlChar *)"uri");
	if (uri == NULL)
		return;

	atime = xmlGetProp (cur, (const xmlChar *)"atime");
	if (atime == NULL)
	{
		xmlFree (uri);
		return;
	}

	item = g_new0 (Item, 1);

	item->atime = g_ascii_strtoll ((char *)atime, NULL, 0);

	item->values = g_hash_table_new_full (g_str_hash,
					      g_str_equal,
					      g_free,
					      g_free);

	cur = cur->xmlChildrenNode;

	while (cur != NULL)
	{
		if (xmlStrcmp (cur->name, (const xmlChar *)"entry") == 0)
		{
			xmlChar *key;
			xmlChar *value;

			key = xmlGetProp (cur, (const xmlChar *)"key");
			value = xmlGetProp (cur, (const xmlChar *)"value");

			if ((key != NULL) && (value != NULL))
			{
				g_hash_table_insert (item->values,
						     g_strdup ((gchar *)key),
						     g_strdup ((gchar *)value));
			}

			if (key != NULL)
				xmlFree (key);
			if (value != NULL)
				xmlFree (value);
		}

		cur = cur->next;
	}

	g_hash_table_insert (tepl_metadata_manager->items,
			     g_strdup ((gchar *)uri),
			     item);

	xmlFree (uri);
	xmlFree (atime);
}

/* Returns FALSE in case of error. */
static gboolean
load_values (void)
{
	xmlDocPtr doc;
	xmlNodePtr cur;

	g_return_val_if_fail (tepl_metadata_manager != NULL, FALSE);
	g_return_val_if_fail (tepl_metadata_manager->values_loaded == FALSE, FALSE);

	tepl_metadata_manager->values_loaded = TRUE;

	xmlKeepBlanksDefault (0);

	if (tepl_metadata_manager->metadata_path == NULL)
	{
		return FALSE;
	}

	/* TODO: avoid races */
	if (!g_file_test (tepl_metadata_manager->metadata_path, G_FILE_TEST_EXISTS))
	{
		return TRUE;
	}

	doc = xmlParseFile (tepl_metadata_manager->metadata_path);

	if (doc == NULL)
	{
		return FALSE;
	}

	cur = xmlDocGetRootElement (doc);
	if (cur == NULL)
	{
		g_message ("The metadata file '%s' is empty",
		           g_path_get_basename (tepl_metadata_manager->metadata_path));
		xmlFreeDoc (doc);

		return TRUE;
	}

	if (xmlStrcmp (cur->name, (const xmlChar *) "metadata"))
	{
		g_message ("File '%s' is of the wrong type",
		           g_path_get_basename (tepl_metadata_manager->metadata_path));
		xmlFreeDoc (doc);

		return FALSE;
	}

	cur = xmlDocGetRootElement (doc);
	cur = cur->xmlChildrenNode;

	while (cur != NULL)
	{
		parseItem (doc, cur);

		cur = cur->next;
	}

	xmlFreeDoc (doc);

	return TRUE;
}

static void
save_values (const gchar *key, const gchar *value, xmlNodePtr parent)
{
	xmlNodePtr xml_node;

	g_return_if_fail (key != NULL);

	if (value == NULL)
		return;

	xml_node = xmlNewChild (parent,
				NULL,
				(const xmlChar *)"entry",
				NULL);

	xmlSetProp (xml_node,
		    (const xmlChar *)"key",
		    (const xmlChar *)key);
	xmlSetProp (xml_node,
		    (const xmlChar *)"value",
		    (const xmlChar *)value);
}

static void
save_item (const gchar *key, const gpointer *data, xmlNodePtr parent)
{
	xmlNodePtr xml_node;
	const Item *item = (const Item *)data;
	gchar *atime;

	g_return_if_fail (key != NULL);

	if (item == NULL)
		return;

	xml_node = xmlNewChild (parent, NULL, (const xmlChar *)"document", NULL);

	xmlSetProp (xml_node, (const xmlChar *)"uri", (const xmlChar *)key);

	atime = g_strdup_printf ("%" G_GINT64_FORMAT, item->atime);
	xmlSetProp (xml_node, (const xmlChar *)"atime", (const xmlChar *)atime);

	g_free (atime);

	g_hash_table_foreach (item->values,
			      (GHFunc)save_values,
			      xml_node);
}

static void
get_oldest (const gchar *key, const gpointer value, const gchar ** key_to_remove)
{
	const Item *item = (const Item *)value;

	if (*key_to_remove == NULL)
	{
		*key_to_remove = key;
	}
	else
	{
		const Item *item_to_remove =
			g_hash_table_lookup (tepl_metadata_manager->items,
					     *key_to_remove);

		g_return_if_fail (item_to_remove != NULL);

		if (item->atime < item_to_remove->atime)
		{
			*key_to_remove = key;
		}
	}
}

static void
resize_items (void)
{
	while (g_hash_table_size (tepl_metadata_manager->items) > MAX_ITEMS)
	{
		gpointer key_to_remove = NULL;

		g_hash_table_foreach (tepl_metadata_manager->items,
				      (GHFunc)get_oldest,
				      &key_to_remove);

		g_return_if_fail (key_to_remove != NULL);

		g_hash_table_remove (tepl_metadata_manager->items,
				     key_to_remove);
	}
}

static gboolean
tepl_metadata_manager_save (gpointer data)
{
	xmlDocPtr  doc;
	xmlNodePtr root;

	tepl_metadata_manager->timeout_id = 0;

	resize_items ();

	xmlIndentTreeOutput = TRUE;

	doc = xmlNewDoc ((const xmlChar *)"1.0");
	if (doc == NULL)
		return TRUE;

	/* Create metadata root */
	root = xmlNewDocNode (doc, NULL, (const xmlChar *)"metadata", NULL);
	xmlDocSetRootElement (doc, root);

	g_hash_table_foreach (tepl_metadata_manager->items,
			      (GHFunc)save_item,
			      root);

	/* FIXME: lock file - Paolo */
	if (tepl_metadata_manager->metadata_path != NULL)
	{
		gchar *cache_dir;
		int res;

		/* make sure the cache dir exists */
		cache_dir = g_path_get_dirname (tepl_metadata_manager->metadata_path);
		res = g_mkdir_with_parents (cache_dir, 0755);
		if (res != -1)
		{
			xmlSaveFormatFile (tepl_metadata_manager->metadata_path,
			                   doc,
			                   1);
		}

		g_free (cache_dir);
	}

	xmlFreeDoc (doc);

	return FALSE;
}

static void
foreach_key_cb (gpointer _key,
		gpointer _value,
		gpointer _user_data)
{
	const gchar *key = _key;
	const gchar *value = _value;
	GFileInfo *metadata = G_FILE_INFO (_user_data);
	gchar *attribute_key;

	if (key == NULL || key[0] == '\0')
	{
		return;
	}

	attribute_key = get_metadata_attribute_key (key);
	g_file_info_set_attribute_string (metadata, attribute_key, value);
	g_free (attribute_key);
}

/* Returns: (transfer full) (nullable) */
GFileInfo *
_tepl_metadata_manager_get_all_metadata_for_location (GFile *location)
{
	gchar *uri;
	Item *item;
	GFileInfo *metadata;

	g_return_val_if_fail (G_IS_FILE (location), NULL);

	if (!tepl_metadata_manager->values_loaded)
	{
		gboolean ok;

		ok = load_values ();

		if (!ok)
		{
			return NULL;
		}
	}

	uri = g_file_get_uri (location);
	item = g_hash_table_lookup (tepl_metadata_manager->items, uri);
	g_free (uri);

	if (item == NULL)
	{
		return NULL;
	}

	item->atime = g_get_real_time () / 1000;

	if (item->values == NULL)
	{
		return NULL;
	}

	metadata = g_file_info_new ();

	g_hash_table_foreach (item->values, foreach_key_cb, metadata);

	return metadata;
}

void
_tepl_metadata_manager_set_metadata_for_location (GFile     *location,
						  GFileInfo *metadata)
{
	gchar **attributes_list;
	gchar *uri;
	Item *item;
	gint i;

	g_return_if_fail (G_IS_FILE (location));
	g_return_if_fail (G_IS_FILE_INFO (metadata));

	if (!tepl_metadata_manager->values_loaded)
	{
		gboolean ok;

		ok = load_values ();

		if (!ok)
		{
			return;
		}
	}

	attributes_list = g_file_info_list_attributes (metadata, "metadata");
	if (attributes_list == NULL || attributes_list[0] == NULL)
	{
		g_strfreev (attributes_list);
		return;
	}

	uri = g_file_get_uri (location);

	item = g_hash_table_lookup (tepl_metadata_manager->items, uri);

	if (item == NULL)
	{
		item = g_new0 (Item, 1);

		g_hash_table_insert (tepl_metadata_manager->items,
				     g_strdup (uri),
				     item);
	}

	if (item->values == NULL)
	{
		item->values = g_hash_table_new_full (g_str_hash,
						      g_str_equal,
						      g_free,
						      g_free);
	}

	for (i = 0; attributes_list[i] != NULL; i++)
	{
		const gchar *attribute_key;
		const gchar *key;
		const gchar *value = NULL;

		attribute_key = attributes_list[i];
		if (!g_str_has_prefix (attribute_key, METADATA_PREFIX))
		{
			g_warning ("Metadata attribute key '%s' doesn't have '" METADATA_PREFIX "' prefix.",
				   attribute_key);
			continue;
		}

		key = attribute_key + strlen (METADATA_PREFIX);

		if (g_file_info_get_attribute_type (metadata, attribute_key) == G_FILE_ATTRIBUTE_TYPE_STRING)
		{
			value = g_file_info_get_attribute_string (metadata, attribute_key);
		}

		if (value != NULL)
		{
			g_hash_table_insert (item->values,
					     g_strdup (key),
					     g_strdup (value));
		}
		else
		{
			g_hash_table_remove (item->values, key);
		}
	}

	item->atime = g_get_real_time () / 1000;

	g_strfreev (attributes_list);
	g_free (uri);

	tepl_metadata_manager_arm_timeout ();
}

void
_tepl_metadata_manager_set_unit_test_mode (void)
{
	tepl_metadata_manager->unit_test_mode = TRUE;

	if (tepl_metadata_manager->timeout_id != 0)
	{
		g_source_remove (tepl_metadata_manager->timeout_id);
		tepl_metadata_manager->timeout_id = 0;
		tepl_metadata_manager_save (NULL);
	}
}
