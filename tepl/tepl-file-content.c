/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2017 - Sébastien Wilmet <swilmet@gnome.org>
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
#include "tepl-encoding.h"

struct _TeplFileContentPrivate
{
	/* List of GBytes*. */
	GQueue *chunks;
};

G_DEFINE_TYPE_WITH_PRIVATE (TeplFileContent, _tepl_file_content, G_TYPE_OBJECT)

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
	g_return_if_fail (chunk != NULL);

	g_queue_push_tail (content->priv->chunks, g_bytes_ref (chunk));
}

GQueue *
_tepl_file_content_get_chunks (TeplFileContent *content)
{
	g_return_val_if_fail (TEPL_IS_FILE_CONTENT (content), NULL);

	return content->priv->chunks;
}

static TeplEncoding *
create_encoding_for_charset (const gchar *charset)
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

/* Returns: (transfer full) (nullable): the encoding, or %NULL if the encoding
 * detection failed.
 */
TeplEncoding *
_tepl_file_content_determine_encoding (TeplFileContent *content)
{
	uchardet_t ud;
	const gchar *charset;
	TeplEncoding *encoding = NULL;
	GList *l;

	g_return_val_if_fail (TEPL_IS_FILE_CONTENT (content), NULL);

	ud = uchardet_new ();

	for (l = content->priv->chunks->head; l != NULL; l = l->next)
	{
		GBytes *chunk = l->data;

		g_assert (chunk != NULL);
		g_assert (g_bytes_get_size (chunk) > 0);

		uchardet_handle_data (ud,
				      g_bytes_get_data (chunk, NULL),
				      g_bytes_get_size (chunk));
	}

	uchardet_data_end (ud);

	charset = uchardet_get_charset (ud);
	if (charset != NULL && charset[0] != '\0')
	{
		encoding = create_encoding_for_charset (charset);
	}

	uchardet_delete (ud);

	return encoding;
}
