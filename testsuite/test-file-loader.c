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

#define DEFAULT_CONTENTS "My shiny content!\n"

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
	GError *error = NULL;
	GtkTextBuffer *buffer;
	GtkTextIter start;
	GtkTextIter end;
	gchar *buffer_contents;
	GFile *location;

	gtef_file_loader_load_finish (loader, result, &error);
	g_assert_no_error (error);

	buffer = GTK_TEXT_BUFFER (gtef_file_loader_get_buffer (loader));
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	buffer_contents = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
	g_assert_cmpstr (buffer_contents, ==, DEFAULT_CONTENTS);

	check_buffer_state (buffer);

	location = gtef_file_loader_get_location (loader);
	g_file_delete (location, NULL, &error);
	g_assert_no_error (error);

	g_free (buffer_contents);
	g_object_unref (loader);

	gtk_main_quit ();
}

static void
test_basic (void)
{
	GtefBuffer *buffer;
	GtefFile *file;
	gchar *path;
	GFile *location;
	GtefFileLoader *loader;
	GError *error = NULL;

	buffer = gtef_buffer_new ();
	gtk_text_buffer_set_text (GTK_TEXT_BUFFER (buffer), "Previous content, must be emptied.", -1);

	file = gtef_buffer_get_file (buffer);

	path = g_build_filename (g_get_tmp_dir (), "gtef-test-file-loader", NULL);
	g_file_set_contents (path, DEFAULT_CONTENTS, -1, &error);
	g_assert_no_error (error);

	location = g_file_new_for_path (path);
	gtef_file_set_location (file, location);

	loader = gtef_file_loader_new (buffer);

	gtef_file_loader_load_async (loader,
				     G_PRIORITY_DEFAULT,
				     NULL,
				     load_cb,
				     NULL);

	gtk_main ();

	g_free (path);
	g_object_unref (buffer);
	g_object_unref (location);
}

gint
main (gint   argc,
      gchar *argv[])
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/file-loader/basic", test_basic);

	return g_test_run ();
}
