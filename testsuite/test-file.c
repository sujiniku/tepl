/* Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
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

static void
sleep_for_one_second (void)
{
	g_usleep (G_USEC_PER_SEC);
}

static void
load_cb (GObject      *source_object,
	 GAsyncResult *result,
	 gpointer      user_data)
{
	TeplFileLoader *loader = TEPL_FILE_LOADER (source_object);
	GError *error = NULL;

	tepl_file_loader_load_finish (loader, result, &error);
	g_assert_no_error (error);

	gtk_main_quit ();
}

static void
load (TeplBuffer *buffer)
{
	TeplFile *file;
	TeplFileLoader *loader;

	file = tepl_buffer_get_file (buffer);

	loader = tepl_file_loader_new (buffer, file);
	tepl_file_loader_load_async (loader,
				     G_PRIORITY_DEFAULT,
				     NULL, /* cancellable */
				     NULL, NULL, NULL, /* progress cb */
				     load_cb,
				     NULL);

	gtk_main ();
	g_object_unref (loader);
}

static void
save_cb (GObject      *source_object,
	 GAsyncResult *result,
	 gpointer      user_data)
{
	TeplFileSaver *saver = TEPL_FILE_SAVER (source_object);
	gboolean expect_externally_modified_error = GPOINTER_TO_INT (user_data);
	GError *error = NULL;

	tepl_file_saver_save_finish (saver, result, &error);
	if (expect_externally_modified_error)
	{
		g_assert_true (g_error_matches (error,
						TEPL_FILE_SAVER_ERROR,
						TEPL_FILE_SAVER_ERROR_EXTERNALLY_MODIFIED));
		g_clear_error (&error);
	}
	else
	{
		g_assert_no_error (error);
	}

	gtk_main_quit ();
}

static void
save (TeplBuffer *buffer,
      gboolean    expect_externally_modified_error)
{
	TeplFile *file;
	TeplFileSaver *saver;

	file = tepl_buffer_get_file (buffer);

	saver = tepl_file_saver_new (buffer, file);
	tepl_file_saver_save_async (saver,
				    G_PRIORITY_DEFAULT,
				    NULL, /* cancellable */
				    NULL, NULL, NULL, /* progress cb */
				    save_cb,
				    GINT_TO_POINTER (expect_externally_modified_error));

	gtk_main ();

	if (expect_externally_modified_error)
	{
		expect_externally_modified_error = FALSE;

		tepl_file_saver_set_flags (saver, TEPL_FILE_SAVER_FLAGS_IGNORE_MODIFICATION_TIME);
		tepl_file_saver_save_async (saver,
					    G_PRIORITY_DEFAULT,
					    NULL, /* cancellable */
					    NULL, NULL, NULL, /* progress cb */
					    save_cb,
					    GINT_TO_POINTER (expect_externally_modified_error));

		gtk_main ();
	}

	g_object_unref (saver);
}

static void
save_as (TeplBuffer *buffer,
	 GFile      *new_location)
{
	TeplFile *file;
	TeplFileSaver *saver;
	gboolean expect_externally_modified_error = FALSE;

	file = tepl_buffer_get_file (buffer);

	saver = tepl_file_saver_new_with_target (buffer, file, new_location);
	tepl_file_saver_save_async (saver,
				    G_PRIORITY_DEFAULT,
				    NULL, /* cancellable */
				    NULL, NULL, NULL, /* progress cb */
				    save_cb,
				    GINT_TO_POINTER (expect_externally_modified_error));

	gtk_main ();
	g_object_unref (saver);
}

static void
test_externally_modified (void)
{
	TeplBuffer *buffer;
	TeplFile *file;
	gchar *path;
	gchar *new_path;
	GFile *location;
	GFile *new_location;
	GError *error = NULL;

	buffer = tepl_buffer_new ();
	file = tepl_buffer_get_file (buffer);

	/* With NULL location */
	g_assert_true (!tepl_file_is_externally_modified (file));
	tepl_file_check_file_on_disk (file);
	g_assert_true (!tepl_file_is_externally_modified (file));

	/* Set location, but not yet loaded or saved */
	path = g_build_filename (g_get_tmp_dir (), "tepl-test-file", NULL);
	g_file_set_contents (path, "a", -1, &error);
	g_assert_no_error (error);

	location = g_file_new_for_path (path);
	tepl_file_set_location (file, location);
	g_assert_true (!tepl_file_is_externally_modified (file));
	tepl_file_check_file_on_disk (file);
	g_assert_true (!tepl_file_is_externally_modified (file));

	/* Load */
	load (buffer);
	g_assert_true (!tepl_file_is_externally_modified (file));
	tepl_file_check_file_on_disk (file);
	g_assert_true (!tepl_file_is_externally_modified (file));

	/* Save */
	save (buffer, FALSE);
	g_assert_true (!tepl_file_is_externally_modified (file));
	tepl_file_check_file_on_disk (file);
	g_assert_true (!tepl_file_is_externally_modified (file));

	/* Modify externally and then save.
	 * Sleep one second to force the timestamp/etag to change.
	 */
	sleep_for_one_second ();
	g_file_set_contents (path, "b", -1, &error);
	g_assert_no_error (error);
	tepl_file_check_file_on_disk (file);
	g_assert_true (tepl_file_is_externally_modified (file));

	save (buffer, TRUE);
	g_assert_true (!tepl_file_is_externally_modified (file));
	tepl_file_check_file_on_disk (file);
	g_assert_true (!tepl_file_is_externally_modified (file));

	/* Modify externally and then load */
	sleep_for_one_second ();
	g_file_set_contents (path, "c", -1, &error);
	g_assert_no_error (error);
	tepl_file_check_file_on_disk (file);
	g_assert_true (tepl_file_is_externally_modified (file));

	load (buffer);
	g_assert_true (!tepl_file_is_externally_modified (file));
	tepl_file_check_file_on_disk (file);
	g_assert_true (!tepl_file_is_externally_modified (file));

	/* Modify externally and then save as */
	sleep_for_one_second ();
	g_file_set_contents (path, "d", -1, &error);
	g_assert_no_error (error);
	tepl_file_check_file_on_disk (file);
	g_assert_true (tepl_file_is_externally_modified (file));

	new_path = g_build_filename (g_get_tmp_dir (), "tepl-test-file-2", NULL);
	g_file_set_contents (new_path, "e", -1, &error);
	g_assert_no_error (error);

	new_location = g_file_new_for_path (new_path);
	save_as (buffer, new_location);
	g_assert_true (g_file_equal (new_location, tepl_file_get_location (file)));
	g_assert_true (!tepl_file_is_externally_modified (file));
	tepl_file_check_file_on_disk (file);
	g_assert_true (!tepl_file_is_externally_modified (file));

	/* Modify externally and then save as to same location */
	sleep_for_one_second ();
	g_file_set_contents (new_path, "f", -1, &error);
	g_assert_no_error (error);
	tepl_file_check_file_on_disk (file);
	g_assert_true (tepl_file_is_externally_modified (file));

	g_assert_true (g_file_equal (new_location, tepl_file_get_location (file)));
	save_as (buffer, new_location);
	g_assert_true (!tepl_file_is_externally_modified (file));
	tepl_file_check_file_on_disk (file);
	g_assert_true (!tepl_file_is_externally_modified (file));

	/* Cleanup */
	g_file_delete (location, NULL, &error);
	g_assert_no_error (error);
	g_file_delete (new_location, NULL, &error);
	g_assert_no_error (error);

	g_free (path);
	g_free (new_path);
	g_object_unref (location);
	g_object_unref (new_location);
	g_object_unref (buffer);
}

gint
main (gint   argc,
      gchar *argv[])
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/file/externally-modified", test_externally_modified);

	return g_test_run ();
}
