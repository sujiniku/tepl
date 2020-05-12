/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <tepl/tepl.h>

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
check_equal_content (GFile       *file,
		     const gchar *content)
{
	gchar *file_content;

	file_content = get_file_content (file);
	g_assert_true (g_str_equal (file_content, content));
	g_free (file_content);
}

static void
check_save_content_cb (GObject      *source_object,
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
check_save_content (const gchar *content)
{
	TeplBuffer *buffer;
	TeplFile *file;
	GFile *location;
	TeplFileSaver *saver;

	buffer = tepl_buffer_new ();
	gtk_text_buffer_set_text (GTK_TEXT_BUFFER (buffer), content, -1);

	file = tepl_file_new ();
	location = g_file_new_build_filename (g_get_tmp_dir (), "tepl-file-saver-test", NULL);
	saver = tepl_file_saver_new_with_target (buffer, file, location);

	tepl_file_saver_save_async (saver,
				    G_PRIORITY_DEFAULT,
				    NULL,
				    check_save_content_cb,
				    NULL);
	gtk_main ();

	check_equal_content (location, content);

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

int
main (int    argc,
      char **argv)
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/file_saver/basic", test_basic);

	return g_test_run ();
}
