/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <tepl/tepl.h>
#include "tepl-test-utils.h"

static void
check_get (TeplMetadata *metadata,
	   const gchar  *key,
	   const gchar  *expected_value)
{
	gchar *received_value;

	received_value = tepl_metadata_get (metadata, key);
	g_assert_cmpstr (expected_value, ==, received_value);
	g_free (received_value);
}

static GFile *
get_store_file_for_test_data_filename (const gchar *filename)
{
	return g_file_new_build_filename (UNIT_TESTS_SOURCE_DIR,
					  "test-metadata-manager-data",
					  filename,
					  NULL);
}

static GFile *
save_metadata_manager (void)
{
	GFile *tmp_file;
	TeplMetadataManager *manager;
	GError *error = NULL;

	tmp_file = g_file_new_build_filename (g_get_tmp_dir (), "tepl-test-metadata-manager.xml", NULL);
	g_file_delete (tmp_file, NULL, &error);
	if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND))
	{
		g_clear_error (&error);
	}
	g_assert_no_error (error);

	manager = tepl_metadata_manager_get_singleton ();
	tepl_metadata_manager_save_to_disk (manager, tmp_file, TRUE, &error);
	g_assert_no_error (error);

	return tmp_file;
}

static void
test_merge_into_and_copy_from_part1 (void)
{
	TeplMetadataManager *manager;
	TeplMetadata *metadata;
	GFile *location;

	manager = tepl_metadata_manager_get_singleton ();
	metadata = tepl_metadata_new ();
	location = g_file_new_for_path ("location");

	tepl_metadata_manager_copy_from (manager, location, metadata);
	tepl_metadata_manager_merge_into (manager, location, metadata);

	tepl_metadata_set (metadata, "key", "valueA");
	tepl_metadata_manager_copy_from (manager, location, metadata);
	check_get (metadata, "key", "valueA"); // the value has been kept even though it is not in @manager.
	tepl_metadata_manager_merge_into (manager, location, metadata);
	check_get (metadata, "key", "valueA");

	g_object_unref (metadata);
	metadata = tepl_metadata_new ();
	tepl_metadata_manager_copy_from (manager, location, metadata);
	check_get (metadata, "key", "valueA");

	tepl_metadata_set (metadata, "key", "valueB");
	check_get (metadata, "key", "valueB");
	tepl_metadata_manager_copy_from (manager, location, metadata);
	check_get (metadata, "key", "valueA"); // the value has been overwritten.

	g_object_unref (metadata);
	g_object_unref (location);
	_tepl_metadata_manager_unref_singleton ();
}

static void
test_merge_into_and_copy_from_part2 (void)
{
	TeplMetadataManager *manager;
	TeplMetadata *metadata;
	GFile *location;

	manager = tepl_metadata_manager_get_singleton ();
	location = g_file_new_for_path ("location");

	metadata = tepl_metadata_new ();
	tepl_metadata_set (metadata, "keyA", "valueA");
	tepl_metadata_manager_merge_into (manager, location, metadata);
	g_object_unref (metadata);

	metadata = tepl_metadata_new ();
	tepl_metadata_set (metadata, "keyB", "valueB");
	tepl_metadata_manager_merge_into (manager, location, metadata); // keyA is kept in @manager.
	g_object_unref (metadata);

	metadata = tepl_metadata_new ();
	tepl_metadata_manager_copy_from (manager, location, metadata);
	check_get (metadata, "keyA", "valueA");
	check_get (metadata, "keyB", "valueB");

	g_object_unref (metadata);
	g_object_unref (location);
	_tepl_metadata_manager_unref_singleton ();
}

/* Store metadata into the TeplMetadataManager for several locations. */
static void
test_merge_into_and_copy_from_part3 (void)
{
	TeplMetadataManager *manager;
	TeplMetadata *metadataA;
	TeplMetadata *metadataB;
	GFile *locationA;
	GFile *locationB;

	manager = tepl_metadata_manager_get_singleton ();
	locationA = g_file_new_for_path ("locationA");
	locationB = g_file_new_for_path ("locationB");

	metadataA = tepl_metadata_new ();
	tepl_metadata_set (metadataA, "key", "valueA");
	tepl_metadata_manager_merge_into (manager, locationA, metadataA);
	g_object_unref (metadataA);

	metadataB = tepl_metadata_new ();
	tepl_metadata_set (metadataB, "key", "valueB");
	tepl_metadata_manager_merge_into (manager, locationB, metadataB);
	g_object_unref (metadataB);

	metadataA = tepl_metadata_new ();
	tepl_metadata_manager_copy_from (manager, locationA, metadataA);
	check_get (metadataA, "key", "valueA");
	g_object_unref (metadataA);

	metadataB = tepl_metadata_new ();
	tepl_metadata_manager_copy_from (manager, locationB, metadataB);
	check_get (metadataB, "key", "valueB");
	g_object_unref (metadataB);

	g_object_unref (locationA);
	g_object_unref (locationB);
	_tepl_metadata_manager_unref_singleton ();
}

static void
check_load_from_disk (const gchar *filename,
		      gboolean     expect_to_succeed)
{
	TeplMetadataManager *manager;
	GFile *file;
	GError *error = NULL;

	manager = tepl_metadata_manager_get_singleton ();

	file = get_store_file_for_test_data_filename (filename);
	tepl_metadata_manager_load_from_disk (manager, file, &error);
	if (expect_to_succeed)
	{
		g_assert_no_error (error);
	}
	else
	{
		g_assert_true (error != NULL);
		g_error_free (error);
	}

	g_object_unref (file);
	_tepl_metadata_manager_unref_singleton ();
}

static void
test_load_from_disk_expected_to_fail (void)
{
	check_load_from_disk ("expected-to-fail-00-empty.xml", FALSE);
	check_load_from_disk ("expected-to-fail-01.xml", FALSE);
	check_load_from_disk ("expected-to-fail-02.xml", FALSE);
	check_load_from_disk ("expected-to-fail-03.xml", FALSE);
	check_load_from_disk ("expected-to-fail-04.xml", FALSE);
	check_load_from_disk ("expected-to-fail-05.xml", FALSE);
	check_load_from_disk ("expected-to-fail-06.xml", FALSE);
	check_load_from_disk ("expected-to-fail-07-garbage.xml", FALSE);
}

static void
test_load_from_disk_expected_to_succeed (void)
{
	check_load_from_disk ("expected-to-succeed-01-trim-before.xml", TRUE);
	check_load_from_disk ("expected-to-succeed-01-trim-after.xml", TRUE);
}

static void
check_value_round_trip (const gchar *value)
{
	TeplMetadataManager *manager;
	GFile *location;
	GFile *store_file;
	TeplMetadata *metadata;
	GError *error = NULL;

	manager = tepl_metadata_manager_get_singleton ();
	location = g_file_new_for_path ("location");

	/* Set value */
	metadata = tepl_metadata_new ();
	tepl_metadata_set (metadata, "key", value);
	tepl_metadata_manager_merge_into (manager, location, metadata);
	g_object_unref (metadata);

	/* Save to disk */
	store_file = save_metadata_manager ();
	_tepl_metadata_manager_unref_singleton ();

	/* Load from disk */
	manager = tepl_metadata_manager_get_singleton ();
	tepl_metadata_manager_load_from_disk (manager, store_file, &error);
	g_assert_no_error (error);

	/* Read value after round-trip */
	metadata = tepl_metadata_new ();
	tepl_metadata_manager_copy_from (manager, location, metadata);
	check_get (metadata, "key", value);
	g_object_unref (metadata);

	g_object_unref (location);
	g_object_unref (store_file);
	_tepl_metadata_manager_unref_singleton ();
}

static void
test_value_round_trip (void)
{
	check_value_round_trip ("");
	check_value_round_trip ("a");
	check_value_round_trip ("1");
	check_value_round_trip ("Évo-UTF-8");
	check_value_round_trip (",");
	check_value_round_trip (";");
	check_value_round_trip (" ");
	check_value_round_trip ("\t");
	check_value_round_trip ("  \t\t \t\noh");
	check_value_round_trip ("123ASCIIabc.,;/_-:");
}

static void
test_trim (void)
{
	TeplMetadataManager *manager;
	GFile *file_before;
	GFile *file_after;
	GFile *expected_file_after;
	GError *error = NULL;

	manager = tepl_metadata_manager_get_singleton ();
	file_before = get_store_file_for_test_data_filename ("expected-to-succeed-01-trim-before.xml");
	tepl_metadata_manager_load_from_disk (manager, file_before, &error);
	g_assert_no_error (error);

	tepl_metadata_manager_trim (manager, 1);

	file_after = save_metadata_manager ();
	expected_file_after = get_store_file_for_test_data_filename ("expected-to-succeed-01-trim-after.xml");
	_tepl_test_utils_check_equal_files_content (file_after, expected_file_after);

	g_object_unref (file_before);
	g_object_unref (file_after);
	g_object_unref (expected_file_after);
	_tepl_metadata_manager_unref_singleton ();
}

int
main (int    argc,
      char **argv)
{
	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/metadata_manager/merge_into_and_copy_from_part1", test_merge_into_and_copy_from_part1);
	g_test_add_func ("/metadata_manager/merge_into_and_copy_from_part2", test_merge_into_and_copy_from_part2);
	g_test_add_func ("/metadata_manager/merge_into_and_copy_from_part3", test_merge_into_and_copy_from_part3);
	g_test_add_func ("/metadata_manager/load_from_disk_expected_to_fail", test_load_from_disk_expected_to_fail);
	g_test_add_func ("/metadata_manager/load_from_disk_expected_to_succeed", test_load_from_disk_expected_to_succeed);
	g_test_add_func ("/metadata_manager/value_round_trip", test_value_round_trip);
	g_test_add_func ("/metadata_manager/trim", test_trim);

	return g_test_run ();
}
