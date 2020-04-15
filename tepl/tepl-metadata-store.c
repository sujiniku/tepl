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

#include "config.h"
#include "tepl-metadata-store.h"
#include <glib/gi18n-lib.h>
#include "tepl-utils.h"

/**
 * SECTION:metadata-store
 * @Short_description: Metadata support on platforms that don't support GVfs metadata
 * @Title: TeplMetadataStore
 *
 * #TeplMetadataStore is a singleton class that permits to save/load metadata on
 * platforms that don't support GVfs metadata, like (at the time of writing)
 * Windows.
 *
 * If GVfs metadata is supported, it's better to use it instead of
 * #TeplMetadataStore because metadata stored with GVfs can be shared between
 * several applications, thanks to its daemon architecture (a possible use case
 * is to share important information such as the character encoding of text
 * files). With #TeplMetadataStore the metadata is not shareable between
 * applications, see tepl_metadata_store_set_store_file().
 */

/* This code is inspired by TeplMetadataManager, which was itself a modified
 * version of GeditMetadataManager, coming from gedit:
 *
 * Copyright 2003-2007 - Paolo Maggi
 *
 * The XML format is the same. TeplMetadataStore can read a file generated by
 * TeplMetadataManager; but the reverse is maybe not true, it hasn't been
 * tested. Not tested either with the older GeditMetadataManager.
 *
 * A better implementation would be to use a database, so that several processes
 * can read and write to it at the same time, to be able to share metadata
 * between apps.
 *
 * The best would be to contribute to GIO, so that the GFileInfo metadata API
 * can be used on any platform (but it would need to support any kind of
 * metadata type for values, not just strings).
 */

typedef struct _DocumentMetadata DocumentMetadata;
struct _DocumentMetadata
{
	GFileInfo *entries;

	/* Time of last access in milliseconds since January 1, 1970 UTC.
	 * Permits to remove the oldest DocumentMetadata's from the XML file, so
	 * that the XML file doesn't grow indefinitely.
	 */
	gint64 atime;
};

struct _TeplMetadataStorePrivate
{
	/* The XML file where all the metadata are stored. Format example:
	 *
	 * <metadata>
	 *   <document uri="..." atime="...">
	 *     <entry key="..." value="..." />
	 *     <entry key="..." value="..." />
	 *   </document>
	 *   <document uri="..." atime="...">
	 *     <entry key="..." value="..." />
	 *   </document>
	 * </metadata>
	 */
	GFile *xml_file;

	/* Keys: GFile* (corresponds to the document uri in the XML file).
	 * Values: DocumentMetadata*
	 */
	GHashTable *hash_table;

	guint max_number_of_locations;

	guint is_loaded : 1;
	guint modified : 1;
};

/* Data structure used during the XML file parsing. */
typedef struct _ParsingData ParsingData;
struct _ParsingData
{
	TeplMetadataStore *store;

	gchar *cur_document_uri;
	DocumentMetadata *cur_document_metadata;

	guint metadata_element_open : 1;
	guint document_element_open : 1;
};

enum
{
	PROP_0,
	PROP_LOADED,
	N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

/* TeplMetadataStore is a singleton. */
static TeplMetadataStore *singleton = NULL;

#define DEFAULT_MAX_NUMBER_OF_LOCATIONS (10000)

#define METADATA_PREFIX "metadata::"
#define METADATA_PREFIX_LENGTH (10) /* strlen (METADATA_PREFIX); */

G_DEFINE_TYPE_WITH_PRIVATE (TeplMetadataStore, tepl_metadata_store, G_TYPE_OBJECT)

static DocumentMetadata *
document_metadata_new (void)
{
	return g_new0 (DocumentMetadata, 1);
}

/* Returns: TRUE on success. */
static gboolean
document_metadata_set_atime_str (DocumentMetadata *document_metadata,
				 const gchar      *atime_str)
{
	return g_ascii_string_to_signed (atime_str,
					 10,
					 0, G_MAXINT64,
					 &document_metadata->atime,
					 NULL);
}

static void
document_metadata_set_current_atime (DocumentMetadata *document_metadata)
{
	document_metadata->atime = g_get_real_time () / 1000;
}

static void
document_metadata_free (DocumentMetadata *document_metadata)
{
	if (document_metadata != NULL)
	{
		g_clear_object (&document_metadata->entries);
		g_free (document_metadata);
	}
}

static ParsingData *
parsing_data_new (TeplMetadataStore *store)
{
	ParsingData *parsing_data;

	parsing_data = g_new0 (ParsingData, 1);
	parsing_data->store = g_object_ref (store);

	return parsing_data;
}

static void
parsing_data_check_invariants (ParsingData *parsing_data)
{
	if (!parsing_data->metadata_element_open)
	{
		g_assert (!parsing_data->document_element_open);
		g_assert (parsing_data->cur_document_uri == NULL);
		g_assert (parsing_data->cur_document_metadata == NULL);
		return;
	}

	if (!parsing_data->document_element_open)
	{
		g_assert (parsing_data->cur_document_uri == NULL);
		g_assert (parsing_data->cur_document_metadata == NULL);
		return;
	}

	g_assert (parsing_data->cur_document_uri != NULL);
	g_assert (parsing_data->cur_document_metadata != NULL);
}

static void
parsing_data_free (ParsingData *parsing_data)
{
	if (parsing_data != NULL)
	{
		g_object_unref (parsing_data->store);
		g_free (parsing_data->cur_document_uri);
		document_metadata_free (parsing_data->cur_document_metadata);

		g_free (parsing_data);
	}
}

static void
tepl_metadata_store_get_property (GObject    *object,
				  guint       prop_id,
				  GValue     *value,
				  GParamSpec *pspec)
{
	TeplMetadataStore *store = TEPL_METADATA_STORE (object);

	switch (prop_id)
	{
		case PROP_LOADED:
			g_value_set_boolean (value, tepl_metadata_store_is_loaded (store));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_metadata_store_finalize (GObject *object)
{
	TeplMetadataStore *store = TEPL_METADATA_STORE (object);

	if (singleton == store)
	{
		singleton = NULL;
	}

	g_clear_object (&store->priv->xml_file);
	g_hash_table_unref (store->priv->hash_table);

	G_OBJECT_CLASS (tepl_metadata_store_parent_class)->finalize (object);
}

static void
tepl_metadata_store_class_init (TeplMetadataStoreClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = tepl_metadata_store_get_property;
	object_class->finalize = tepl_metadata_store_finalize;

	/**
	 * TeplMetadataStore:loaded:
	 *
	 * %TRUE when the metadata has been loaded, or when there has been at
	 * least an attempt to load it (i.e. when
	 * tepl_metadata_store_load_finish() has been called).
	 *
	 * %FALSE otherwise.
	 *
	 * Since: 5.0
	 */
	properties[PROP_LOADED] =
		g_param_spec_boolean ("loaded",
				      "Loaded",
				      "",
				      FALSE,
				      G_PARAM_READABLE |
				      G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
tepl_metadata_store_init (TeplMetadataStore *store)
{
	store->priv = tepl_metadata_store_get_instance_private (store);

	store->priv->hash_table = g_hash_table_new_full (g_file_hash,
							 (GEqualFunc) g_file_equal,
							 g_object_unref,
							 (GDestroyNotify) document_metadata_free);

	store->priv->max_number_of_locations = DEFAULT_MAX_NUMBER_OF_LOCATIONS;
}

/**
 * tepl_metadata_store_get_singleton:
 *
 * Returns: (transfer none): the #TeplMetadataStore singleton instance.
 * Since: 5.0
 */
TeplMetadataStore *
tepl_metadata_store_get_singleton (void)
{
	if (singleton == NULL)
	{
		singleton = g_object_new (TEPL_TYPE_METADATA_STORE, NULL);
	}

	return singleton;
}

void
_tepl_metadata_store_unref_singleton (void)
{
	if (singleton != NULL)
	{
		g_object_unref (singleton);
	}

	/* singleton is not set to NULL here, it is set to NULL in
	 * tepl_metadata_store_finalize() (i.e. when we are sure that the ref
	 * count reaches 0).
	 */
}

/**
 * tepl_metadata_store_set_store_file:
 * @store: the #TeplMetadataStore.
 * @store_file: the #GFile where the metadata is or will be stored.
 *
 * The @store_file must be different for each process. It is advised for your
 * application to rely on #GApplication process uniqueness.
 *
 * A good place to store the metadata is in a sub-directory of the user data
 * directory. See g_get_user_data_dir().
 *
 * Note that this function does no I/O. To load the metadata from the
 * @store_file, call tepl_metadata_store_load_async(). To save the metadata,
 * call tepl_metadata_store_save().
 *
 * Since: 5.0
 */
void
tepl_metadata_store_set_store_file (TeplMetadataStore *store,
				    GFile             *store_file)
{
	g_return_if_fail (TEPL_IS_METADATA_STORE (store));
	g_return_if_fail (G_IS_FILE (store_file));

	g_set_object (&store->priv->xml_file, store_file);
}

/**
 * tepl_metadata_store_set_max_number_of_locations:
 * @store: the #TeplMetadataStore.
 * @max_number_of_locations: the maximum size.
 *
 * If you don't call this function, a default internal value is used that should
 * fit most applications' needs.
 *
 * The purpose of having a maximum size is to avoid the store file (as set with
 * tepl_metadata_store_set_store_file()) to grow indefinitely.
 *
 * @max_number_of_locations is the maximum number of #GFile locations for which
 * metadata are written to the store file. See
 * tepl_metadata_store_set_metadata_for_location() (this sets the metadata for
 * _one_ location).
 *
 * Upon saving, the #TeplMetadataStore discards the least recently accessed
 * metadata if needed.
 *
 * Since: 5.0
 */
void
tepl_metadata_store_set_max_number_of_locations (TeplMetadataStore *store,
						 guint              max_number_of_locations)
{
	g_return_if_fail (TEPL_IS_METADATA_STORE (store));

	store->priv->max_number_of_locations = max_number_of_locations;
}

/* <metadata> */
static void
parse_metadata_element (GMarkupParseContext  *context,
			const gchar          *element_name,
			ParsingData          *parsing_data,
			GError              **error)
{
	g_assert (!parsing_data->metadata_element_open);

	if (!g_str_equal (element_name, "metadata"))
	{
		g_set_error (error,
			     G_MARKUP_ERROR,
			     G_MARKUP_ERROR_INVALID_CONTENT,
			     /* Translators: do not translate <metadata>. */
			     _("The XML file must start with a <metadata> element, not “%s”."),
			     element_name);
		return;
	}

	parsing_data->metadata_element_open = TRUE;
}

/* <document uri="..." atime="..."> */
static void
parse_document_element (GMarkupParseContext  *context,
			const gchar          *element_name,
			const gchar         **attribute_names,
			const gchar         **attribute_values,
			ParsingData          *parsing_data,
			GError              **error)
{
	gboolean got_uri = FALSE;
	gboolean got_atime = FALSE;
	gint attr_num;

	g_assert (parsing_data->metadata_element_open);
	g_assert (!parsing_data->document_element_open);
	g_assert (parsing_data->cur_document_uri == NULL);
	g_assert (parsing_data->cur_document_metadata == NULL);

	if (!g_str_equal (element_name, "document"))
	{
		g_set_error (error,
			     G_MARKUP_ERROR,
			     G_MARKUP_ERROR_INVALID_CONTENT,
			     /* Translators: do not translate <document>. */
			     _("Expected a <document> element, got “%s” instead."),
			     element_name);
		return;
	}

	parsing_data->cur_document_metadata = document_metadata_new ();

	for (attr_num = 0; attribute_names[attr_num] != NULL; attr_num++)
	{
		const gchar *cur_attr_name = attribute_names[attr_num];
		const gchar *cur_attr_value = attribute_values[attr_num];

		if (!got_uri && g_str_equal (cur_attr_name, "uri"))
		{
			parsing_data->cur_document_uri = g_strdup (cur_attr_value);
			got_uri = TRUE;
		}
		else if (!got_atime && g_str_equal (cur_attr_name, "atime"))
		{
			if (!document_metadata_set_atime_str (parsing_data->cur_document_metadata,
							      cur_attr_value))
			{
				g_set_error (error,
					     G_MARKUP_ERROR,
					     G_MARKUP_ERROR_INVALID_CONTENT,
					     /* Translators: do not translate “atime”. */
					     _("Failed to parse the “atime” attribute value “%s”."),
					     cur_attr_value);
				return;
			}

			got_atime = TRUE;
		}
	}

	if (!got_uri || !got_atime)
	{
		g_set_error_literal (error,
				     G_MARKUP_ERROR,
				     G_MARKUP_ERROR_MISSING_ATTRIBUTE,
				     /* Translators: do not translate <document>, “uri” and “atime”. */
				     _("The <document> element must contain the “uri” and “atime” attributes."));
	}

	parsing_data->document_element_open = TRUE;
}

static void
insert_entry_to_current_document (ParsingData *parsing_data,
				  const gchar *key,
				  const gchar *value)
{
	gchar *attribute_key;

	if (key[0] == '\0')
	{
		return;
	}

	if (parsing_data->cur_document_metadata->entries == NULL)
	{
		/* Lazy creation of the GFileInfo, so if it is not created, we
		 * know that there was no <entry>.
		 */
		parsing_data->cur_document_metadata->entries = g_file_info_new ();
	}

	attribute_key = g_strconcat (METADATA_PREFIX, key, NULL);
	g_file_info_set_attribute_string (parsing_data->cur_document_metadata->entries,
					  attribute_key, value);
	g_free (attribute_key);
}

/* <entry key="..." value="..." /> */
static void
parse_entry_element (GMarkupParseContext  *context,
		     const gchar          *element_name,
		     const gchar         **attribute_names,
		     const gchar         **attribute_values,
		     ParsingData          *parsing_data,
		     GError              **error)
{
	const gchar *key = NULL;
	const gchar *value = NULL;
	gint attr_num;

	g_assert (parsing_data->metadata_element_open);
	g_assert (parsing_data->document_element_open);
	g_assert (parsing_data->cur_document_metadata != NULL);

	if (!g_str_equal (element_name, "entry"))
	{
		g_set_error (error,
			     G_MARKUP_ERROR,
			     G_MARKUP_ERROR_INVALID_CONTENT,
			     /* Translators: do not translate <entry>. */
			     _("Expected an <entry> element, got “%s” instead."),
			     element_name);
		return;
	}

	for (attr_num = 0; attribute_names[attr_num] != NULL; attr_num++)
	{
		const gchar *cur_attr_name = attribute_names[attr_num];
		const gchar *cur_attr_value = attribute_values[attr_num];

		if (key == NULL && g_str_equal (cur_attr_name, "key"))
		{
			key = cur_attr_value;
		}
		else if (value == NULL && g_str_equal (cur_attr_name, "value"))
		{
			value = cur_attr_value;
		}
	}

	if (key == NULL || value == NULL)
	{
		g_set_error_literal (error,
				     G_MARKUP_ERROR,
				     G_MARKUP_ERROR_MISSING_ATTRIBUTE,
				     /* Translators: do not translate <entry>, “key” and “value”. */
				     _("The <entry> element must contain the “key” and “value” attributes."));
		return;
	}

	insert_entry_to_current_document (parsing_data, key, value);
}

static void
parser_start_element_cb (GMarkupParseContext  *context,
			 const gchar          *element_name,
			 const gchar         **attribute_names,
			 const gchar         **attribute_values,
			 gpointer              user_data,
			 GError              **error)
{
	ParsingData *parsing_data = user_data;

	g_return_if_fail (element_name != NULL);

	parsing_data_check_invariants (parsing_data);

	/* <metadata> */
	if (!parsing_data->metadata_element_open)
	{
		parse_metadata_element (context,
					element_name,
					parsing_data,
					error);
		return;
	}

	/* <document uri="..." atime="..."> */
	if (!parsing_data->document_element_open)
	{
		parse_document_element (context,
					element_name,
					attribute_names,
					attribute_values,
					parsing_data,
					error);
		return;
	}

	/* <entry key="..." value="..." /> */
	parse_entry_element (context,
			     element_name,
			     attribute_names,
			     attribute_values,
			     parsing_data,
			     error);
}

static void
insert_document_to_hash_table (ParsingData *parsing_data)
{
	g_assert (parsing_data->document_element_open);
	parsing_data_check_invariants (parsing_data);

	if (parsing_data->cur_document_metadata->entries != NULL)
	{
		g_hash_table_replace (parsing_data->store->priv->hash_table,
				      g_file_new_for_uri (parsing_data->cur_document_uri),
				      parsing_data->cur_document_metadata);
		parsing_data->cur_document_metadata = NULL;
	}
	else
	{
		/* No entries, empty, do not store it. */
		document_metadata_free (parsing_data->cur_document_metadata);
		parsing_data->cur_document_metadata = NULL;
	}

	g_free (parsing_data->cur_document_uri);
	parsing_data->cur_document_uri = NULL;

	parsing_data->document_element_open = FALSE;
}

static void
parser_end_element_cb (GMarkupParseContext  *context,
		       const gchar          *element_name,
		       gpointer              user_data,
		       GError              **error)
{
	ParsingData *parsing_data = user_data;

	g_return_if_fail (element_name != NULL);

	/* </document> */
	if (g_str_equal (element_name, "document"))
	{
		g_return_if_fail (parsing_data->document_element_open);
		insert_document_to_hash_table (parsing_data);
	}
}

static void
parse_xml_file_content (GTask   *task,
			GBytes  *xml_file_bytes,
			GError **error)
{
	TeplMetadataStore *store = TEPL_METADATA_STORE (g_task_get_source_object (task));
	GMarkupParser parser = { parser_start_element_cb, parser_end_element_cb, NULL, NULL, NULL };
	GMarkupParseContext *parse_context;
	ParsingData *parsing_data;
	gboolean ok;

	parsing_data = parsing_data_new (store);
	parse_context = g_markup_parse_context_new (&parser, 0, parsing_data, NULL);
	ok = g_markup_parse_context_parse (parse_context,
					   g_bytes_get_data (xml_file_bytes, NULL),
					   g_bytes_get_size (xml_file_bytes),
					   error);
	if (!ok)
	{
		goto out;
	}

	g_markup_parse_context_end_parse (parse_context, error);

out:
	g_markup_parse_context_free (parse_context);
	parsing_data_free (parsing_data);
}

static void
load_xml_file_cb (GObject      *source_object,
		  GAsyncResult *result,
		  gpointer      user_data)
{
	GFile *xml_file = G_FILE (source_object);
	GTask *task = G_TASK (user_data);
	GBytes *xml_file_bytes;
	GError *error = NULL;

	xml_file_bytes = g_file_load_bytes_finish (xml_file, result, NULL, &error);

	/* If the XML file has not yet been created, e.g. on the first run of
	 * the application. The store is empty.
	 */
	if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND))
	{
		g_task_return_boolean (task, TRUE);
		g_object_unref (task);
		g_clear_error (&error);
		goto out;
	}

	if (error != NULL)
	{
		g_task_return_error (task, error);
		g_object_unref (task);
		goto out;
	}

	parse_xml_file_content (task, xml_file_bytes, &error);
	if (error != NULL)
	{
		g_task_return_error (task, error);
		g_object_unref (task);
		goto out;
	}

	g_task_return_boolean (task, TRUE);
	g_object_unref (task);

out:
	g_bytes_unref (xml_file_bytes);
}

/**
 * tepl_metadata_store_load_async:
 * @store: the #TeplMetadataStore.
 * @io_priority: the I/O priority of the request. E.g. %G_PRIORITY_LOW,
 *   %G_PRIORITY_DEFAULT or %G_PRIORITY_HIGH.
 * @cancellable: (nullable): optional #GCancellable object, %NULL to ignore.
 * @callback: (scope async): a #GAsyncReadyCallback to call when the request is
 *   satisfied.
 * @user_data: user data to pass to @callback.
 *
 * Loads asynchronously the content of the store file. You need to call
 * tepl_metadata_store_set_store_file() before.
 *
 * You can call this function only once. Once the #TeplMetadataStore is loaded
 * it cannot be loaded a second time. A good moment to call this function is on
 * application startup, see the #GApplication::startup signal.
 *
 * See the #GAsyncResult documentation to know how to use this function.
 *
 * Since: 5.0
 */
void
tepl_metadata_store_load_async (TeplMetadataStore   *store,
				gint                 io_priority,
				GCancellable        *cancellable,
				GAsyncReadyCallback  callback,
				gpointer             user_data)
{
	GTask *task;

	g_return_if_fail (TEPL_IS_METADATA_STORE (store));
	g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));
	g_return_if_fail (!tepl_metadata_store_is_loaded (store));
	g_return_if_fail (store->priv->xml_file != NULL);

	task = g_task_new (store, cancellable, callback, user_data);
	g_task_set_priority (task, io_priority);

	/* Note that the priority isn't used here...
	 * If needed the code can be improved to use the priority.
	 */
	g_file_load_bytes_async (store->priv->xml_file,
				 cancellable,
				 load_xml_file_cb,
				 task);
}

/**
 * tepl_metadata_store_load_finish:
 * @store: the #TeplMetadataStore.
 * @result: a #GAsyncResult.
 * @error: location to a %NULL #GError, or %NULL.
 *
 * Finishes the metadata loading started with tepl_metadata_store_load_async().
 *
 * Regardless of whether the operation was successful or not, calling this
 * function sets the #TeplMetadataStore:loaded property to %TRUE.
 *
 * Returns: whether the metadata was loaded successfully.
 * Since: 5.0
 */
gboolean
tepl_metadata_store_load_finish (TeplMetadataStore  *store,
				 GAsyncResult       *result,
				 GError            **error)
{
	g_return_val_if_fail (TEPL_IS_METADATA_STORE (store), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail (g_task_is_valid (result, store), FALSE);
	g_return_val_if_fail (!tepl_metadata_store_is_loaded (store), FALSE);

	store->priv->is_loaded = TRUE;
	g_object_notify_by_pspec (G_OBJECT (store), properties[PROP_LOADED]);

	return g_task_propagate_boolean (G_TASK (result), error);
}

/**
 * tepl_metadata_store_is_loaded:
 * @store: the #TeplMetadataStore.
 *
 * Returns: the value of the #TeplMetadataStore:loaded property.
 * Since: 5.0
 */
gboolean
tepl_metadata_store_is_loaded (TeplMetadataStore *store)
{
	g_return_val_if_fail (TEPL_IS_METADATA_STORE (store), FALSE);

	return store->priv->is_loaded;
}

static void
entries_to_string (DocumentMetadata *document_metadata,
		   GString          *string)
{
	gchar **attrs;
	gint attr_num;

	attrs = g_file_info_list_attributes (document_metadata->entries, "metadata");
	if (attrs == NULL || attrs[0] == NULL)
	{
		goto out;
	}

	for (attr_num = 0; attrs[attr_num] != NULL; attr_num++)
	{
		const gchar *attr_key = attrs[attr_num];
		const gchar *key;
		const gchar *value = NULL;
		GFileAttributeType attr_type;
		gchar *key_escaped;
		gchar *value_escaped;

		if (!g_str_has_prefix (attr_key, METADATA_PREFIX))
		{
			g_warning ("Metadata attribute key '%s' doesn't have '" METADATA_PREFIX "' prefix.",
				   attr_key);
			continue;
		}

		key = attr_key + METADATA_PREFIX_LENGTH;
		if (key[0] == '\0')
		{
			continue;
		}

		attr_type = g_file_info_get_attribute_type (document_metadata->entries, attr_key);
		if (attr_type == G_FILE_ATTRIBUTE_TYPE_STRING)
		{
			value = g_file_info_get_attribute_string (document_metadata->entries, attr_key);
		}

		if (value == NULL)
		{
			continue;
		}

		key_escaped = g_markup_escape_text (key, -1);
		value_escaped = g_markup_escape_text (value, -1);

		g_string_append_printf (string,
					"    <entry key=\"%s\" value=\"%s\" />\n",
					key_escaped,
					value_escaped);

		g_free (key_escaped);
		g_free (value_escaped);
	}

out:
	g_strfreev (attrs);
}

static void
document_to_string_cb (gpointer key,
		       gpointer value,
		       gpointer user_data)
{
	GFile *location = G_FILE (key);
	DocumentMetadata *document_metadata = value;
	GString *string = user_data;
	GString *entries_string;
	gchar *entries_str;
	gchar *uri;
	gchar *uri_escaped;

	g_return_if_fail (document_metadata != NULL);
	g_return_if_fail (document_metadata->entries != NULL);

	entries_string = g_string_new (NULL);
	entries_to_string (document_metadata, entries_string);
	entries_str = g_string_free (entries_string, FALSE);

	if (entries_str == NULL || entries_str[0] == '\0')
	{
		/* No valid entries, no need to write the <document>. */
		g_free (entries_str);
		return;
	}

	uri = g_file_get_uri (location);
	uri_escaped = g_markup_escape_text (uri, -1);

	g_string_append_printf (string,
				"  <document uri=\"%s\" atime=\"%" G_GINT64_FORMAT "\">\n",
				uri_escaped,
				document_metadata->atime);
	g_string_append (string, entries_str);
	g_string_append (string, "  </document>\n");

	g_free (entries_str);
	g_free (uri);
	g_free (uri_escaped);
}

static GBytes *
to_string (TeplMetadataStore *store)
{
	GString *string;

	string = g_string_new (NULL);
	g_string_append (string, "<metadata>\n");

	g_hash_table_foreach (store->priv->hash_table,
			      document_to_string_cb,
			      string);

	g_string_append (string, "</metadata>\n");

	return g_string_free_to_bytes (string);
}

static void
resize_hash_table_according_to_max_number_of_locations (TeplMetadataStore *store)
{
	while (g_hash_table_size (store->priv->hash_table) > store->priv->max_number_of_locations)
	{
		GHashTableIter iter;
		gpointer key;
		gpointer value;
		GFile *oldest_location = NULL;
		DocumentMetadata *oldest_document_metadata = NULL;

		g_hash_table_iter_init (&iter, store->priv->hash_table);
		while (g_hash_table_iter_next (&iter, &key, &value))
		{
			GFile *location = G_FILE (key);
			DocumentMetadata *document_metadata = value;

			if (oldest_location == NULL ||
			    document_metadata->atime < oldest_document_metadata->atime)
			{
				oldest_location = location;
				oldest_document_metadata = document_metadata;
			}
		}

		g_hash_table_remove (store->priv->hash_table, oldest_location);
	}
}

/**
 * tepl_metadata_store_save:
 * @store: the #TeplMetadataStore.
 * @cancellable: (nullable): optional #GCancellable object, %NULL to ignore.
 * @error: location to a %NULL #GError, or %NULL.
 *
 * Saves synchronously the metadata to the store file. You need to call
 * tepl_metadata_store_set_store_file() before.
 *
 * An asynchronous version doesn't exist because this function is meant to be
 * called on application shutdown. TODO: refer to #GApplication API, the exact
 * shutdown phase.
 *
 * This function respects the configuration as set with
 * tepl_metadata_store_set_max_number_of_locations().
 *
 * Returns: whether the metadata was saved successfully.
 * Since: 5.0
 */
gboolean
tepl_metadata_store_save (TeplMetadataStore  *store,
			  GCancellable       *cancellable,
			  GError            **error)
{
	GBytes *bytes;
	gboolean ok;

	g_return_val_if_fail (TEPL_IS_METADATA_STORE (store), FALSE);
	g_return_val_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail (store->priv->xml_file != NULL, FALSE);

	if (!store->priv->modified)
	{
		return TRUE;
	}

	if (!tepl_utils_create_parent_directories (store->priv->xml_file, cancellable, error))
	{
		return FALSE;
	}

	resize_hash_table_according_to_max_number_of_locations (store);
	bytes = to_string (store);
	ok = g_file_replace_contents (store->priv->xml_file,
				      g_bytes_get_data (bytes, NULL),
				      g_bytes_get_size (bytes),
				      NULL,
				      FALSE,
				      G_FILE_CREATE_NONE,
				      NULL,
				      cancellable,
				      error);

	g_bytes_unref (bytes);
	return ok;
}

/**
 * tepl_metadata_store_get_metadata_for_location:
 * @store: the #TeplMetadataStore.
 * @location: a #GFile.
 *
 * Returns: (transfer full) (nullable): a #GFileInfo containing the metadata,
 * under the "metadata" namespace. Or %NULL if there is no metadata for
 * @location.
 * Since: 5.0
 */
GFileInfo *
tepl_metadata_store_get_metadata_for_location (TeplMetadataStore *store,
					       GFile             *location)
{
	DocumentMetadata *document_metadata;

	g_return_val_if_fail (TEPL_IS_METADATA_STORE (store), NULL);
	g_return_val_if_fail (G_IS_FILE (location), NULL);

	document_metadata = g_hash_table_lookup (store->priv->hash_table, location);

	if (document_metadata == NULL)
	{
		return NULL;
	}

	document_metadata_set_current_atime (document_metadata);
	store->priv->modified = TRUE;

	return g_object_ref (document_metadata->entries);
}

/**
 * tepl_metadata_store_set_metadata_for_location:
 * @store: the #TeplMetadataStore.
 * @location: a #GFile.
 * @metadata: (nullable): a #GFileInfo containing the metadata, or %NULL to
 * remove the metadata for @location.
 *
 * Since: 5.0
 */
void
tepl_metadata_store_set_metadata_for_location (TeplMetadataStore *store,
					       GFile             *location,
					       GFileInfo         *metadata)
{
	g_return_if_fail (TEPL_IS_METADATA_STORE (store));
	g_return_if_fail (G_IS_FILE (location));
	g_return_if_fail (metadata == NULL || G_IS_FILE_INFO (metadata));

	if (metadata != NULL)
	{
		DocumentMetadata *document_metadata;

		document_metadata = document_metadata_new ();
		document_metadata_set_current_atime (document_metadata);
		document_metadata->entries = g_object_ref (metadata);

		g_hash_table_replace (store->priv->hash_table,
				      g_object_ref (location),
				      document_metadata);
	}
	else
	{
		g_hash_table_remove (store->priv->hash_table, location);
	}

	store->priv->modified = TRUE;
}
