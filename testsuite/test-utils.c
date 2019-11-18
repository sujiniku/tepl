/*
 * This file is part of Tepl, a text editor library.
 *
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

#include <tepl/tepl.h>

static void
test_str_middle_truncate (void)
{
	gchar *truncated_str;

	truncated_str = tepl_utils_str_middle_truncate ("1234567890", 5);
	g_assert_cmpstr (truncated_str, ==, "12…90");
	g_free (truncated_str);
}

static void
test_str_end_truncate (void)
{
	gchar *truncated_str;

	truncated_str = tepl_utils_str_end_truncate ("1234567890", 5);
	g_assert_cmpstr (truncated_str, ==, "1234…");
	g_free (truncated_str);
}

static void
test_get_file_extension (void)
{
	gchar *extension;

	extension = tepl_utils_get_file_extension ("file.pdf");
	g_assert_cmpstr (extension, ==, ".pdf");
	g_free (extension);

	extension = tepl_utils_get_file_extension ("file.PDF");
	g_assert_cmpstr (extension, ==, ".pdf");
	g_free (extension);

	extension = tepl_utils_get_file_extension ("file.tar.gz");
	g_assert_cmpstr (extension, ==, ".gz");
	g_free (extension);

	extension = tepl_utils_get_file_extension ("path/to/file.pdf");
	g_assert_cmpstr (extension, ==, ".pdf");
	g_free (extension);

	extension = tepl_utils_get_file_extension ("file");
	g_assert_cmpstr (extension, ==, "");
	g_free (extension);
}

static void
test_get_file_shortname (void)
{
	gchar *shortname;

	shortname = tepl_utils_get_file_shortname ("file.txt");
	g_assert_cmpstr (shortname, ==, "file");
	g_free (shortname);

	shortname = tepl_utils_get_file_shortname ("file.tar.gz");
	g_assert_cmpstr (shortname, ==, "file.tar");
	g_free (shortname);

	shortname = tepl_utils_get_file_shortname ("file");
	g_assert_cmpstr (shortname, ==, "file");
	g_free (shortname);

	shortname = tepl_utils_get_file_shortname ("dir.ext/blah");
	g_assert_cmpstr (shortname, ==, "dir.ext/blah");
	g_free (shortname);
}

static void
test_replace_home_dir_with_tilde (void)
{
	const gchar *homedir = g_get_home_dir ();
	gchar *before;
	gchar *after;

	before = g_build_filename (homedir, "blah", NULL);
	after = _tepl_utils_replace_home_dir_with_tilde (before);
	g_assert_cmpstr (after, ==, "~/blah");
	g_free (before);
	g_free (after);

	after = _tepl_utils_replace_home_dir_with_tilde (homedir);
	g_assert_cmpstr (after, ==, "~");
	g_free (after);

	after = _tepl_utils_replace_home_dir_with_tilde ("/blah");
	g_assert_cmpstr (after, ==, "/blah");
	g_free (after);
}

static void
test_decode_uri (void)
{
	gchar *host;
	gboolean ret;

	/* Basic test, for what is used in Tepl (the host). */
	ret = _tepl_utils_decode_uri ("smb://example.net/home/file.c",
				      NULL, NULL, &host, NULL, NULL);
	g_assert_true (ret);
	g_assert_cmpstr (host, ==, "example.net");
	g_free (host);

	ret = _tepl_utils_decode_uri ("smb://154.23.201.4/home/file.c",
				      NULL, NULL, &host, NULL, NULL);
	g_assert_true (ret);
	g_assert_cmpstr (host, ==, "154.23.201.4");
	g_free (host);
}

static void
test_get_fallback_basename_for_display (void)
{
	GFile *location;
	gchar *basename;

	location = g_file_new_for_path ("/home/seb/blom");
	basename = _tepl_utils_get_fallback_basename_for_display (location);
	g_assert_cmpstr (basename, ==, "blom");
	g_object_unref (location);
	g_free (basename);

	location = g_file_new_for_uri ("ssh://swilmet@example.net/home/swilmet/bloum");
	basename = _tepl_utils_get_fallback_basename_for_display (location);
	g_assert_cmpstr (basename, ==, "bloum");
	g_object_unref (location);
	g_free (basename);

	location = g_file_new_for_uri ("https://example.net");
	basename = _tepl_utils_get_fallback_basename_for_display (location);
	g_assert_cmpstr (basename, ==, "https://example.net");
	g_object_unref (location);
	g_free (basename);

	location = g_file_new_for_uri ("https://bugzilla.gnome.org/page.cgi?id=browse.html&product=gtksourceview");
	basename = _tepl_utils_get_fallback_basename_for_display (location);
	g_assert_cmpstr (basename, ==, "page.cgi?id=browse.html&product=gtksourceview");
	g_object_unref (location);
	g_free (basename);
}

gint
main (gint    argc,
      gchar **argv)
{
	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/utils/str-middle-truncate", test_str_middle_truncate);
	g_test_add_func ("/utils/str-end-truncate", test_str_end_truncate);
	g_test_add_func ("/utils/get-file-extension", test_get_file_extension);
	g_test_add_func ("/utils/get-file-shortname", test_get_file_shortname);
	g_test_add_func ("/utils/replace-home-dir-with-tilde", test_replace_home_dir_with_tilde);
	g_test_add_func ("/utils/decode-uri", test_decode_uri);
	g_test_add_func ("/utils/get-fallback-basename-for-display", test_get_fallback_basename_for_display);

	return g_test_run ();
}
