/* Copyright 2017 - Sébastien Wilmet <swilmet@gnome.org>
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

#include "tepl-file-content.h"
#include <uchardet.h>
#include "tepl-encoding-private.h"

struct _TeplFileContentPrivate
{
	/* List of GBytes*. */
	GQueue *chunks;
};

/* Take the default buffer-size of TeplEncodingConverter. */
#define ENCODING_CONVERTER_BUFFER_SIZE (-1)

G_DEFINE_TYPE_WITH_PRIVATE (TeplFileContent, _tepl_file_content, G_TYPE_OBJECT)

static gboolean
chunk_is_valid (GBytes *chunk)
{
	return (chunk != NULL &&
		g_bytes_get_size (chunk) > 0);
}

static void
_tepl_file_content_finalize (GObject *object)
{
	TeplFileContent *content = TEPL_FILE_CONTENT (object);

	if (content->priv->chunks != NULL)
	{
		g_queue_free_full (content->priv->chunks, (GDestroyNotify)g_bytes_unref);
		content->priv->chunks = NULL;
	}

	G_OBJECT_CLASS (_tepl_file_content_parent_class)->finalize (object);
}

static void
_tepl_file_content_class_init (TeplFileContentClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = _tepl_file_content_finalize;
}

static void
_tepl_file_content_init (TeplFileContent *content)
{
	content->priv = _tepl_file_content_get_instance_private (content);

	content->priv->chunks = g_queue_new ();
}

TeplFileContent *
_tepl_file_content_new (void)
{
	return g_object_new (TEPL_TYPE_FILE_CONTENT, NULL);
}

void
_tepl_file_content_add_chunk (TeplFileContent *content,
			      GBytes          *chunk)
{
	g_return_if_fail (TEPL_IS_FILE_CONTENT (content));
	g_return_if_fail (chunk_is_valid (chunk));

	g_queue_push_tail (content->priv->chunks, g_bytes_ref (chunk));
}

static TeplEncoding *
create_encoding_for_uchardet_charset (const gchar *charset)
{
	TeplEncoding *encoding_for_charset;
	TeplEncoding *ascii_encoding;
	TeplEncoding *locale_encoding;

	g_assert (charset != NULL);

	encoding_for_charset = tepl_encoding_new (charset);

	ascii_encoding = tepl_encoding_new ("ASCII");
	locale_encoding = tepl_encoding_new_from_locale ();

	/* ASCII -> UTF-8 if locale is UTF-8.
	 *
	 * uchardet returns ASCII if only ASCII chars are present. But since any
	 * UTF-8 char can be inserted in a GtkTextView, it would be annoying for
	 * the user to have an error each time the text becomes UTF-8. I think
	 * most users expect their files to be UTF-8 if their locale is UTF-8.
	 * The exception here is for example to keep source code ASCII-only,
	 * maybe some projects prefer that, but I think that's the minority of
	 * users.
	 *
	 * TODO: have a list of candidate encodings, and if ASCII is before
	 * UTF-8, keep ASCII. This could be configurable if there is a GSetting
	 * for the candidate encodings, with a GUI to configure the list, like
	 * in gedit.
	 */
	if (tepl_encoding_equals (encoding_for_charset, ascii_encoding) &&
	    tepl_encoding_is_utf8 (locale_encoding))
	{
		tepl_encoding_free (encoding_for_charset);
		encoding_for_charset = tepl_encoding_new_utf8 ();
	}

	tepl_encoding_free (ascii_encoding);
	tepl_encoding_free (locale_encoding);

	return encoding_for_charset;
}

static TeplEncoding *
determine_encoding_with_uchardet (TeplFileContent *content)
{
	uchardet_t ud;
	const gchar *charset;
	TeplEncoding *encoding = NULL;
	GList *l;

	ud = uchardet_new ();

	for (l = content->priv->chunks->head; l != NULL; l = l->next)
	{
		GBytes *chunk = l->data;

		g_assert (chunk_is_valid (chunk));

		uchardet_handle_data (ud,
				      g_bytes_get_data (chunk, NULL),
				      g_bytes_get_size (chunk));
	}

	uchardet_data_end (ud);

	charset = uchardet_get_charset (ud);
	if (charset != NULL && charset[0] != '\0')
	{
		encoding = create_encoding_for_uchardet_charset (charset);
	}

	uchardet_delete (ud);

	return encoding;
}

/* Try the candidate encodings one by one, taking the first without conversion
 * error.
 */
TeplEncoding *
_tepl_file_content_determine_encoding_with_fallback_mode (TeplFileContent *content,
							  GSList          *candidate_encodings)
{
	TeplEncoding *encoding = NULL;
	GSList *l;

	g_return_val_if_fail (TEPL_IS_FILE_CONTENT (content), NULL);

	for (l = candidate_encodings; l != NULL; l = l->next)
	{
		TeplEncoding *cur_encoding = l->data;

		if (_tepl_file_content_convert_to_utf8 (content, cur_encoding, NULL, NULL, NULL))
		{
			encoding = tepl_encoding_copy (cur_encoding);
			break;
		}
	}

	return encoding;
}

/* Returns: (transfer full) (nullable): the encoding, or %NULL if the encoding
 * detection failed.
 */
TeplEncoding *
_tepl_file_content_determine_encoding (TeplFileContent *content)
{
	TeplEncoding *encoding;

	g_return_val_if_fail (TEPL_IS_FILE_CONTENT (content), NULL);

	encoding = determine_encoding_with_uchardet (content);

	if (encoding == NULL)
	{
		GSList *candidate_encodings;

		candidate_encodings = tepl_encoding_get_default_candidates ();
		candidate_encodings = g_slist_concat (candidate_encodings,
						      tepl_encoding_get_all ());
		candidate_encodings = _tepl_encoding_remove_duplicates (candidate_encodings,
									TEPL_ENCODING_DUPLICATES_KEEP_FIRST);

		encoding = _tepl_file_content_determine_encoding_with_fallback_mode (content,
										     candidate_encodings);

		g_slist_free_full (candidate_encodings, (GDestroyNotify)tepl_encoding_free);
	}

	return encoding;
}

gboolean
_tepl_file_content_convert_to_utf8 (TeplFileContent                 *content,
				    TeplEncoding                    *from_encoding,
				    TeplEncodingConversionCallback   callback,
				    gpointer                         callback_user_data,
				    GError                         **error)
{
	TeplEncodingConverter *converter;
	GList *l;
	gboolean success = FALSE;

	g_return_val_if_fail (TEPL_IS_FILE_CONTENT (content), FALSE);
	g_return_val_if_fail (from_encoding != NULL, FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	converter = _tepl_encoding_converter_new (ENCODING_CONVERTER_BUFFER_SIZE);
	_tepl_encoding_converter_set_callback (converter, callback, callback_user_data);

	if (!_tepl_encoding_converter_open (converter,
					    "UTF-8",
					    tepl_encoding_get_charset (from_encoding),
					    error))
	{
		goto out;
	}

	for (l = content->priv->chunks->head; l != NULL; l = l->next)
	{
		GBytes *chunk = l->data;

		g_assert (chunk_is_valid (chunk));

		if (!_tepl_encoding_converter_feed (converter,
						    g_bytes_get_data (chunk, NULL),
						    g_bytes_get_size (chunk),
						    error))
		{
			goto out;
		}
	}

	if (!_tepl_encoding_converter_close (converter, error))
	{
		goto out;
	}

	success = TRUE;

out:
	g_object_unref (converter);
	return success;
}

/* For unit tests. */
gint64
_tepl_file_content_get_encoding_converter_buffer_size (void)
{
	TeplEncodingConverter *converter;
	gint64 buffer_size;

	converter = _tepl_encoding_converter_new (ENCODING_CONVERTER_BUFFER_SIZE);
	buffer_size = _tepl_encoding_converter_get_buffer_size (converter);
	g_object_unref (converter);

	return buffer_size;
}
