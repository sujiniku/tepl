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
#define MAX_SIZE 10000
#define CHUNK_SIZE 1024

typedef struct _TestData TestData;
struct _TestData
{
	gchar *expected_buffer_content;
	GQuark expected_error_domain;
	gint expected_error_code;
};

static TestData *
test_data_new (const gchar *expected_buffer_content,
	       GQuark       expected_error_domain,
	       gint         expected_error_code)
{
	TestData *data;

	g_assert (expected_buffer_content != NULL);

	data = g_new0 (TestData, 1);
	data->expected_buffer_content = g_strdup (expected_buffer_content);
	data->expected_error_domain = expected_error_domain;
	data->expected_error_code = expected_error_code;

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

	if (data->expected_error_domain == 0)
	{
		g_assert_no_error (error);
	}
	else
	{
		g_assert (g_error_matches (error,
					   data->expected_error_domain,
					   data->expected_error_code));
		g_clear_error (&error);
	}

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
	     const gchar *expected_buffer_content,
	     GQuark       expected_error_domain,
	     gint         expected_error_code,
	     gboolean     implicit_trailing_newline,
	     gint64       max_size)
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
	gtk_source_buffer_set_implicit_trailing_newline (GTK_SOURCE_BUFFER (buffer),
							 implicit_trailing_newline);

	file = gtef_buffer_get_file (buffer);

	path = g_build_filename (g_get_tmp_dir (), "gtef-test-file-loader", NULL);
	g_file_set_contents (path, contents, -1, &error);
	g_assert_no_error (error);

	location = g_file_new_for_path (path);
	gtef_file_set_location (file, location);

	data = test_data_new (expected_buffer_content,
			      expected_error_domain,
			      expected_error_code);

	loader = gtef_file_loader_new (buffer);
	gtef_file_loader_set_max_size (loader, max_size);
	gtef_file_loader_set_chunk_size (loader, CHUNK_SIZE);

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
test_loader_implicit_trailing_newline (const gchar *contents,
				       const gchar *expected_buffer_content,
				       gboolean     implicit_trailing_newline)
{
	test_loader (contents,
		     expected_buffer_content,
		     0, 0,
		     implicit_trailing_newline,
		     MAX_SIZE);
}

static void
test_implicit_trailing_newline (void)
{
	test_loader_implicit_trailing_newline (DEFAULT_CONTENTS, DEFAULT_CONTENTS, TRUE);
	test_loader_implicit_trailing_newline (DEFAULT_CONTENTS "\n", DEFAULT_CONTENTS, TRUE);
	test_loader_implicit_trailing_newline (DEFAULT_CONTENTS "\r", DEFAULT_CONTENTS, TRUE);
	test_loader_implicit_trailing_newline (DEFAULT_CONTENTS "\r\n", DEFAULT_CONTENTS, TRUE);

	test_loader_implicit_trailing_newline (DEFAULT_CONTENTS, DEFAULT_CONTENTS, FALSE);
	test_loader_implicit_trailing_newline (DEFAULT_CONTENTS "\n", DEFAULT_CONTENTS "\n", FALSE);
	test_loader_implicit_trailing_newline (DEFAULT_CONTENTS "\r", DEFAULT_CONTENTS "\r", FALSE);
	test_loader_implicit_trailing_newline (DEFAULT_CONTENTS "\r\n", DEFAULT_CONTENTS "\r\n", FALSE);
}

static gchar *
generate_contents (gint n_bytes)
{
	gchar *contents;
	gint n_lines = n_bytes / 10;
	const gchar *line = "123456789\n";
	gint i;

	g_assert (n_bytes % 10 == 0);

	contents = g_malloc (n_bytes + 1);
	for (i = 0; i < n_lines; i++)
	{
		memcpy (contents + i*10, line, 10);
	}
	contents[n_bytes] = '\0';

	return contents;
}

static void
test_max_size (void)
{
	gchar *contents;

	contents = generate_contents (MAX_SIZE + 10);

	/* Unlimited */
	test_loader (contents, contents, 0, 0, FALSE, -1);

	/* Too big */
	test_loader (contents,
		     "",
		     GTEF_FILE_LOADER_ERROR,
		     GTEF_FILE_LOADER_ERROR_TOO_BIG,
		     FALSE,
		     MAX_SIZE);

	g_free (contents);

	/* Exactly max size */
	contents = generate_contents (MAX_SIZE);
	test_loader (contents, contents, 0, 0, FALSE, MAX_SIZE);
	g_free (contents);
}

gint
main (gint   argc,
      gchar *argv[])
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/file-loader/implicit-trailing-newline", test_implicit_trailing_newline);
	g_test_add_func ("/file-loader/max-size", test_max_size);

	return g_test_run ();
}
