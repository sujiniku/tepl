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
check_round_trip_full (GFile       *location,
		       gboolean     location_already_exists,
		       const gchar *key,
		       const gchar *value)
{
	TeplFileMetadata *metadata;
	gchar *received_value;
	GError *error = NULL;
	gboolean ok;

	// TEST_OTHER_KEY is used below.
	g_assert_cmpstr (key, !=, TEST_OTHER_KEY);

	metadata = tepl_file_metadata_new ();
	tepl_file_metadata_set (metadata, key, value);

	/* Save metadata */

	if (!location_already_exists)
	{
		const gchar *path = g_file_peek_path (location);

		ok = save_sync (metadata, location, &error);
		g_assert_true (error != NULL); /* No such file or directory */
		g_clear_error (&error);
		g_assert_true (!ok);

		g_file_set_contents (path, "blum", -1, &error);
		g_assert_no_error (error);
	}

	ok = save_sync (metadata, location, &error);
	g_assert_no_error (error);
	g_assert_true (ok);

	g_object_unref (metadata);

	/* Load metadata */

	metadata = tepl_file_metadata_new ();
	tepl_file_metadata_set (metadata, TEST_OTHER_KEY, "orange bill");

	ok = load_sync (metadata, location, &error);
	g_assert_no_error (error);
	g_assert_true (ok);

	received_value = tepl_file_metadata_get (metadata, TEST_OTHER_KEY);
	g_assert_true (received_value == NULL);

	received_value = tepl_file_metadata_get (metadata, key);
	g_assert_cmpstr (received_value, ==, value);
	g_free (received_value);

	/* Unset */

	tepl_file_metadata_set (metadata, key, NULL);
	ok = save_sync (metadata, location, &error);
	g_assert_no_error (error);
	g_assert_true (ok);

	ok = load_sync (metadata, location, &error);
	g_assert_no_error (error);
	g_assert_true (ok);

	received_value = tepl_file_metadata_get (metadata, key);
	g_assert_true (received_value == NULL);

	/* Clean-up */

	if (!location_already_exists)
	{
		g_file_delete (location, NULL, &error);
		g_assert_no_error (error);

		ok = load_sync (metadata, location, &error);
		g_assert_true (error != NULL); /* No such file or directory */
		g_clear_error (&error);
		g_assert_true (!ok);
	}

	g_object_unref (metadata);
}

static void
check_round_trip (const gchar *key,
		  const gchar *value)
{
	gchar *path;
	GFile *location;

	path = g_build_filename (g_get_tmp_dir (), "tepl-file-metadata-test", NULL);
	location = g_file_new_for_path (path);

	check_round_trip_full (location, FALSE, key, value);

	g_free (path);
	g_object_unref (location);
}

static void
check_round_trip_expect_failure (const gchar *key,
				 const gchar *value)
{
	if (g_test_subprocess ())
	{
		check_round_trip (key, value);
		return;
	}

	g_test_trap_subprocess (NULL, 0, 0);
	g_test_trap_assert_failed ();
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

static void
test_arbitrary_keys_and_values_success (void)
{
	check_round_trip ("tepl-simple-key", "simple-value");
	check_round_trip ("tepl-simple-key", " ");
	check_round_trip ("tepl-simple-key", "\t");
	check_round_trip ("tepl-simple-key", "\t\t  ");
	check_round_trip ("tepl-simple-key", ",");
	check_round_trip ("tepl-simple-key", ";");
	check_round_trip ("tepl-simple-key", "*");
	check_round_trip ("tepl-simple-key", "::");
	check_round_trip ("tepl-simple-key", "Évolution-UTF-8");

	check_round_trip ("gCSVedit_column_delimiter", "simple-value");
	check_round_trip ("Fourty_Two-1337", "simple-value");
	check_round_trip ("1337-beginning-with-digit", "simple-value");
	check_round_trip ("a", "simple-value");
	check_round_trip ("9", "simple-value");
}

static void
test_arbitrary_keys_and_values_failure_01 (void)
{
	/* Non-UTF-8 value. */
	g_assert_true (!g_utf8_validate ("\xFF", -1, NULL));
	check_round_trip_expect_failure ("tepl-simple-key", "\xFF");
}

static void
test_arbitrary_keys_and_values_failure_02 (void)
{
	/* Key containing ':'. */
	check_round_trip_expect_failure ("metadata::gCSVedit-column-delimiter", "simple-value");
}

static void
test_arbitrary_keys_and_values_failure_03 (void)
{
	/* UTF-8 key. */
	check_round_trip_expect_failure ("Évolution-UTF-8", "simple-value");
}

static void
test_arbitrary_keys_and_values_failure_04 (void)
{
	/* Non-UTF-8 key. */
	g_assert_true (!g_utf8_validate ("\xFF", -1, NULL));
	check_round_trip_expect_failure ("\xFF", "simple-value");
}

static void
test_for_remote_file_success (void)
{
	GFile *remote_location;

	remote_location = g_file_new_for_uri ("https://www.google.com/");
	check_round_trip_full (remote_location, TRUE, TEST_KEY, "tell me");
	g_object_unref (remote_location);
}

static void
test_for_remote_file_failure (void)
{
	if (g_test_subprocess ())
	{
		GFile *remote_location;

		/* Doesn't exist. */
		remote_location = g_file_new_for_uri ("https://www.ursietuteiedludiev.be/");
		check_round_trip_full (remote_location, TRUE, TEST_KEY, "tell me");
		g_object_unref (remote_location);
		return;
	}

	g_test_trap_subprocess (NULL, 0, 0);
	g_test_trap_assert_failed ();
}

int
main (int    argc,
      char **argv)
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/file_metadata/get_set_metadata", test_get_set_metadata);
	g_test_add_func ("/file_metadata/set_without_load", test_set_without_load);
	g_test_add_func ("/file_metadata/arbitrary_keys_and_values_success", test_arbitrary_keys_and_values_success);
	g_test_add_func ("/file_metadata/arbitrary_keys_and_values_failure_01", test_arbitrary_keys_and_values_failure_01);
	g_test_add_func ("/file_metadata/arbitrary_keys_and_values_failure_02", test_arbitrary_keys_and_values_failure_02);
	g_test_add_func ("/file_metadata/arbitrary_keys_and_values_failure_03", test_arbitrary_keys_and_values_failure_03);
	g_test_add_func ("/file_metadata/arbitrary_keys_and_values_failure_04", test_arbitrary_keys_and_values_failure_04);
	g_test_add_func ("/file_metadata/for_remote_file_success", test_for_remote_file_success);
	g_test_add_func ("/file_metadata/for_remote_file_failure", test_for_remote_file_failure);

	return g_test_run ();
}
