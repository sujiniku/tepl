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

#define TEST_KEY "gtef-test-key"
#define TEST_OTHER_KEY "gtef-test-other-key"

static void
test_get_set_metadata (void)
{
	GtefFile *file;
	gchar *value;

	file = gtef_file_new ();

	value = gtef_file_get_metadata (file, TEST_KEY);
	g_assert (value == NULL);

	gtef_file_set_metadata (file, TEST_KEY, "zippy");
	value = gtef_file_get_metadata (file, TEST_KEY);
	g_assert_cmpstr (value, ==, "zippy");
	g_free (value);

	value = gtef_file_get_metadata (file, TEST_OTHER_KEY);
	g_assert (value == NULL);

	gtef_file_set_metadata (file, TEST_KEY, "zippiness");
	value = gtef_file_get_metadata (file, TEST_KEY);
	g_assert_cmpstr (value, ==, "zippiness");
	g_free (value);

	/* Unset */
	gtef_file_set_metadata (file, TEST_KEY, NULL);
	value = gtef_file_get_metadata (file, TEST_KEY);
	g_assert (value == NULL);

	/* Unset non-set metadata */
	gtef_file_set_metadata (file, TEST_OTHER_KEY, NULL);
	value = gtef_file_get_metadata (file, TEST_OTHER_KEY);
	g_assert (value == NULL);

	g_object_unref (file);
}

static void
test_load_save_metadata_sync (void)
{
	GtefFile *file;
	gchar *value;
	gchar *path;
	GFile *location;
	GError *error = NULL;
	gboolean ok;

	file = gtef_file_new ();

	/* NULL location */

	gtef_file_set_metadata (file, TEST_KEY, "epica");

	ok = gtef_file_load_metadata (file, NULL, &error);
	g_assert_no_error (error);
	g_assert (!ok);

	ok = gtef_file_save_metadata (file, NULL, &error);
	g_assert_no_error (error);
	g_assert (!ok);

	value = gtef_file_get_metadata (file, TEST_KEY);
	g_assert_cmpstr (value, ==, "epica");
	g_free (value);

	/* Save metadata */

	path = g_build_filename (g_get_tmp_dir (), "gtef-metadata-test-sync", NULL);
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

	gtef_file_set_metadata (file, TEST_OTHER_KEY, "embrace");

	ok = gtef_file_load_metadata (file, NULL, &error);
	g_assert_no_error (error);
	g_assert (ok);

	value = gtef_file_get_metadata (file, TEST_OTHER_KEY);
	g_assert (value == NULL);

	value = gtef_file_get_metadata (file, TEST_KEY);
	g_assert_cmpstr (value, ==, "epica");
	g_free (value);

	/* Unset */

	gtef_file_set_metadata (file, TEST_KEY, NULL);
	ok = gtef_file_save_metadata (file, NULL, &error);
	g_assert_no_error (error);
	g_assert (ok);

	ok = gtef_file_load_metadata (file, NULL, &error);
	g_assert_no_error (error);
	g_assert (ok);

	value = gtef_file_get_metadata (file, TEST_KEY);
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

static void
load_metadata_async_cb (GObject      *source_object,
			GAsyncResult *result,
			gpointer      user_data)
{
	GtefFile *file = GTEF_FILE (source_object);
	gchar *value;
	GFile *location;
	GError *error = NULL;
	gboolean ok;

	ok = gtef_file_load_metadata_finish (file, result, &error);
	g_assert_no_error (error);
	g_assert (ok);

	value = gtef_file_get_metadata (file, TEST_KEY);
	g_assert_cmpstr (value, ==, "in flames");
	g_free (value);

	/* Unset and clean-up */

	gtef_file_set_metadata (file, TEST_KEY, NULL);
	ok = gtef_file_save_metadata (file, NULL, &error);
	g_assert_no_error (error);
	g_assert (ok);

	location = gtk_source_file_get_location (GTK_SOURCE_FILE (file));
	g_file_delete (location, NULL, &error);
	g_assert_no_error (error);

	g_object_unref (file);
	gtk_main_quit ();
}

static void
save_metadata_async_cb (GObject      *source_object,
			GAsyncResult *result,
			gpointer      user_data)
{
	GtefFile *file = GTEF_FILE (source_object);
	GError *error = NULL;
	gboolean ok;

	ok = gtef_file_save_metadata_finish (file, result, &error);
	g_assert_no_error (error);
	g_assert (ok);

	gtef_file_set_metadata (file, TEST_KEY, NULL);

	gtef_file_load_metadata_async (file,
				       G_PRIORITY_DEFAULT,
				       NULL,
				       load_metadata_async_cb,
				       NULL);
}

/* Unit test not as complete as the sync version. */
static void
test_load_save_metadata_async (void)
{
	GtefFile *file;
	gchar *path;
	GFile *location;
	GError *error = NULL;

	file = gtef_file_new ();

	path = g_build_filename (g_get_tmp_dir (), "gtef-metadata-test-async", NULL);

	location = g_file_new_for_path (path);
	gtk_source_file_set_location (GTK_SOURCE_FILE (file), location);
	g_object_unref (location);

	g_file_set_contents (path, "blum", -1, &error);
	g_assert_no_error (error);
	g_free (path);

	gtef_file_set_metadata (file, TEST_KEY, "in flames");

	gtef_file_save_metadata_async (file,
				       G_PRIORITY_DEFAULT,
				       NULL,
				       save_metadata_async_cb,
				       NULL);

	gtk_main ();
}

gint
main (gint    argc,
      gchar **argv)
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/file/get_set_metadata", test_get_set_metadata);
	g_test_add_func ("/file/load_save_metadata_sync", test_load_save_metadata_sync);
	g_test_add_func ("/file/load_save_metadata_async", test_load_save_metadata_async);

	return g_test_run ();
}
