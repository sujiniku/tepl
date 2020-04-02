/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2016-2020 - Sébastien Wilmet <swilmet@gnome.org>
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

#define TEST_KEY "tepl-test-key"
#define TEST_OTHER_KEY "tepl-test-other-key"

typedef struct
{
	GError **error;
	gboolean ok;
} Data;

static void
save_sync_cb (GObject      *source_object,
	      GAsyncResult *result,
	      gpointer      user_data)
{
	TeplFileMetadata *metadata = TEPL_FILE_METADATA (source_object);
	Data *data = user_data;

	data->ok = tepl_file_metadata_save_finish (metadata, result, data->error);

	gtk_main_quit ();
}

static gboolean
save_sync (TeplFileMetadata  *metadata,
	   GFile             *location,
	   GError           **error)
{
	Data data;
	data.error = error;
	data.ok = FALSE;

	tepl_file_metadata_save_async (metadata,
				       location,
				       G_PRIORITY_DEFAULT,
				       NULL,
				       save_sync_cb,
				       &data);
	gtk_main ();

	return data.ok;
}

static void
load_sync_cb (GObject      *source_object,
	      GAsyncResult *result,
	      gpointer      user_data)
{
	TeplFileMetadata *metadata = TEPL_FILE_METADATA (source_object);
	Data *data = user_data;

	data->ok = tepl_file_metadata_load_finish (metadata, result, data->error);

	gtk_main_quit ();
}

static gboolean
load_sync (TeplFileMetadata  *metadata,
	   GFile             *location,
	   GError           **error)
{
	Data data;
	data.error = error;
	data.ok = FALSE;

	tepl_file_metadata_load_async (metadata,
				       location,
				       G_PRIORITY_DEFAULT,
				       NULL,
				       load_sync_cb,
				       &data);
	gtk_main ();

	return data.ok;
}

static void
test_get_set_metadata (void)
{
	TeplFileMetadata *metadata;
	gchar *value;

	metadata = tepl_file_metadata_new ();

	value = tepl_file_metadata_get (metadata, TEST_KEY);
	g_assert_true (value == NULL);

	tepl_file_metadata_set (metadata, TEST_KEY, "zippy");
	value = tepl_file_metadata_get (metadata, TEST_KEY);
	g_assert_cmpstr (value, ==, "zippy");
	g_free (value);

	value = tepl_file_metadata_get (metadata, TEST_OTHER_KEY);
	g_assert_true (value == NULL);

	tepl_file_metadata_set (metadata, TEST_KEY, "zippiness");
	value = tepl_file_metadata_get (metadata, TEST_KEY);
	g_assert_cmpstr (value, ==, "zippiness");
	g_free (value);

	/* Unset */
	tepl_file_metadata_set (metadata, TEST_KEY, NULL);
	value = tepl_file_metadata_get (metadata, TEST_KEY);
	g_assert_true (value == NULL);

	/* Unset non-set metadata */
	tepl_file_metadata_set (metadata, TEST_OTHER_KEY, NULL);
	value = tepl_file_metadata_get (metadata, TEST_OTHER_KEY);
	g_assert_true (value == NULL);

	g_object_unref (metadata);
}

static void
test_load_save_metadata (void)
{
	TeplFileMetadata *metadata;
	gchar *path;
	GFile *location;
	gchar *value;
	GError *error = NULL;
	gboolean ok;

	metadata = tepl_file_metadata_new ();
	tepl_file_metadata_set (metadata, TEST_KEY, "epica");

	/* Save metadata */

	path = g_build_filename (g_get_tmp_dir (), "tepl-file-metadata-test", NULL);
	location = g_file_new_for_path (path);

	ok = save_sync (metadata, location, &error);
	g_assert_true (error != NULL); /* No such file or directory */
	g_clear_error (&error);
	g_assert_true (!ok);

	g_file_set_contents (path, "blum", -1, &error);
	g_assert_no_error (error);

	ok = save_sync (metadata, location, &error);
	g_assert_no_error (error);
	g_assert_true (ok);

	g_object_unref (metadata);

	/* Load metadata */

	metadata = tepl_file_metadata_new ();
	tepl_file_metadata_set (metadata, TEST_OTHER_KEY, "embrace");

	ok = load_sync (metadata, location, &error);
	g_assert_no_error (error);
	g_assert_true (ok);

	value = tepl_file_metadata_get (metadata, TEST_OTHER_KEY);
	g_assert_true (value == NULL);

	value = tepl_file_metadata_get (metadata, TEST_KEY);
	g_assert_cmpstr (value, ==, "epica");
	g_free (value);

	/* Unset */

	tepl_file_metadata_set (metadata, TEST_KEY, NULL);
	ok = save_sync (metadata, location, &error);
	g_assert_no_error (error);
	g_assert_true (ok);

	ok = load_sync (metadata, location, &error);
	g_assert_no_error (error);
	g_assert_true (ok);

	value = tepl_file_metadata_get (metadata, TEST_KEY);
	g_assert_true (value == NULL);

	/* Clean-up */

	g_file_delete (location, NULL, &error);
	g_assert_no_error (error);

	ok = load_sync (metadata, location, &error);
	g_assert_true (error != NULL); /* No such file or directory */
	g_clear_error (&error);
	g_assert_true (!ok);

	g_object_unref (metadata);
	g_free (path);
	g_object_unref (location);
}

/* Setting and saving metadata should not erase previously set metadata if the
 * metadata were not loaded beforehand.
 */
static void
test_set_without_load (void)
{
	TeplFileMetadata *metadata;
	gchar *path;
	GFile *location;
	gchar *value;
	GError *error = NULL;
	gboolean ok;

	metadata = tepl_file_metadata_new ();
	path = g_build_filename (g_get_tmp_dir (), "tepl-file-metadata-test-set-without-load", NULL);
	location = g_file_new_for_path (path);

	g_file_set_contents (path, "blom", -1, &error);
	g_assert_no_error (error);

	/* Set and save one metadata */
	tepl_file_metadata_set (metadata, TEST_KEY, "dimmu");
	ok = save_sync (metadata, location, &error);
	g_assert_no_error (error);
	g_assert_true (ok);

	g_object_unref (metadata);

	/* Set and save another metadata, independently */
	metadata = tepl_file_metadata_new ();
	tepl_file_metadata_set (metadata, TEST_OTHER_KEY, "borgir");
	ok = save_sync (metadata, location, &error);
	g_assert_no_error (error);
	g_assert_true (ok);

	/* Load */
	ok = load_sync (metadata, location, &error);
	g_assert_no_error (error);
	g_assert_true (ok);

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
	ok = save_sync (metadata, location, &error);
	g_assert_no_error (error);
	g_assert_true (ok);

	g_file_delete (location, NULL, &error);
	g_assert_no_error (error);

	g_object_unref (metadata);
	g_free (path);
	g_object_unref (location);
}

int
main (int    argc,
      char **argv)
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/file_metadata/get_set_metadata", test_get_set_metadata);
	g_test_add_func ("/file_metadata/load_save_metadata", test_load_save_metadata);
	g_test_add_func ("/file_metadata/set_without_load", test_set_without_load);

	return g_test_run ();
}
