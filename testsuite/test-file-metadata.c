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
#include <glib/gstdio.h>

#define TEST_KEY "gtef-test-key"
#define TEST_OTHER_KEY "gtef-test-other-key"

static gchar *
get_metadata_manager_path (void)
{
	return g_build_filename (g_get_tmp_dir (), "gtef-metadata-manager-store.xml", NULL);
}

static void
setup_unit_test (void)
{
	gchar *path;

	path = get_metadata_manager_path ();
	gtef_metadata_manager_init (path);
	_gtef_metadata_manager_set_unit_test_mode ();
	g_free (path);
}

static void
teardown_unit_test (void)
{
	gchar *path;

	gtef_metadata_manager_shutdown ();

	path = get_metadata_manager_path ();
	g_unlink (path);
	g_free (path);
}

static GtefFile *
create_file (gboolean use_gvfs_metadata)
{
	GtefFile *file;
	GtefFileMetadata *metadata;

	file = gtef_file_new ();

	metadata = gtef_file_get_file_metadata (file);
	_gtef_file_metadata_set_use_gvfs_metadata (metadata, use_gvfs_metadata);

	return file;
}

static void
do_test_get_set_metadata (gboolean use_gvfs_metadata)
{
	GtefFile *file;
	GtefFileMetadata *metadata;
	gchar *value;

	file = create_file (use_gvfs_metadata);
	metadata = gtef_file_get_file_metadata (file);

	value = gtef_file_metadata_get (metadata, TEST_KEY);
	g_assert (value == NULL);

	gtef_file_metadata_set (metadata, TEST_KEY, "zippy");
	value = gtef_file_metadata_get (metadata, TEST_KEY);
	g_assert_cmpstr (value, ==, "zippy");
	g_free (value);

	value = gtef_file_metadata_get (metadata, TEST_OTHER_KEY);
	g_assert (value == NULL);

	gtef_file_metadata_set (metadata, TEST_KEY, "zippiness");
	value = gtef_file_metadata_get (metadata, TEST_KEY);
	g_assert_cmpstr (value, ==, "zippiness");
	g_free (value);

	/* Unset */
	gtef_file_metadata_set (metadata, TEST_KEY, NULL);
	value = gtef_file_metadata_get (metadata, TEST_KEY);
	g_assert (value == NULL);

	/* Unset non-set metadata */
	gtef_file_metadata_set (metadata, TEST_OTHER_KEY, NULL);
	value = gtef_file_metadata_get (metadata, TEST_OTHER_KEY);
	g_assert (value == NULL);

	g_object_unref (file);
}

static void
test_get_set_metadata (void)
{
	setup_unit_test ();
	do_test_get_set_metadata (TRUE);
	do_test_get_set_metadata (FALSE);
	teardown_unit_test ();
}

static void
do_test_load_save_metadata_sync (gboolean use_gvfs_metadata)
{
	GtefFile *file;
	GtefFileMetadata *metadata;
	gchar *value;
	gchar *path;
	GFile *location;
	GError *error = NULL;
	gboolean ok;

	file = create_file (use_gvfs_metadata);
	metadata = gtef_file_get_file_metadata (file);

	/* NULL location */

	gtef_file_metadata_set (metadata, TEST_KEY, "epica");

	ok = gtef_file_metadata_load (metadata, NULL, &error);
	g_assert_no_error (error);
	g_assert (!ok);

	ok = gtef_file_metadata_save (metadata, NULL, &error);
	g_assert_no_error (error);
	g_assert (!ok);

	value = gtef_file_metadata_get (metadata, TEST_KEY);
	g_assert_cmpstr (value, ==, "epica");
	g_free (value);

	/* Save metadata */

	path = g_build_filename (g_get_tmp_dir (), "gtef-metadata-test-sync", NULL);
	location = g_file_new_for_path (path);

	gtk_source_file_set_location (GTK_SOURCE_FILE (file), location);

	ok = gtef_file_metadata_save (metadata, NULL, &error);
	if (use_gvfs_metadata)
	{
		g_assert (error != NULL); /* No such file or directory */
		g_clear_error (&error);
		g_assert (!ok);
	}
	else
	{
		g_assert_no_error (error);
		g_assert (ok);
	}

	g_file_set_contents (path, "blum", -1, &error);
	g_assert_no_error (error);

	ok = gtef_file_metadata_save (metadata, NULL, &error);
	g_assert_no_error (error);
	g_assert (ok);

	g_object_unref (file);

	/* Load metadata */

	file = create_file (use_gvfs_metadata);
	metadata = gtef_file_get_file_metadata (file);
	gtk_source_file_set_location (GTK_SOURCE_FILE (file), location);

	gtef_file_metadata_set (metadata, TEST_OTHER_KEY, "embrace");

	ok = gtef_file_metadata_load (metadata, NULL, &error);
	g_assert_no_error (error);
	g_assert (ok);

	value = gtef_file_metadata_get (metadata, TEST_OTHER_KEY);
	g_assert (value == NULL);

	value = gtef_file_metadata_get (metadata, TEST_KEY);
	g_assert_cmpstr (value, ==, "epica");
	g_free (value);

	/* Unset */

	gtef_file_metadata_set (metadata, TEST_KEY, NULL);
	ok = gtef_file_metadata_save (metadata, NULL, &error);
	g_assert_no_error (error);
	g_assert (ok);

	ok = gtef_file_metadata_load (metadata, NULL, &error);
	g_assert_no_error (error);
	g_assert (ok);

	value = gtef_file_metadata_get (metadata, TEST_KEY);
	g_assert (value == NULL);

	/* Clean-up */

	g_file_delete (location, NULL, &error);
	g_assert_no_error (error);

	ok = gtef_file_metadata_load (metadata, NULL, &error);
	if (use_gvfs_metadata)
	{
		g_assert (error != NULL); /* No such file or directory */
		g_clear_error (&error);
		g_assert (!ok);
	}
	else
	{
		g_assert_no_error (error);
		g_assert (ok);
	}

	g_free (path);
	g_object_unref (location);
	g_object_unref (file);
}

static void
test_load_save_metadata_sync (void)
{
	setup_unit_test ();
	do_test_load_save_metadata_sync (TRUE);
	do_test_load_save_metadata_sync (FALSE);
	teardown_unit_test ();
}

static void
load_metadata_async_cb (GObject      *source_object,
			GAsyncResult *result,
			gpointer      user_data)
{
	GtefFileMetadata *metadata = GTEF_FILE_METADATA (source_object);
	GtefFile *file;
	gchar *value;
	GFile *location;
	GError *error = NULL;
	gboolean ok;

	file = gtef_file_metadata_get_file (metadata);

	ok = gtef_file_metadata_load_finish (metadata, result, &error);
	g_assert_no_error (error);
	g_assert (ok);

	value = gtef_file_metadata_get (metadata, TEST_KEY);
	g_assert_cmpstr (value, ==, "in flames");
	g_free (value);

	/* Unset and clean-up */

	gtef_file_metadata_set (metadata, TEST_KEY, NULL);
	ok = gtef_file_metadata_save (metadata, NULL, &error);
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
	GtefFileMetadata *metadata = GTEF_FILE_METADATA (source_object);
	GError *error = NULL;
	gboolean ok;

	ok = gtef_file_metadata_save_finish (metadata, result, &error);
	g_assert_no_error (error);
	g_assert (ok);

	gtef_file_metadata_set (metadata, TEST_KEY, NULL);

	gtef_file_metadata_load_async (metadata,
				       G_PRIORITY_DEFAULT,
				       NULL,
				       load_metadata_async_cb,
				       NULL);
}

/* Unit test not as complete as the sync version. */
static void
do_test_load_save_metadata_async (gboolean use_gvfs_metadata)
{
	GtefFile *file;
	GtefFileMetadata *metadata;
	gchar *path;
	GFile *location;
	GError *error = NULL;

	file = create_file (use_gvfs_metadata);
	metadata = gtef_file_get_file_metadata (file);

	path = g_build_filename (g_get_tmp_dir (), "gtef-metadata-test-async", NULL);

	location = g_file_new_for_path (path);
	gtk_source_file_set_location (GTK_SOURCE_FILE (file), location);
	g_object_unref (location);

	g_file_set_contents (path, "blum", -1, &error);
	g_assert_no_error (error);
	g_free (path);

	gtef_file_metadata_set (metadata, TEST_KEY, "in flames");

	gtef_file_metadata_save_async (metadata,
				       G_PRIORITY_DEFAULT,
				       NULL,
				       save_metadata_async_cb,
				       NULL);

	gtk_main ();
}

static void
test_load_save_metadata_async (void)
{
	setup_unit_test ();
	do_test_load_save_metadata_async (TRUE);
	do_test_load_save_metadata_async (FALSE);
	teardown_unit_test ();
}

/* Setting and saving metadata should not erase previously set metadata if the
 * metadata were not loaded beforehand.
 */
static void
do_test_set_without_load (gboolean use_gvfs_metadata)
{
	GtefFile *file;
	GtefFileMetadata *metadata;
	gchar *path;
	GFile *location;
	gchar *value;
	GError *error = NULL;
	gboolean ok;

	file = create_file (use_gvfs_metadata);
	metadata = gtef_file_get_file_metadata (file);
	path = g_build_filename (g_get_tmp_dir (), "gtef-metadata-test-set-without-load", NULL);
	location = g_file_new_for_path (path);
	gtk_source_file_set_location (GTK_SOURCE_FILE (file), location);

	g_file_set_contents (path, "blom", -1, &error);
	g_assert_no_error (error);

	/* Set and save one metadata */
	gtef_file_metadata_set (metadata, TEST_KEY, "dimmu");
	ok = gtef_file_metadata_save (metadata, NULL, &error);
	g_assert_no_error (error);
	g_assert (ok);

	g_object_unref (file);

	/* Set and save another metadata, independently */
	file = create_file (use_gvfs_metadata);
	metadata = gtef_file_get_file_metadata (file);
	gtk_source_file_set_location (GTK_SOURCE_FILE (file), location);
	gtef_file_metadata_set (metadata, TEST_OTHER_KEY, "borgir");
	ok = gtef_file_metadata_save (metadata, NULL, &error);
	g_assert_no_error (error);
	g_assert (ok);

	/* Load */
	ok = gtef_file_metadata_load (metadata, NULL, &error);
	g_assert_no_error (error);
	g_assert (ok);

	/* Check that the two metadata are present */
	value = gtef_file_metadata_get (metadata, TEST_KEY);
	g_assert_cmpstr (value, ==, "dimmu");
	g_free (value);

	value = gtef_file_metadata_get (metadata, TEST_OTHER_KEY);
	g_assert_cmpstr (value, ==, "borgir");
	g_free (value);

	/* Clean-up */
	gtef_file_metadata_set (metadata, TEST_KEY, NULL);
	gtef_file_metadata_set (metadata, TEST_OTHER_KEY, NULL);
	ok = gtef_file_metadata_save (metadata, NULL, &error);
	g_assert_no_error (error);
	g_assert (ok);

	g_file_delete (location, NULL, &error);
	g_assert_no_error (error);

	g_free (path);
	g_object_unref (location);
	g_object_unref (file);
}

static void
test_set_without_load (void)
{
	setup_unit_test ();
	do_test_set_without_load (TRUE);
	do_test_set_without_load (FALSE);
	teardown_unit_test ();
}

gint
main (gint    argc,
      gchar **argv)
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/file/get_set_metadata", test_get_set_metadata);
	g_test_add_func ("/file/load_save_metadata_sync", test_load_save_metadata_sync);
	g_test_add_func ("/file/load_save_metadata_async", test_load_save_metadata_async);
	g_test_add_func ("/file/set_without_load", test_set_without_load);

	return g_test_run ();
}
