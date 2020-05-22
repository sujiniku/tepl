/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <tepl/tepl.h>
#include "tepl-test-utils.h"

static GFile *
get_tmp_location (void)
{
	return g_file_new_build_filename (g_get_tmp_dir (), "tepl-file-saver-test", NULL);
}

static GFile *
get_tmp_backup_location (void)
{
	return g_file_new_build_filename (g_get_tmp_dir (), "tepl-file-saver-test~", NULL);
}

static void
save_sync_cb (GObject      *source_object,
	      GAsyncResult *result,
	      gpointer      user_data)
{
	TeplFileSaver *saver = TEPL_FILE_SAVER (source_object);
	gboolean ok;
	GError *error = NULL;

	ok = tepl_file_saver_save_finish (saver, result, &error);
	g_assert_true (ok);
	g_assert_no_error (error);

	gtk_main_quit ();
}

static void
save_sync (TeplFileSaver *saver)
{
	tepl_file_saver_save_async (saver,
				    G_PRIORITY_DEFAULT,
				    NULL,
				    save_sync_cb,
				    NULL);
	gtk_main ();
}

static void
check_save_content (const gchar *content)
{
	TeplBuffer *buffer;
	TeplFile *file;
	GFile *location;
	TeplFileSaver *saver;

	buffer = tepl_buffer_new ();
	gtk_text_buffer_set_text (GTK_TEXT_BUFFER (buffer), content, -1);

	file = tepl_file_new ();
	location = get_tmp_location ();
	saver = tepl_file_saver_new_with_target (buffer, file, location);

	save_sync (saver);
	_tepl_test_utils_check_file_content (location, content);

	g_object_unref (buffer);
	g_object_unref (file);
	g_object_unref (location);
	g_object_unref (saver);
}

static void
test_basic (void)
{
	check_save_content ("");
	check_save_content ("ho");
	check_save_content ("several\nlines");
	check_save_content ("several\nlines\n");
	check_save_content ("UTF-8-Évo");
}

static void
test_backup (void)
{
	TeplBuffer *buffer;
	TeplFile *file;
	GFile *location;
	GFile *backup_location;
	TeplFileSaver *saver;

	buffer = tepl_buffer_new ();
	gtk_text_buffer_set_text (GTK_TEXT_BUFFER (buffer), "contentA", -1);

	file = tepl_file_new ();
	location = get_tmp_location ();

	saver = tepl_file_saver_new_with_target (buffer, file, location);
	save_sync (saver);
	_tepl_test_utils_check_file_content (location, "contentA");
	g_object_unref (saver);

	gtk_text_buffer_set_text (GTK_TEXT_BUFFER (buffer), "contentB", -1);

	saver = tepl_file_saver_new_with_target (buffer, file, location);
	tepl_file_saver_set_flags (saver, TEPL_FILE_SAVER_FLAGS_CREATE_BACKUP);
	save_sync (saver);
	_tepl_test_utils_check_file_content (location, "contentB");
	g_object_unref (saver);

	backup_location = get_tmp_backup_location ();
	_tepl_test_utils_check_file_content (backup_location, "contentA");

	g_object_unref (buffer);
	g_object_unref (file);
	g_object_unref (location);
	g_object_unref (backup_location);
}

static void
test_properties (void)
{
	TeplBuffer *buffer;
	GtkTextBuffer *text_buffer;
	TeplFile *file;
	GFile *location;
	TeplFileSaver *saver;

	buffer = tepl_buffer_new ();
	text_buffer = GTK_TEXT_BUFFER (buffer);
	file = tepl_file_new ();
	location = get_tmp_location ();

	saver = tepl_file_saver_new_with_target (buffer, file, location);
	g_assert_true (tepl_file_saver_get_buffer (saver) == buffer);
	g_assert_true (tepl_file_saver_get_file (saver) == file);
	g_assert_true (tepl_file_saver_get_location (saver) == location);
	g_assert_cmpint (tepl_file_saver_get_flags (saver), ==, TEPL_FILE_SAVER_FLAGS_NONE);
	g_object_unref (saver);

	tepl_file_set_location (file, location);
	saver = tepl_file_saver_new (buffer, file);
	g_assert_true (tepl_file_saver_get_buffer (saver) == buffer);
	g_assert_true (tepl_file_saver_get_file (saver) == file);
	g_assert_true (tepl_file_saver_get_location (saver) == location);
	g_assert_cmpint (tepl_file_saver_get_flags (saver), ==, TEPL_FILE_SAVER_FLAGS_NONE);

	gtk_text_buffer_set_text (text_buffer, "oh", -1);
	g_assert_true (gtk_text_buffer_get_modified (text_buffer));
	save_sync (saver);
	g_assert_true (!gtk_text_buffer_get_modified (text_buffer));

	g_object_unref (file);
	g_object_unref (saver);

	file = tepl_file_new ();
	g_assert_true (tepl_file_get_location (file) == NULL);
	saver = tepl_file_saver_new_with_target (buffer, file, location);
	g_assert_true (tepl_file_get_location (file) == NULL);
	save_sync (saver);
	g_assert_true (tepl_file_get_location (file) == location);

	g_object_unref (buffer);
	g_object_unref (file);
	g_object_unref (location);
	g_object_unref (saver);
}

int
main (int    argc,
      char **argv)
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/file_saver/basic", test_basic);
	g_test_add_func ("/file_saver/backup", test_backup);
	g_test_add_func ("/file_saver/properties", test_properties);

	return g_test_run ();
}
