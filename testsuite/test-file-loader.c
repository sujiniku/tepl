/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <tepl/tepl.h>
#include <string.h>
#include "tepl-test-utils.h"

static GFile *
get_tmp_location (void)
{
	return g_file_new_build_filename (g_get_tmp_dir (), "tepl-file-loader-test", NULL);
}

static void
load_sync_cb (GObject      *source_object,
	      GAsyncResult *result,
	      gpointer      user_data)
{
	TeplFileLoader *loader = TEPL_FILE_LOADER (source_object);
	GError **error = user_data;

	tepl_file_loader_load_finish (loader, result, error);
	gtk_main_quit ();
}

static void
load_sync (TeplFileLoader  *loader,
	   GError         **error)
{
	tepl_file_loader_load_async (loader,
				     G_PRIORITY_DEFAULT,
				     NULL,
				     load_sync_cb,
				     error);
	gtk_main ();
}

static void
load_sync_expect_no_error (TeplFileLoader *loader)
{
	GError *error = NULL;

	load_sync (loader, &error);
	g_assert_no_error (error);
}

/* Useful to check that the buffer has been reset after the load. */
static TeplBuffer *
create_buffer (void)
{
	TeplBuffer *buffer = tepl_buffer_new ();
	GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (buffer);
	GtkSourceBuffer *gsv_buffer = GTK_SOURCE_BUFFER (buffer);

	gtk_text_buffer_set_text (text_buffer, "Initial content", -1);
	g_assert_true (gtk_source_buffer_can_undo (gsv_buffer));

	return buffer;
}

static void
check_buffer_state_after_load (TeplBuffer  *buffer,
			       const gchar *expected_content)
{
	GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (buffer);
	GtkSourceBuffer *gsv_buffer = GTK_SOURCE_BUFFER (buffer);
	GtkTextIter start;
	GtkTextIter end;
	GtkTextIter selection_start;
	GtkTextIter selection_end;
	gchar *received_content;

	gtk_text_buffer_get_bounds (text_buffer, &start, &end);
	received_content = gtk_text_buffer_get_text (text_buffer, &start, &end, TRUE);
	g_assert_true (g_str_equal (received_content, expected_content));
	g_free (received_content);

	g_assert_true (!gtk_text_buffer_get_modified (text_buffer));

	g_assert_true (!gtk_source_buffer_can_undo (gsv_buffer));
	g_assert_true (!gtk_source_buffer_can_redo (gsv_buffer));

	gtk_text_buffer_get_selection_bounds (text_buffer, &selection_start, &selection_end);
	g_assert_true (gtk_text_iter_is_start (&selection_start));
	g_assert_true (gtk_text_iter_is_start (&selection_end));
}

static void
test_properties (void)
{
	TeplBuffer *buffer;
	TeplFile *file;
	GFile *location;
	TeplFileLoader *loader;

	buffer = tepl_buffer_new ();

	file = tepl_buffer_get_file (buffer);
	location = g_file_new_for_path ("location");
	tepl_file_set_location (file, location);

	loader = tepl_file_loader_new (buffer, file);
	g_assert_true (tepl_file_loader_get_buffer (loader) == buffer);
	g_assert_true (tepl_file_loader_get_file (loader) == file);
	g_assert_true (tepl_file_loader_get_location (loader) == location);

	g_object_unref (buffer);
	g_object_unref (location);
	g_object_unref (loader);
}

static void
test_non_existing_file (void)
{
	TeplBuffer *buffer;
	TeplFile *file;
	GFile *location;
	TeplFileLoader *loader;
	GError *error = NULL;

	buffer = create_buffer ();
	file = tepl_buffer_get_file (buffer);

	location = get_tmp_location ();
	g_file_delete (location, NULL, &error);
	if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND))
	{
		g_clear_error (&error);
	}
	g_assert_no_error (error);

	tepl_file_set_location (file, location);
	loader = tepl_file_loader_new (buffer, file);

	load_sync (loader, &error);
	g_assert_error (error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND);
	g_clear_error (&error);

	check_buffer_state_after_load (buffer, "");

	g_object_unref (buffer);
	g_object_unref (location);
	g_object_unref (loader);
}

static void
test_utf8_file (void)
{
	TeplBuffer *buffer;
	TeplFile *file;
	GFile *location;
	const gchar *content;
	TeplFileLoader *loader;

	buffer = create_buffer ();
	file = tepl_buffer_get_file (buffer);

	location = get_tmp_location ();
	content = "Valid UTF-8: ÉÈßÇ";
	_tepl_test_utils_set_file_content (location, content);

	tepl_file_set_location (file, location);
	loader = tepl_file_loader_new (buffer, file);
	load_sync_expect_no_error (loader);

	check_buffer_state_after_load (buffer, content);

	g_object_unref (buffer);
	g_object_unref (location);
	g_object_unref (loader);
}

int
main (int    argc,
      char **argv)
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/file_loader/properties", test_properties);
	g_test_add_func ("/file_loader/non_existing_file", test_non_existing_file);
	g_test_add_func ("/file_loader/utf8_file", test_utf8_file);

	return g_test_run ();
}
