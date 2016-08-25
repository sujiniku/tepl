/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * Gtef is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * Gtef is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <gtef/gtef.h>

#define DEFAULT_CONTENTS "My shiny content!"

typedef struct _TestData TestData;
struct _TestData
{
	gchar *expected_buffer_content;
};

static TestData *
test_data_new (const gchar *expected_buffer_content)
{
	TestData *data;

	g_assert (expected_buffer_content != NULL);

	data = g_new0 (TestData, 1);
	data->expected_buffer_content = g_strdup (expected_buffer_content);

	return data;
}

static void
test_data_free (TestData *data)
{
	if (data != NULL)
	{
		g_free (data->expected_buffer_content);
		g_free (data);
	}
}

static void
check_buffer_state (GtkTextBuffer *buffer)
{
	GtkTextIter selection_start;
	GtkTextIter selection_end;
	gint offset;
	gboolean modified;

	gtk_text_buffer_get_selection_bounds (buffer,
					      &selection_start,
					      &selection_end);

	offset = gtk_text_iter_get_offset (&selection_start);
	g_assert_cmpint (offset, ==, 0);

	offset = gtk_text_iter_get_offset (&selection_end);
	g_assert_cmpint (offset, ==, 0);

	modified = gtk_text_buffer_get_modified (buffer);
	g_assert (!modified);
}

static void
load_cb (GObject      *source_object,
	 GAsyncResult *result,
	 gpointer      user_data)
{
	GtefFileLoader *loader = GTEF_FILE_LOADER (source_object);
	TestData *data = user_data;
	GtkTextBuffer *buffer;
	GtkTextIter start;
	GtkTextIter end;
	gchar *buffer_contents;
	GFile *location;
	GError *error = NULL;

	gtef_file_loader_load_finish (loader, result, &error);
	g_assert_no_error (error);

	buffer = GTK_TEXT_BUFFER (gtef_file_loader_get_buffer (loader));
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	buffer_contents = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
	g_assert_cmpstr (buffer_contents, ==, data->expected_buffer_content);

	check_buffer_state (buffer);

	location = gtef_file_loader_get_location (loader);
	g_file_delete (location, NULL, &error);
	g_assert_no_error (error);

	g_free (buffer_contents);
	g_object_unref (loader);

	gtk_main_quit ();
}

static void
test_loader (const gchar *contents,
	     const gchar *expected_buffer_content)
{
	GtefBuffer *buffer;
	GtefFile *file;
	gchar *path;
	GFile *location;
	GtefFileLoader *loader;
	TestData *data;
	GError *error = NULL;

	buffer = gtef_buffer_new ();
	gtk_text_buffer_set_text (GTK_TEXT_BUFFER (buffer), "Previous contents, must be emptied.", -1);

	file = gtef_buffer_get_file (buffer);

	path = g_build_filename (g_get_tmp_dir (), "gtef-test-file-loader", NULL);
	g_file_set_contents (path, contents, -1, &error);
	g_assert_no_error (error);

	location = g_file_new_for_path (path);
	gtef_file_set_location (file, location);

	data = test_data_new (expected_buffer_content);

	loader = gtef_file_loader_new (buffer);

	gtef_file_loader_load_async (loader,
				     G_PRIORITY_DEFAULT,
				     NULL,
				     load_cb,
				     data);

	gtk_main ();

	g_free (path);
	g_object_unref (buffer);
	g_object_unref (location);
	test_data_free (data);
}

static void
test_implicit_trailing_newline (void)
{
	test_loader (DEFAULT_CONTENTS, DEFAULT_CONTENTS);
	test_loader (DEFAULT_CONTENTS "\n", DEFAULT_CONTENTS);
	test_loader (DEFAULT_CONTENTS "\r", DEFAULT_CONTENTS);
	test_loader (DEFAULT_CONTENTS "\r\n", DEFAULT_CONTENTS);
}

gint
main (gint   argc,
      gchar *argv[])
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/file-loader/implicit-trailing-newline", test_implicit_trailing_newline);

	return g_test_run ();
}
