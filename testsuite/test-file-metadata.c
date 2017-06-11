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
#include <glib/gstdio.h>

#define TEST_KEY "tepl-test-key"
#define TEST_OTHER_KEY "tepl-test-other-key"

static gchar *
get_metadata_manager_path (void)
{
	return g_build_filename (g_get_tmp_dir (), "tepl-metadata-manager-store.xml", NULL);
}

static void
setup_unit_test (void)
{
	gchar *path;

	path = get_metadata_manager_path ();
	tepl_metadata_manager_init (path);
	_tepl_metadata_manager_set_unit_test_mode ();
	g_free (path);
}

static void
teardown_unit_test (void)
{
	gchar *path;

	tepl_metadata_manager_shutdown ();

	path = get_metadata_manager_path ();
	g_unlink (path);
	g_free (path);
}

static TeplFile *
create_file (gboolean use_gvfs_metadata)
{
	TeplFile *file;
	TeplFileMetadata *metadata;

	file = tepl_file_new ();

	metadata = tepl_file_get_file_metadata (file);
	_tepl_file_metadata_set_use_gvfs_metadata (metadata, use_gvfs_metadata);

	return file;
}

static void
do_test_get_set_metadata (gboolean use_gvfs_metadata)
{
	TeplFile *file;
	TeplFileMetadata *metadata;
	gchar *value;

	file = create_file (use_gvfs_metadata);
	metadata = tepl_file_get_file_metadata (file);

	value = tepl_file_metadata_get (metadata, TEST_KEY);
	g_assert (value == NULL);

	tepl_file_metadata_set (metadata, TEST_KEY, "zippy");
	value = tepl_file_metadata_get (metadata, TEST_KEY);
	g_assert_cmpstr (value, ==, "zippy");
	g_free (value);

	value = tepl_file_metadata_get (metadata, TEST_OTHER_KEY);
	g_assert (value == NULL);

	tepl_file_metadata_set (metadata, TEST_KEY, "zippiness");
	value = tepl_file_metadata_get (metadata, TEST_KEY);
	g_assert_cmpstr (value, ==, "zippiness");
	g_free (value);

	/* Unset */
	tepl_file_metadata_set (metadata, TEST_KEY, NULL);
	value = tepl_file_metadata_get (metadata, TEST_KEY);
	g_assert (value == NULL);

	/* Unset non-set metadata */
	tepl_file_metadata_set (metadata, TEST_OTHER_KEY, NULL);
	value = tepl_file_metadata_get (metadata, TEST_OTHER_KEY);
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
	TeplFile *file;
	TeplFileMetadata *metadata;
	gchar *value;
	gchar *path;
	GFile *location;
	GError *error = NULL;
	gboolean ok;

	file = create_file (use_gvfs_metadata);
	metadata = tepl_file_get_file_metadata (file);

	/* NULL location */

	tepl_file_metadata_set (metadata, TEST_KEY, "epica");

	ok = tepl_file_metadata_load (metadata, NULL, &error);
	g_assert_no_error (error);
	g_assert (!ok);

	ok = tepl_file_metadata_save (metadata, NULL, &error);
	g_assert_no_error (error);
	g_assert (!ok);

	value = tepl_file_metadata_get (metadata, TEST_KEY);
	g_assert_cmpstr (value, ==, "epica");
	g_free (value);

	/* Save metadata */

	path = g_build_filename (g_get_tmp_dir (), "tepl-metadata-test-sync", NULL);
	location = g_file_new_for_path (path);

	tepl_file_set_location (file, location);

	ok = tepl_file_metadata_save (metadata, NULL, &error);
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

	ok = tepl_file_metadata_save (metadata, NULL, &error);
	g_assert_no_error (error);
	g_assert (ok);

	g_object_unref (file);

	/* Load metadata */

	file = create_file (use_gvfs_metadata);
	metadata = tepl_file_get_file_metadata (file);
	tepl_file_set_location (file, location);

	tepl_file_metadata_set (metadata, TEST_OTHER_KEY, "embrace");

	ok = tepl_file_metadata_load (metadata, NULL, &error);
	g_assert_no_error (error);
	g_assert (ok);

	value = tepl_file_metadata_get (metadata, TEST_OTHER_KEY);
	g_assert (value == NULL);

	value = tepl_file_metadata_get (metadata, TEST_KEY);
	g_assert_cmpstr (value, ==, "epica");
	g_free (value);

	/* Unset */

	tepl_file_metadata_set (metadata, TEST_KEY, NULL);
	ok = tepl_file_metadata_save (metadata, NULL, &error);
	g_assert_no_error (error);
	g_assert (ok);

	ok = tepl_file_metadata_load (metadata, NULL, &error);
	g_assert_no_error (error);
	g_assert (ok);

	value = tepl_file_metadata_get (metadata, TEST_KEY);
	g_assert (value == NULL);

	/* Clean-up */

	g_file_delete (location, NULL, &error);
	g_assert_no_error (error);

	ok = tepl_file_metadata_load (metadata, NULL, &error);
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
	TeplFileMetadata *metadata = TEPL_FILE_METADATA (source_object);
	TeplFile *file;
	gchar *value;
	GFile *location;
	GError *error = NULL;
	gboolean ok;

	file = tepl_file_metadata_get_file (metadata);

	ok = tepl_file_metadata_load_finish (metadata, result, &error);
	g_assert_no_error (error);
	g_assert (ok);

	value = tepl_file_metadata_get (metadata, TEST_KEY);
	g_assert_cmpstr (value, ==, "in flames");
	g_free (value);

	/* Unset and clean-up */

	tepl_file_metadata_set (metadata, TEST_KEY, NULL);
	ok = tepl_file_metadata_save (metadata, NULL, &error);
	g_assert_no_error (error);
	g_assert (ok);

	location = tepl_file_get_location (file);
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
	TeplFileMetadata *metadata = TEPL_FILE_METADATA (source_object);
	GError *error = NULL;
	gboolean ok;

	ok = tepl_file_metadata_save_finish (metadata, result, &error);
	g_assert_no_error (error);
	g_assert (ok);

	tepl_file_metadata_set (metadata, TEST_KEY, NULL);

	tepl_file_metadata_load_async (metadata,
				       G_PRIORITY_DEFAULT,
				       NULL,
				       load_metadata_async_cb,
				       NULL);
}

/* Unit test not as complete as the sync version. */
static void
do_test_load_save_metadata_async (gboolean use_gvfs_metadata)
{
	TeplFile *file;
	TeplFileMetadata *metadata;
	gchar *path;
	GFile *location;
	GError *error = NULL;

	file = create_file (use_gvfs_metadata);
	metadata = tepl_file_get_file_metadata (file);

	path = g_build_filename (g_get_tmp_dir (), "tepl-metadata-test-async", NULL);

	location = g_file_new_for_path (path);
	tepl_file_set_location (file, location);
	g_object_unref (location);

	g_file_set_contents (path, "blum", -1, &error);
	g_assert_no_error (error);
	g_free (path);

	tepl_file_metadata_set (metadata, TEST_KEY, "in flames");

	tepl_file_metadata_save_async (metadata,
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
	TeplFile *file;
	TeplFileMetadata *metadata;
	gchar *path;
	GFile *location;
	gchar *value;
	GError *error = NULL;
	gboolean ok;

	file = create_file (use_gvfs_metadata);
	metadata = tepl_file_get_file_metadata (file);
	path = g_build_filename (g_get_tmp_dir (), "tepl-metadata-test-set-without-load", NULL);
	location = g_file_new_for_path (path);
	tepl_file_set_location (file, location);

	g_file_set_contents (path, "blom", -1, &error);
	g_assert_no_error (error);

	/* Set and save one metadata */
	tepl_file_metadata_set (metadata, TEST_KEY, "dimmu");
	ok = tepl_file_metadata_save (metadata, NULL, &error);
	g_assert_no_error (error);
	g_assert (ok);

	g_object_unref (file);

	/* Set and save another metadata, independently */
	file = create_file (use_gvfs_metadata);
	metadata = tepl_file_get_file_metadata (file);
	tepl_file_set_location (file, location);
	tepl_file_metadata_set (metadata, TEST_OTHER_KEY, "borgir");
	ok = tepl_file_metadata_save (metadata, NULL, &error);
	g_assert_no_error (error);
	g_assert (ok);

	/* Load */
	ok = tepl_file_metadata_load (metadata, NULL, &error);
	g_assert_no_error (error);
	g_assert (ok);

	/* Check that the two metadata are present */
	value = tepl_file_metadata_get (metadata, TEST_KEY);
	g_assert_cmpstr (value, ==, "dimmu");
	g_free (value);

	value = tepl_file_metadata_get (metadata, TEST_OTHER_KEY);
	g_assert_cmpstr (value, ==, "borgir");
	g_free (value);

	/* Clean-up */
	tepl_file_metadata_set (metadata, TEST_KEY, NULL);
	tepl_file_metadata_set (metadata, TEST_OTHER_KEY, NULL);
	ok = tepl_file_metadata_save (metadata, NULL, &error);
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
