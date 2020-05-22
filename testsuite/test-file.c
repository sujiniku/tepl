/* SPDX-FileCopyrightText: 2016-2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "config.h"
#include <tepl/tepl.h>
#include <glib/gi18n-lib.h>
#include "tepl-test-utils.h"

typedef struct _WaitSignalData WaitSignalData;
struct _WaitSignalData
{
	guint signal_received : 1;
	guint nested_main_loop : 1;
};

static void
wait_signal_cb (WaitSignalData *data)
{
	data->signal_received = TRUE;

	if (data->nested_main_loop)
	{
		gtk_main_quit ();
	}
}

static WaitSignalData *
wait_signal_setup (GObject     *object,
		   const gchar *detailed_signal_name)
{
	WaitSignalData *data;

	data = g_new0 (WaitSignalData, 1);
	data->signal_received = FALSE;
	data->nested_main_loop = FALSE;

	g_signal_connect_swapped (object,
				  detailed_signal_name,
				  G_CALLBACK (wait_signal_cb),
				  data);

	return data;
}

static void
wait_signal (WaitSignalData *data)
{
	if (!data->signal_received)
	{
		data->nested_main_loop = TRUE;
		gtk_main ();
	}

	g_free (data);
}

static void
check_short_name (TeplFile    *file,
		  const gchar *expected_short_name)
{
	gchar *received_short_name;

	received_short_name = tepl_file_get_short_name (file);
	g_assert_cmpstr (received_short_name, ==, expected_short_name);
	g_free (received_short_name);
}

static void
check_short_name_is_untitled_file_number (TeplFile *file,
					  gint      untitled_number)
{
	gchar *expected_short_name;

	/* For the translation it needs to be the exact same string as in the
	 * TeplFile implementation, to be able to run the unit test with any
	 * locale.
	 */
	expected_short_name = g_strdup_printf (_("Untitled File %d"), untitled_number);
	check_short_name (file, expected_short_name);
	g_free (expected_short_name);
}

static void
test_untitled_files (void)
{
	TeplFile *file1;
	TeplFile *file2;
	TeplFile *file3;
	GFile *location;

	file1 = tepl_file_new ();
	check_short_name_is_untitled_file_number (file1, 1);

	file2 = tepl_file_new ();
	check_short_name_is_untitled_file_number (file2, 2);

	/* Release an untitled number by destroying a file. */
	g_object_unref (file1);
	check_short_name_is_untitled_file_number (file2, 2); // still the same.
	file1 = tepl_file_new ();
	check_short_name_is_untitled_file_number (file1, 1);

	/* Release an untitled number by setting a location. */
	location = g_file_new_for_path ("location");
	tepl_file_set_location (file1, location);
	check_short_name_is_untitled_file_number (file2, 2); // still the same.
	file3 = tepl_file_new ();
	check_short_name_is_untitled_file_number (file3, 1);

	/* Reset location to NULL. */
	tepl_file_set_location (file1, NULL);
	check_short_name_is_untitled_file_number (file1, 3);

	g_object_unref (file1);
	g_object_unref (file2);
	g_object_unref (file3);
	g_object_unref (location);
}

static void
test_short_name (void)
{
	TeplFile *file;
	GFile *location;
	WaitSignalData *data;
	GError *error = NULL;

	location = g_file_new_build_filename (g_get_tmp_dir (), "tepl-test-file", NULL);

	/* Get the fallback short-name, for a file that doesn't exist. */
	g_file_delete (location, NULL, &error);
	if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND))
	{
		g_clear_error (&error);
	}
	g_assert_no_error (error);

	file = tepl_file_new ();
	data = wait_signal_setup (G_OBJECT (file), "notify::short-name");
	tepl_file_set_location (file, location);
	wait_signal (data);
	check_short_name (file, "tepl-test-file");
	g_object_unref (file);

	/* Get the display-name for a local file (so the file must exist). */
	_tepl_test_utils_set_file_content (location, "file content");

	file = tepl_file_new ();
	data = wait_signal_setup (G_OBJECT (file), "notify::short-name");
	tepl_file_set_location (file, location);
	wait_signal (data);
	check_short_name (file, "tepl-test-file");
	g_object_unref (file);
	g_object_unref (location);

	/* Test the special case for a remote location that has no parent GFile. */
	location = g_file_new_for_uri ("https://swilmet.be");
	file = tepl_file_new ();
	data = wait_signal_setup (G_OBJECT (file), "notify::short-name");
	tepl_file_set_location (file, location);
	wait_signal (data);
	check_short_name (file, "https://swilmet.be");
	g_object_unref (file);
	g_object_unref (location);

	// It's not really important if the trailing slash is still present or
	// not in the short-name, but test it anyway. The important thing is to
	// have the https://swilmet.be prefix, with an optional trailing slash.
	location = g_file_new_for_uri ("https://swilmet.be/");
	file = tepl_file_new ();
	data = wait_signal_setup (G_OBJECT (file), "notify::short-name");
	tepl_file_set_location (file, location);
	wait_signal (data);
	check_short_name (file, "https://swilmet.be/");
	g_object_unref (file);
	g_object_unref (location);
}

#if 0
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
#endif

int
main (int    argc,
      char **argv)
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/file/untitled_files", test_untitled_files);
	g_test_add_func ("/file/short_name", test_short_name);

#if 0
	g_test_add_func ("/file/externally_modified", test_externally_modified);
#endif

	return g_test_run ();
}
