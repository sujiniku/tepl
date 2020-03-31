/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2020 - Sébastien Wilmet <swilmet@gnome.org>
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

static GFile *
get_store_file_for_test_data_filename (const gchar *test_data_filename,
				       gboolean     should_exist)
{
	GFile *store_file;

	store_file = g_file_new_build_filename (UNIT_TESTS_SOURCE_DIR,
						"test-metadata-store",
						test_data_filename,
						NULL);
	g_assert_cmpint (should_exist, ==, g_file_query_exists (store_file, NULL));

	return store_file;
}

static gchar *
get_file_content (GFile *file)
{
	GError *error = NULL;
	gchar *file_content = NULL;

	g_file_load_contents (file, NULL, &file_content, NULL, NULL, &error);
	g_assert_no_error (error);
	g_assert_true (file_content != NULL);

	return file_content;
}

static void
check_equal_file_content (GFile *file1,
			  GFile *file2)
{
	gchar *file1_content = get_file_content (file1);
	gchar *file2_content = get_file_content (file2);

	g_assert_true (g_str_equal (file1_content, file2_content));

	g_free (file1_content);
	g_free (file2_content);
}

static void
load_store_file_cb (GObject      *source_object,
		    GAsyncResult *result,
		    gpointer      user_data)
{
	TeplMetadataStore *store = TEPL_METADATA_STORE (source_object);
	GError **error = user_data;

	tepl_metadata_store_load_finish (store, result, error);
	gtk_main_quit ();
}

static void
load_store_file (GFile    *store_file,
		 gboolean  expect_no_error)
{
	TeplMetadataStore *store = tepl_metadata_store_get_singleton ();
	GError *error = NULL;

	tepl_metadata_store_set_store_file (store, store_file);
	tepl_metadata_store_load_async (store,
					G_PRIORITY_DEFAULT,
					NULL,
					load_store_file_cb,
					&error);
	gtk_main ();

	if (expect_no_error)
	{
		g_assert_no_error (error);
	}
	else
	{
		g_assert_true (error != NULL && error->domain == G_MARKUP_ERROR);
		g_clear_error (&error);
	}
}

static void
check_load_test_data_filename (const gchar *test_data_filename)
{
	GFile *store_file;
	gboolean expect_no_error;

	store_file = get_store_file_for_test_data_filename (test_data_filename, TRUE);
	expect_no_error = !g_str_has_prefix (test_data_filename, "expected-to-fail");

	load_store_file (store_file, expect_no_error);

	g_object_unref (store_file);
	_tepl_metadata_store_unref_singleton ();
}

static GFile *
save_store (void)
{
	TeplMetadataStore *store = tepl_metadata_store_get_singleton ();
	GFile *tmp_file;
	GError *error = NULL;

	tmp_file = g_file_new_build_filename (g_get_tmp_dir (),
					      "tepl-metadata-store-test.xml",
					      NULL);
	g_file_delete (tmp_file, NULL, &error);
	if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND))
	{
		g_clear_error (&error);
	}
	g_assert_no_error (error);

	tepl_metadata_store_set_store_file (store, tmp_file);
	tepl_metadata_store_save (store, NULL, &error);
	g_assert_no_error (error);

	return tmp_file;
}

/* To force saving the store file. */
static void
mark_metadata_store_as_modified (void)
{
	TeplMetadataStore *store = tepl_metadata_store_get_singleton ();
	GFile *location;
	GFileInfo *metadata;

	location = g_file_new_for_path ("foo");
	metadata = g_file_info_new ();

	g_assert_true (tepl_metadata_store_get_metadata_for_location (store, location) == NULL);
	tepl_metadata_store_set_metadata_for_location (store, location, metadata);
	tepl_metadata_store_set_metadata_for_location (store, location, NULL);

	g_object_unref (location);
	g_object_unref (metadata);
}

static void
check_metadata_exists (const gchar *uri,
		       const gchar *key,
		       const gchar *expected_value)
{
	TeplMetadataStore *store = tepl_metadata_store_get_singleton ();
	GFile *location;
	GFileInfo *metadata;
	gchar *complete_key;
	const gchar *received_value;

	location = g_file_new_for_uri (uri);
	metadata = tepl_metadata_store_get_metadata_for_location (store, location);
	g_assert_true (metadata != NULL);

	complete_key = g_strconcat ("metadata::", key, NULL);
	g_assert_true (g_file_info_get_attribute_type (metadata, complete_key) == G_FILE_ATTRIBUTE_TYPE_STRING);
	received_value = g_file_info_get_attribute_string (metadata, complete_key);
	g_assert_cmpstr (received_value, ==, expected_value);

	g_object_unref (location);
	g_object_unref (metadata);
	g_free (complete_key);
}

static void
test_expected_to_fail (void)
{
	check_load_test_data_filename ("expected-to-fail-00-empty.xml");
}

static void
test_load_non_existing_store_file (void)
{
	GFile *store_file;

	store_file = get_store_file_for_test_data_filename ("does_not_exist.xml", FALSE);
	load_store_file (store_file, TRUE);

	g_object_unref (store_file);
	_tepl_metadata_store_unref_singleton ();
}

static void
test_empty_store_impl (void)
{
	GFile *tmp_file;
	GFile *store_file;

	/* Empty store, and not modified. */

	tmp_file = save_store ();
	g_assert_true (!g_file_query_exists (tmp_file, NULL));
	g_clear_object (&tmp_file);

	/* Empty store, modified. */

	mark_metadata_store_as_modified ();

	tmp_file = save_store ();
	g_assert_true (g_file_query_exists (tmp_file, NULL));

	store_file = get_store_file_for_test_data_filename ("metadata-tag-only.xml", TRUE);
	check_equal_file_content (store_file, tmp_file);

	g_object_unref (tmp_file);
	g_object_unref (store_file);
	_tepl_metadata_store_unref_singleton ();
}

static void
test_empty_store (void)
{
	GFile *store_file;

	/* Without loading */
	test_empty_store_impl ();

	/* With loading */
	store_file = get_store_file_for_test_data_filename ("metadata-tag-only.xml", TRUE);
	load_store_file (store_file, TRUE);
	g_object_unref (store_file);

	test_empty_store_impl ();
}

static void
test_load_xml_from_old_metadata_manager (void)
{
	TeplMetadataStore *store = tepl_metadata_store_get_singleton ();
	GFile *store_file;
	GFile *tmp_file;
	GFile *reference_saved_store_file;
	GFile *location;

	/* See the first line of the file, TeplMetadataStore doesn't print such
	 * line.
	 */
	store_file = get_store_file_for_test_data_filename ("from-old-metadata-manager.xml", TRUE);
	load_store_file (store_file, TRUE);

	check_metadata_exists ("file:///home/seb/test-header.csv", "gcsvedit-title-line", "4");
	check_metadata_exists ("file:///home/seb/test-header.csv", "gcsvedit-delimiter", ",");
	check_metadata_exists ("file:///home/seb/test-tab.tsv", "gcsvedit-delimiter", "\t");

	/* Start again from a not-modified state (the atime values have been
	 * updated above).
	 */
	_tepl_metadata_store_unref_singleton ();
	store = tepl_metadata_store_get_singleton ();
	load_store_file (store_file, TRUE);
	g_clear_object (&store_file);

	/* Keep only one <document> with one <entry>, so when we save it, we are
	 * sure that the XML file content will be the same (otherwise we are not
	 * sure of the order of the <document> elements and the order of the
	 * <entry> elements).
	 * Do not modify the atime of the other <document>.
	 */
	location = g_file_new_for_uri ("file:///home/seb/test-header.csv");
	tepl_metadata_store_set_metadata_for_location (store, location, NULL);
	g_clear_object (&location);

	tmp_file = save_store ();
	reference_saved_store_file = get_store_file_for_test_data_filename ("gcsvedit-one-entry.xml", TRUE);
	check_equal_file_content (reference_saved_store_file, tmp_file);
	g_object_unref (tmp_file);
	g_object_unref (reference_saved_store_file);

	_tepl_metadata_store_unref_singleton ();
}

static void
test_load_gcsvedit_one_entry (void)
{
	GFile *store_file;

	store_file = get_store_file_for_test_data_filename ("gcsvedit-one-entry.xml", TRUE);
	load_store_file (store_file, TRUE);

#if 0
	/* FIXME: GLib GMarkup bug? It's a tab character in the XML file, not a
	 * space.
	 * https://gitlab.gnome.org/GNOME/glib/-/issues/2080
	 */
	check_metadata_exists ("file:///home/seb/test-tab.tsv", "gcsvedit-delimiter", "\t");
#else
	check_metadata_exists ("file:///home/seb/test-tab.tsv", "gcsvedit-delimiter", " ");
#endif

	g_object_unref (store_file);
	_tepl_metadata_store_unref_singleton ();
}

static void
test_generate_new_store_file_simple (void)
{
	TeplMetadataStore *store = tepl_metadata_store_get_singleton ();
	GFile *location;
	GFileInfo *metadata;
	GFile *tmp_file;
	gchar *current_dir;
	gchar *relatively_uri;

	location = g_file_new_for_path ("/my_absolute_file_absolutely");
	metadata = g_file_info_new ();
	g_file_info_set_attribute_string (metadata, "metadata::my_key", "my_value");
	tepl_metadata_store_set_metadata_for_location (store, location, metadata);
	g_object_unref (location);
	g_object_unref (metadata);

	location = g_file_new_for_path ("a_relative_path_relatively");
	metadata = g_file_info_new ();
	g_file_info_set_attribute_string (metadata, "metadata::a_key", "a_value");
	g_file_info_set_attribute_string (metadata, "metadata::another_key", "another_value");
	tepl_metadata_store_set_metadata_for_location (store, location, metadata);
	g_object_unref (location);
	g_object_unref (metadata);

	tmp_file = save_store ();

	/* Reload the store file that we have just saved. */
	_tepl_metadata_store_unref_singleton ();
	store = tepl_metadata_store_get_singleton ();
	load_store_file (tmp_file, TRUE);
	g_object_unref (tmp_file);

	current_dir = g_get_current_dir ();
	relatively_uri = g_strconcat ("file://", current_dir, "/a_relative_path_relatively", NULL);

	check_metadata_exists ("file:///my_absolute_file_absolutely", "my_key", "my_value");
	check_metadata_exists (relatively_uri, "a_key", "a_value");
	check_metadata_exists (relatively_uri, "another_key", "another_value");

	g_free (current_dir);
	g_free (relatively_uri);
	_tepl_metadata_store_unref_singleton ();
}

static void
test_generate_new_store_file (void)
{
	TeplMetadataStore *store = tepl_metadata_store_get_singleton ();
	GFile *location;
	GFileInfo *metadata;
	GFile *tmp_file;

	/* Test with non-ASCII chars and XML special character escape/unescape. */
	location = g_file_new_for_path ("/home/seb/santé/pandémie-coronavirus-stats.csv");
	metadata = g_file_info_new ();
	g_file_info_set_attribute_string (metadata, "metadata::CLÉ", "Évolution \"<=>\"");
	tepl_metadata_store_set_metadata_for_location (store, location, metadata);
	g_object_unref (location);
	g_object_unref (metadata);

	tmp_file = save_store ();

	/* Reload the store file that we have just saved. */
	_tepl_metadata_store_unref_singleton ();
	store = tepl_metadata_store_get_singleton ();
	load_store_file (tmp_file, TRUE);
	g_object_unref (tmp_file);

	check_metadata_exists ("file:///home/seb/santé/pandémie-coronavirus-stats.csv", "CLÉ", "Évolution \"<=>\"");

	_tepl_metadata_store_unref_singleton ();
}

static void
test_markup_unescape_escape (void)
{
	GFile *store_file;
	GFile *tmp_file;

	store_file = get_store_file_for_test_data_filename ("one-entry-markup-escape.xml", TRUE);
	load_store_file (store_file, TRUE);

	check_metadata_exists ("file:///home/seb/santé/pandémie-coronavirus-stats.csv", "CLÉ", "Évolution \"<=>\"");

	_tepl_metadata_store_unref_singleton ();
	load_store_file (store_file, TRUE);

	mark_metadata_store_as_modified ();
	tmp_file = save_store ();
	check_equal_file_content (tmp_file, store_file);

	g_object_unref (store_file);
	g_object_unref (tmp_file);
	_tepl_metadata_store_unref_singleton ();
}

static void
test_max_number_of_locations (void)
{
	TeplMetadataStore *store = tepl_metadata_store_get_singleton ();
	GFile *before;
	GFile *after;
	GFile *tmp_file;

	tepl_metadata_store_set_max_number_of_locations (store, 1);

	before = get_store_file_for_test_data_filename ("max-num-locations-before.xml", TRUE);
	after = get_store_file_for_test_data_filename ("max-num-locations-after.xml", TRUE);

	load_store_file (before, TRUE);

	mark_metadata_store_as_modified ();
	tmp_file = save_store ();
	check_equal_file_content (tmp_file, after);

	g_object_unref (before);
	g_object_unref (after);
	g_object_unref (tmp_file);
	_tepl_metadata_store_unref_singleton ();
}

int
main (int    argc,
      char **argv)
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/metadata_store/expected_to_fail", test_expected_to_fail);
	g_test_add_func ("/metadata_store/load_non_existing_store_file", test_load_non_existing_store_file);
	g_test_add_func ("/metadata_store/empty_store", test_empty_store);
	g_test_add_func ("/metadata_store/load_xml_from_old_metadata_manager", test_load_xml_from_old_metadata_manager);
	g_test_add_func ("/metadata_store/load_gcsvedit_one_entry", test_load_gcsvedit_one_entry);
	g_test_add_func ("/metadata_store/generate_new_store_file_simple", test_generate_new_store_file_simple);
	g_test_add_func ("/metadata_store/generate_new_store_file", test_generate_new_store_file);
	g_test_add_func ("/metadata_store/markup_unescape_escape", test_markup_unescape_escape);
	g_test_add_func ("/metadata_store/max_number_of_locations", test_max_number_of_locations);

	return g_test_run ();
}
