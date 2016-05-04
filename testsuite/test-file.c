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

#include <gtef/gtef.h>

static void
test_get_set_metadata (void)
{
	GtefFile *file;
	const gchar *key;
	gchar *value;

	file = gtef_file_new ();

	key = "gtef-test-key";
	value = gtef_file_get_metadata (file, key);
	g_assert (value == NULL);

	gtef_file_set_metadata (file, key, "zippy");
	value = gtef_file_get_metadata (file, key);
	g_assert_cmpstr (value, ==, "zippy");
	g_free (value);

	value = gtef_file_get_metadata (file, "gtef-test-other-key");
	g_assert (value == NULL);

	gtef_file_set_metadata (file, key, "zippiness");
	value = gtef_file_get_metadata (file, key);
	g_assert_cmpstr (value, ==, "zippiness");
	g_free (value);

	/* Unset */
	gtef_file_set_metadata (file, key, NULL);
	value = gtef_file_get_metadata (file, key);
	g_assert (value == NULL);

	/* Unset non-set metadata */
	key = "gtef-test-other-key";
	gtef_file_set_metadata (file, key, NULL);
	value = gtef_file_get_metadata (file, key);
	g_assert (value == NULL);

	g_object_unref (file);
}

static void
test_load_save_metadata (void)
{
	GtefFile *file;
	const gchar *key;
	const gchar *other_key;
	gchar *value;
	gchar *path;
	GFile *location;
	GError *error = NULL;
	gboolean ok;

	file = gtef_file_new ();

	/* NULL location */

	key = "gtef-test-key";
	gtef_file_set_metadata (file, key, "epica");

	ok = gtef_file_load_metadata (file, NULL, &error);
	g_assert_no_error (error);
	g_assert (!ok);

	ok = gtef_file_save_metadata (file, NULL, &error);
	g_assert_no_error (error);
	g_assert (!ok);

	value = gtef_file_get_metadata (file, key);
	g_assert_cmpstr (value, ==, "epica");
	g_free (value);

	/* Save metadata */

	path = g_build_filename (g_get_tmp_dir (), "gtef-metadata-test", NULL);
	location = g_file_new_for_path (path);

	gtk_source_file_set_location (GTK_SOURCE_FILE (file), location);

	ok = gtef_file_save_metadata (file, NULL, &error);
	g_assert (error != NULL); /* No such file or directory */
	g_clear_error (&error);
	g_assert (!ok);

	g_file_set_contents (path, "blum", -1, &error);
	g_assert_no_error (error);

	ok = gtef_file_save_metadata (file, NULL, &error);
	g_assert_no_error (error);
	g_assert (ok);

	g_object_unref (file);

	/* Load metadata */

	file = gtef_file_new ();
	gtk_source_file_set_location (GTK_SOURCE_FILE (file), location);

	other_key = "gtef-test-other-key";
	gtef_file_set_metadata (file, other_key, "embrace");

	ok = gtef_file_load_metadata (file, NULL, &error);
	g_assert_no_error (error);
	g_assert (ok);

	value = gtef_file_get_metadata (file, other_key);
	g_assert (value == NULL);

	value = gtef_file_get_metadata (file, key);
	g_assert_cmpstr (value, ==, "epica");
	g_free (value);

	/* Unset */

	gtef_file_set_metadata (file, key, NULL);
	ok = gtef_file_save_metadata (file, NULL, &error);
	g_assert_no_error (error);
	g_assert (ok);

	ok = gtef_file_load_metadata (file, NULL, &error);
	g_assert_no_error (error);
	g_assert (ok);

	value = gtef_file_get_metadata (file, key);
	g_assert (value == NULL);

	/* Clean-up */

	g_file_delete (location, NULL, &error);
	g_assert_no_error (error);

	ok = gtef_file_load_metadata (file, NULL, &error);
	g_assert (error != NULL); /* No such file or directory */
	g_clear_error (&error);
	g_assert (!ok);

	g_free (path);
	g_object_unref (location);
	g_object_unref (file);
}

gint
main (gint    argc,
      gchar **argv)
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/file/get_set_metadata", test_get_set_metadata);
	g_test_add_func ("/file/load_save_metadata", test_load_save_metadata);

	return g_test_run ();
}
