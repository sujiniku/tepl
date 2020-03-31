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
load_store_file (GFile   *store_file,
		 GError **error)
{
	TeplMetadataStore *store = tepl_metadata_store_get_singleton ();

	tepl_metadata_store_set_store_file (store, store_file);
	tepl_metadata_store_load_async (store,
					G_PRIORITY_DEFAULT,
					NULL,
					load_store_file_cb,
					error);
	gtk_main ();
}

static GFile *
save_store (GError **error)
{
	TeplMetadataStore *store = tepl_metadata_store_get_singleton ();
	GFile *tmp_file;
	GError *my_error = NULL;

	tmp_file = g_file_new_build_filename (g_get_tmp_dir (),
					      "tepl-metadata-store-test.xml",
					      NULL);
	g_file_delete (tmp_file, NULL, &my_error);
	if (g_error_matches (my_error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND))
	{
		g_clear_error (&my_error);
	}
	g_assert_no_error (my_error);

	tepl_metadata_store_set_store_file (store, tmp_file);
	tepl_metadata_store_save (store, NULL, error);

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
test_load_non_existing_store_file (void)
{
	GFile *store_file;
	GError *error = NULL;

	store_file = get_store_file_for_test_data_filename ("does_not_exist.xml", FALSE);
	load_store_file (store_file, &error);
	g_assert_no_error (error);

	g_object_unref (store_file);
	_tepl_metadata_store_unref_singleton ();
}

static void
test_load_empty_store_file (void)
{
	GFile *store_file;
	GError *error = NULL;

	store_file = get_store_file_for_test_data_filename ("empty.xml", TRUE);
	load_store_file (store_file, &error);
	g_assert_true (error != NULL && error->domain == G_MARKUP_ERROR);
	g_clear_error (&error);

	g_object_unref (store_file);
	_tepl_metadata_store_unref_singleton ();
}

static void
test_empty_store_impl (void)
{
	GFile *tmp_file;
	GFile *store_file;
	GError *error = NULL;

	/* Empty store, and not modified. */

	tmp_file = save_store (&error);
	g_assert_no_error (error);
	g_assert_true (!g_file_query_exists (tmp_file, NULL));
	g_clear_object (&tmp_file);

	/* Empty store, modified. */

	mark_metadata_store_as_modified ();

	tmp_file = save_store (&error);
	g_assert_no_error (error);
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
	GError *error = NULL;

	/* Without loading */
	test_empty_store_impl ();

	/* With loading */
	store_file = get_store_file_for_test_data_filename ("metadata-tag-only.xml", TRUE);
	load_store_file (store_file, &error);
	g_assert_no_error (error);
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
	GError *error = NULL;

	/* See the first line of the file, TeplMetadataStore doesn't print such
	 * line.
	 */
	store_file = get_store_file_for_test_data_filename ("from-old-metadata-manager.xml", TRUE);
	load_store_file (store_file, &error);
	g_assert_no_error (error);

	check_metadata_exists ("file:///home/seb/test-header.csv", "gcsvedit-title-line", "4");
	check_metadata_exists ("file:///home/seb/test-header.csv", "gcsvedit-delimiter", ",");
	check_metadata_exists ("file:///home/seb/test-tab.tsv", "gcsvedit-delimiter", "\t");

	/* Start again from a not-modified state (the atime values have been
	 * updated above).
	 */
	_tepl_metadata_store_unref_singleton ();
	store = tepl_metadata_store_get_singleton ();
	load_store_file (store_file, &error);
	g_assert_no_error (error);
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

	tmp_file = save_store (&error);
	g_assert_no_error (error);
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
	GError *error = NULL;

	store_file = get_store_file_for_test_data_filename ("gcsvedit-one-entry.xml", TRUE);
	load_store_file (store_file, &error);
	g_assert_no_error (error);

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

int
main (int    argc,
      char **argv)
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/metadata_store/load_non_existing_store_file", test_load_non_existing_store_file);
	g_test_add_func ("/metadata_store/load_empty_store_file", test_load_empty_store_file);
	g_test_add_func ("/metadata_store/empty_store", test_empty_store);
	g_test_add_func ("/metadata_store/load_xml_from_old_metadata_manager", test_load_xml_from_old_metadata_manager);
	g_test_add_func ("/metadata_store/load_gcsvedit_one_entry", test_load_gcsvedit_one_entry);

	return g_test_run ();
}
