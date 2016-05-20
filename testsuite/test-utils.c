/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "gtef/gtef-utils.h"

static void
test_replace_home_dir_with_tilde (void)
{
	const gchar *homedir = g_get_home_dir ();
	gchar *before;
	gchar *after;

	before = g_build_filename (homedir, "blah", NULL);
	after = _gtef_utils_replace_home_dir_with_tilde (before);
	g_assert_cmpstr (after, ==, "~/blah");
	g_free (before);
	g_free (after);

	after = _gtef_utils_replace_home_dir_with_tilde (homedir);
	g_assert_cmpstr (after, ==, "~");
	g_free (after);

	after = _gtef_utils_replace_home_dir_with_tilde ("/blah");
	g_assert_cmpstr (after, ==, "/blah");
	g_free (after);
}

static void
test_decode_uri (void)
{
	gchar *host;
	gboolean ret;

	/* Basic test, for what is used in Gtef (the host). */
	ret = _gtef_utils_decode_uri ("smb://example.net/home/file.c",
				      NULL, NULL, &host, NULL, NULL);
	g_assert (ret);
	g_assert_cmpstr (host, ==, "example.net");
	g_free (host);

	ret = _gtef_utils_decode_uri ("smb://154.23.201.4/home/file.c",
				      NULL, NULL, &host, NULL, NULL);
	g_assert (ret);
	g_assert_cmpstr (host, ==, "154.23.201.4");
	g_free (host);
}

static void
test_make_valid_utf8 (void)
{
	gchar *result;
	const gchar *invalid_str;

	/* Empty */
	result = _gtef_utils_make_valid_utf8 ("");
	g_assert_cmpstr (result, ==, "");
	g_free (result);

	/* Ascii */
	result = _gtef_utils_make_valid_utf8 ("valid");
	g_assert_cmpstr (result, ==, "valid");
	g_free (result);

	/* Multi-byte char: Modifier Letter Apostrophe U+02BC */
	result = _gtef_utils_make_valid_utf8 ("doesn\xCA\xBCt");
	g_assert_cmpstr (result, ==, "doesn\xCA\xBCt");
	g_free (result);

	/* Invalid byte: \xFF
	 *
	 * \xFF in a string gives a warning with GCC. But GCC is not (yet?)
	 * smart enough for octal values. If a warning appears in the future,
	 * we'll need to find another trick.
	 */
	invalid_str = "inval\377d";
	g_assert (!g_utf8_validate (invalid_str, -1, NULL));
	result = _gtef_utils_make_valid_utf8 (invalid_str);
	g_assert_cmpstr (result, ==, "inval\357\277\275d");
	g_free (result);

	/* Several invalid bytes */
	invalid_str = "\377nval\377d";
	g_assert (!g_utf8_validate (invalid_str, -1, NULL));
	result = _gtef_utils_make_valid_utf8 (invalid_str);
	g_assert_cmpstr (result, ==, "\357\277\275nval\357\277\275d");
	g_free (result);
}

gint
main (gint    argc,
      gchar **argv)
{
	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/utils/replace-home-dir-with-tilde", test_replace_home_dir_with_tilde);
	g_test_add_func ("/utils/decode-uri", test_decode_uri);
	g_test_add_func ("/utils/make-valid-utf8", test_make_valid_utf8);

	return g_test_run ();
}
