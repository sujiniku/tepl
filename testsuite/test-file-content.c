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

#include <tepl/tepl.h>
#include "tepl/tepl-file-content.h"
#include <string.h>

static void
add_string (TeplFileContent *content,
	    const gchar     *str)
{
	GBytes *chunk;

	chunk = g_bytes_new (str, strlen (str));
	_tepl_file_content_add_chunk (content, chunk);
	g_bytes_unref (chunk);
}

static void
check_determine_encoding_fallback (GSList       *candidate_encodings,
				   const gchar  *str,
				   TeplEncoding *expected_encoding)
{
	TeplFileContent *content;
	TeplEncoding *detected_encoding;

	content = _tepl_file_content_new ();

	if (str != NULL)
	{
		add_string (content, str);
	}

	detected_encoding = _tepl_file_content_determine_encoding_with_fallback_mode (content,
										      candidate_encodings);

	g_assert (tepl_encoding_equals (detected_encoding, expected_encoding));

	g_object_unref (content);
	tepl_encoding_free (detected_encoding);
}

static void
test_determine_encoding_with_fallback_mode (void)
{
	GSList *candidate_encodings = NULL;
	TeplEncoding *utf8_encoding;
	TeplEncoding *ascii_encoding;

	utf8_encoding = tepl_encoding_new_utf8 ();
	ascii_encoding = tepl_encoding_new ("ASCII");

	/* UTF-8 -> ASCII */
	candidate_encodings = g_slist_append (candidate_encodings, utf8_encoding);
	candidate_encodings = g_slist_append (candidate_encodings, ascii_encoding);

	// An empty/0-bytes file has no GBytes chunks, the list is empty.
	check_determine_encoding_fallback (candidate_encodings, NULL, utf8_encoding);

	check_determine_encoding_fallback (candidate_encodings, "Wistiti", utf8_encoding);
	check_determine_encoding_fallback (candidate_encodings, "Wißtiti", utf8_encoding);

	g_slist_free (candidate_encodings);
	candidate_encodings = NULL;

	/* ASCII -> UTF-8 */
	candidate_encodings = g_slist_append (candidate_encodings, ascii_encoding);
	candidate_encodings = g_slist_append (candidate_encodings, utf8_encoding);

	check_determine_encoding_fallback (candidate_encodings, NULL, ascii_encoding);
	check_determine_encoding_fallback (candidate_encodings, "Wistiti", ascii_encoding);
	check_determine_encoding_fallback (candidate_encodings, "Wißtiti", utf8_encoding);

	g_slist_free (candidate_encodings);
	candidate_encodings = NULL;

	tepl_encoding_free (utf8_encoding);
	tepl_encoding_free (ascii_encoding);
}

int
main (int    argc,
      char **argv)
{
	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/FileContent/test_determine_encoding_with_fallback_mode",
			 test_determine_encoding_with_fallback_mode);

	return g_test_run ();
}
