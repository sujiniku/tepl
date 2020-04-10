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

#define EXPECT_SUCCESS (TRUE)
#define EXPECT_FAILURE (FALSE)

static void
check_expect_success (gboolean  expect_success,
		      gboolean  result_ok,
		      GError   *result_error)
{
	if (expect_success)
	{
		g_assert_no_error (result_error);
		g_assert_true (result_ok);
	}
	else
	{
		g_assert_true (result_error != NULL);
		g_error_free (result_error);
		g_assert_true (!result_ok);
	}
}

static void
save_sync_cb (GObject      *source_object,
	      GAsyncResult *result,
	      gpointer      user_data)
{
	TeplFileMetadata *metadata = TEPL_FILE_METADATA (source_object);
	gboolean *expect_success = user_data;
	gboolean ok;
	GError *error = NULL;

	ok = tepl_file_metadata_save_finish (metadata, result, &error);
	check_expect_success (*expect_success, ok, error);

	gtk_main_quit ();
}

static void
save_sync (TeplFileMetadata *metadata,
	   GFile            *location,
	   gboolean          save_as,
	   gboolean          expect_success)
{
	tepl_file_metadata_save_async (metadata,
				       location,
				       save_as,
				       G_PRIORITY_DEFAULT,
				       NULL,
				       save_sync_cb,
				       &expect_success);
	gtk_main ();
}

static void
load_sync_cb (GObject      *source_object,
	      GAsyncResult *result,
	      gpointer      user_data)
{
	TeplFileMetadata *metadata = TEPL_FILE_METADATA (source_object);
	gboolean *expect_success = user_data;
	gboolean ok;
	GError *error = NULL;

	ok = tepl_file_metadata_load_finish (metadata, result, &error);
	check_expect_success (*expect_success, ok, error);

	gtk_main_quit ();
}

static void
load_sync (TeplFileMetadata *metadata,
	   GFile            *location,
	   gboolean          expect_success)
{
	tepl_file_metadata_load_async (metadata,
				       location,
				       G_PRIORITY_DEFAULT,
				       NULL,
				       load_sync_cb,
				       &expect_success);
	gtk_main ();
}

static void
check_get_metadata (TeplFileMetadata *metadata,
		    const gchar      *key,
		    const gchar      *expected_value)
{
	gchar *received_value;

	received_value = tepl_file_metadata_get (metadata, key);
	g_assert_cmpstr (received_value, ==, expected_value);
	g_free (received_value);
}

static void
file_set_contents (GFile *location)
{
	const gchar *path;
	GError *error = NULL;

	path = g_file_peek_path (location);
	g_file_set_contents (path, "blum", -1, &error);
	g_assert_no_error (error);
}

static GFile *
create_location (const gchar *test_filename,
		 gboolean     create_file)
{
	GFile *location;

	location = g_file_new_build_filename (g_get_tmp_dir (), test_filename, NULL);

	if (create_file)
	{
		file_set_contents (location);
	}

	return location;
}

static void
check_round_trip_full (GFile       *location,
		       gboolean     location_already_exists,
		       const gchar *key,
		       const gchar *value)
{
	TeplFileMetadata *metadata;
	GError *error = NULL;

	// TEST_OTHER_KEY is used below.
	g_assert_cmpstr (key, !=, TEST_OTHER_KEY);

	metadata = tepl_file_metadata_new ();
	tepl_file_metadata_set (metadata, key, value);

	/* Save metadata */

	if (!location_already_exists)
	{
		save_sync (metadata, location, FALSE, EXPECT_FAILURE); /* No such file or directory */
		file_set_contents (location);
	}

	save_sync (metadata, location, FALSE, EXPECT_SUCCESS);
	g_object_unref (metadata);

	/* Load metadata */

	metadata = tepl_file_metadata_new ();
	tepl_file_metadata_set (metadata, TEST_OTHER_KEY, "orange bill");

	load_sync (metadata, location, EXPECT_SUCCESS);

	check_get_metadata (metadata, TEST_OTHER_KEY, NULL);
	check_get_metadata (metadata, key, value);

	/* Unset */

	tepl_file_metadata_set (metadata, key, NULL);
	save_sync (metadata, location, FALSE, EXPECT_SUCCESS);
	load_sync (metadata, location, EXPECT_SUCCESS);
	check_get_metadata (metadata, key, NULL);

	/* Clean-up */

	if (!location_already_exists)
	{
		g_file_delete (location, NULL, &error);
		g_assert_no_error (error);

		load_sync (metadata, location, EXPECT_FAILURE); /* No such file or directory */
	}

	g_object_unref (metadata);
}

static void
check_round_trip (const gchar *key,
		  const gchar *value)
{
	GFile *location;

	location = create_location ("tepl-file-metadata-test", FALSE);
	check_round_trip_full (location, FALSE, key, value);
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
	TeplFileMetadata *metadata = tepl_file_metadata_new ();

	check_get_metadata (metadata, TEST_KEY, NULL);

	tepl_file_metadata_set (metadata, TEST_KEY, "zippy");
	check_get_metadata (metadata, TEST_KEY, "zippy");

	check_get_metadata (metadata, TEST_OTHER_KEY, NULL);

	tepl_file_metadata_set (metadata, TEST_KEY, "zippiness");
	check_get_metadata (metadata, TEST_KEY, "zippiness");

	/* Unset */
	tepl_file_metadata_set (metadata, TEST_KEY, NULL);
	check_get_metadata (metadata, TEST_KEY, NULL);

	/* Unset non-set metadata */
	tepl_file_metadata_set (metadata, TEST_OTHER_KEY, NULL);
	check_get_metadata (metadata, TEST_OTHER_KEY, NULL);

	g_object_unref (metadata);
}

/* Setting and saving metadata should not erase previously set metadata if the
 * metadata were not loaded beforehand.
 */
static void
test_set_without_load (void)
{
	TeplFileMetadata *metadata;
	GFile *location;
	GError *error = NULL;

	metadata = tepl_file_metadata_new ();
	location = create_location ("tepl-file-metadata-test-set-without-load", TRUE);

	/* Set and save one metadata */
	tepl_file_metadata_set (metadata, TEST_KEY, "dimmu");
	save_sync (metadata, location, FALSE, EXPECT_SUCCESS);
	g_object_unref (metadata);

	/* Set and save another metadata, independently */
	metadata = tepl_file_metadata_new ();
	tepl_file_metadata_set (metadata, TEST_OTHER_KEY, "borgir");
	save_sync (metadata, location, FALSE, EXPECT_SUCCESS);

	/* Load */
	load_sync (metadata, location, EXPECT_SUCCESS);

	/* Check that the two metadata are present */
	check_get_metadata (metadata, TEST_KEY, "dimmu");
	check_get_metadata (metadata, TEST_OTHER_KEY, "borgir");

	/* Clean-up */
	tepl_file_metadata_set (metadata, TEST_KEY, NULL);
	tepl_file_metadata_set (metadata, TEST_OTHER_KEY, NULL);
	save_sync (metadata, location, FALSE, EXPECT_SUCCESS);

	g_file_delete (location, NULL, &error);
	g_assert_no_error (error);

	g_object_unref (metadata);
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

/* Simulate several apps writing both metadata for the same GFile.
 * Or the same GFile being opened several times in the same app.
 * App1: load() -> set() -> save()
 * App2:   load() -> set() -> ...  save().
 * [-----------------------------------------> time
 * The save() done by App2 must not erase the metadata set by App1 if the
 * metadata keys are different.
 * S(t)imulating!
 */
static void
test_simulate_several_apps (void)
{
	TeplFileMetadata *metadata1;
	TeplFileMetadata *metadata2;
	GFile *location;
	GError *error = NULL;

	metadata1 = tepl_file_metadata_new ();
	metadata2 = tepl_file_metadata_new ();
	location = create_location ("tepl-file-metadata-stimulating-test", TRUE);

	/* Set and save an initial metadata from App1. */

	tepl_file_metadata_set (metadata1, "app1-key", "app1-value1");
	save_sync (metadata1, location, FALSE, EXPECT_SUCCESS);

	/* Load (for App1 it's not needed). */

	load_sync (metadata2, location, EXPECT_SUCCESS);
	check_get_metadata (metadata2, "app1-key", "app1-value1");

	/* Now set/change values */

	tepl_file_metadata_set (metadata1, "app1-key", "app1-value2");
	tepl_file_metadata_set (metadata2, "app2-key", "app2-value");

	/* And save */

	save_sync (metadata1, location, FALSE, EXPECT_SUCCESS);
	save_sync (metadata2, location, FALSE, EXPECT_SUCCESS);
	g_object_unref (metadata1);
	g_object_unref (metadata2);

	/* Now what's the value of "app1-key"? */

	metadata1 = tepl_file_metadata_new ();
	load_sync (metadata1, location, EXPECT_SUCCESS);

	// It must be "app1-value2" because App2 didn't set/modify "app1-key".
	check_get_metadata (metadata1, "app1-key", "app1-value2");

	/* Clean-up */

	tepl_file_metadata_set (metadata1, "app1-key", NULL);
	tepl_file_metadata_set (metadata1, "app2-key", NULL);
	save_sync (metadata1, location, FALSE, EXPECT_SUCCESS);

	g_file_delete (location, NULL, &error);
	g_assert_no_error (error);

	g_object_unref (metadata1);
	g_object_unref (location);
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
	g_test_add_func ("/file_metadata/simulate_several_apps", test_simulate_several_apps);

	return g_test_run ();
}
