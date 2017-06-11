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
	g_assert (ret);
	g_assert_cmpstr (host, ==, "example.net");
	g_free (host);

	ret = _tepl_utils_decode_uri ("smb://154.23.201.4/home/file.c",
				      NULL, NULL, &host, NULL, NULL);
	g_assert (ret);
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

static void
check_strv_equal (const gchar * const *strv1,
		  const gchar * const *strv2)
{
	gint i;

	if (strv1 == NULL || strv2 == NULL)
	{
		g_assert (strv1 == NULL && strv2 == NULL);
		return;
	}

	for (i = 0; strv1[i] != NULL && strv2[i] != NULL; i++)
	{
		g_assert_cmpstr (strv1[i], ==, strv2[i]);
	}

	g_assert (strv1[i] == NULL);
	g_assert (strv2[i] == NULL);
}

static void
test_strv_copy (void)
{
	const gchar *stack_strv_empty[] = { NULL };
	const gchar *stack_strv_nonempty[] = { "a", "b", NULL };
	GPtrArray *ptr_array;
	gchar **heap_strv;
	gchar **strv_copy;

	/* NULL */
	strv_copy = _tepl_utils_strv_copy (NULL);
	g_assert (strv_copy == NULL);

	/* Empty */
	strv_copy = _tepl_utils_strv_copy (stack_strv_empty);
	check_strv_equal (stack_strv_empty, (const gchar * const *)strv_copy);
	g_strfreev (strv_copy);

	/* Non-empty */
	strv_copy = _tepl_utils_strv_copy (stack_strv_nonempty);
	check_strv_equal (stack_strv_nonempty, (const gchar * const *)strv_copy);
	g_strfreev (strv_copy);

	/* Created from a GPtrArray */
	ptr_array = g_ptr_array_new ();
	g_ptr_array_add (ptr_array, g_strdup (""));
	g_ptr_array_add (ptr_array, g_strdup ("non-empty"));
	g_ptr_array_add (ptr_array, g_strdup ("bathory"));
	g_ptr_array_add (ptr_array, NULL);

	heap_strv = (gchar **)g_ptr_array_free (ptr_array, FALSE);

	strv_copy = _tepl_utils_strv_copy ((const gchar * const *)heap_strv);
	check_strv_equal ((const gchar * const *)heap_strv,
			  (const gchar * const *)strv_copy);
	g_strfreev (strv_copy);

	g_strfreev (heap_strv);
}

gint
main (gint    argc,
      gchar **argv)
{
	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/utils/replace-home-dir-with-tilde", test_replace_home_dir_with_tilde);
	g_test_add_func ("/utils/decode-uri", test_decode_uri);
	g_test_add_func ("/utils/get-fallback-basename-for-display", test_get_fallback_basename_for_display);
	g_test_add_func ("/utils/strv-copy", test_strv_copy);

	return g_test_run ();
}
