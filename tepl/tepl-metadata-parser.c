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
#include "tepl-metadata-parser.h"
#include <glib/gi18n-lib.h>
#include "tepl-file-metadata.h"

typedef struct _ParsingData ParsingData;
struct _ParsingData
{
	GHashTable *hash_table;

	gchar *cur_document_uri;
	TeplFileMetadata *cur_file_metadata;

	guint metadata_element_open : 1;
	guint document_element_open : 1;
};

static ParsingData *
parsing_data_new (GHashTable *hash_table)
{
	ParsingData *parsing_data;

	parsing_data = g_new0 (ParsingData, 1);
	parsing_data->hash_table = g_hash_table_ref (hash_table);

	return parsing_data;
}

static void
parsing_data_check_invariants (ParsingData *parsing_data)
{
	if (!parsing_data->metadata_element_open)
	{
		g_assert (!parsing_data->document_element_open);
		g_assert (parsing_data->cur_document_uri == NULL);
		g_assert (parsing_data->cur_file_metadata == NULL);
		return;
	}

	if (!parsing_data->document_element_open)
	{
		g_assert (parsing_data->cur_document_uri == NULL);
		g_assert (parsing_data->cur_file_metadata == NULL);
		return;
	}

	g_assert (parsing_data->cur_document_uri != NULL);
	g_assert (parsing_data->cur_file_metadata != NULL);
}

static void
parsing_data_free (ParsingData *parsing_data)
{
	if (parsing_data != NULL)
	{
		g_hash_table_unref (parsing_data->hash_table);
		g_free (parsing_data->cur_document_uri);
		g_clear_object (&parsing_data->cur_file_metadata);

		g_free (parsing_data);
	}
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
	g_assert (parsing_data->cur_file_metadata == NULL);

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

	parsing_data->cur_file_metadata = tepl_file_metadata_new ();

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
			if (!_tepl_file_metadata_set_atime_str (parsing_data->cur_file_metadata,
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
	g_assert (parsing_data->cur_file_metadata != NULL);

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
				     _("The <entry> element is missing the “key” or “value” attribute."));
		return;
	}

	_tepl_file_metadata_insert_entry (parsing_data->cur_file_metadata, key, value);
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

	g_hash_table_replace (parsing_data->hash_table,
			      g_file_new_for_uri (parsing_data->cur_document_uri),
			      parsing_data->cur_file_metadata);

	g_free (parsing_data->cur_document_uri);
	parsing_data->cur_document_uri = NULL;
	parsing_data->cur_file_metadata = NULL;

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
parse_xml_file_content (GBytes      *xml_file_bytes,
			GHashTable  *hash_table,
			GError     **error)
{
	GMarkupParser parser = { parser_start_element_cb, parser_end_element_cb, NULL, NULL, NULL };
	GMarkupParseContext *parse_context;
	ParsingData *parsing_data;
	gboolean ok;

	parsing_data = parsing_data_new (hash_table);
	parse_context = g_markup_parse_context_new (&parser, 0, parsing_data, NULL);
	ok = g_markup_parse_context_parse (parse_context,
					   g_bytes_get_data (xml_file_bytes, NULL),
					   g_bytes_get_size (xml_file_bytes),
					   error);
	if (ok)
	{
		g_markup_parse_context_end_parse (parse_context, error);
	}

	g_markup_parse_context_free (parse_context);
	parsing_data_free (parsing_data);
}

gboolean
_tepl_metadata_parser_read_file (GFile       *from_file,
				 GHashTable  *hash_table,
				 GError     **error)
{
	GBytes *bytes;
	GError *my_error = NULL;
	gboolean ok = TRUE;

	g_return_val_if_fail (G_IS_FILE (from_file), FALSE);
	g_return_val_if_fail (hash_table != NULL, FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	bytes = g_file_load_bytes (from_file, NULL, NULL, &my_error);

	/* If the XML file has not yet been created, e.g. on the first run of
	 * the application.
	 */
	if (g_error_matches (my_error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND))
	{
		g_clear_error (&my_error);
		goto out;
	}

	if (my_error != NULL)
	{
		g_propagate_error (error, my_error);
		ok = FALSE;
		goto out;
	}

	parse_xml_file_content (bytes, hash_table, &my_error);
	if (my_error != NULL)
	{
		g_propagate_error (error, my_error);
		ok = FALSE;
	}

out:
	g_bytes_unref (bytes);
	return ok;
}
