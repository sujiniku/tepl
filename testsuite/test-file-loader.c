/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2016-2019 - Sébastien Wilmet <swilmet@gnome.org>
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

#include <string.h>
#include <sys/stat.h>
#include <tepl/tepl.h>
#include "tepl/tepl-file-content.h"

#define DEFAULT_CONTENTS "My shiny content!"
#define MAX_SIZE 10000
#define CHUNK_SIZE 1024

/* linux/bsd has it. others such as Solaris, do not */
#ifndef ACCESSPERMS
#define ACCESSPERMS (S_IRWXU|S_IRWXG|S_IRWXO)
#endif

typedef struct _TestData TestData;
struct _TestData
{
	gchar *expected_buffer_content;
	GQuark expected_error_domain;
	gint expected_error_code;
	TeplEncoding *expected_encoding;
	TeplNewlineType expected_newline_type;

	/* Can be -1 to disable checking the number of lines in the buffer. */
	gint expected_line_count;
};

static TestData *
test_data_new (const gchar     *expected_buffer_content,
	       GQuark           expected_error_domain,
	       gint             expected_error_code,
	       const gchar     *expected_charset,
	       TeplNewlineType  expected_newline_type,
	       gint             expected_line_count)
{
	TestData *data;

	g_assert_true (expected_buffer_content != NULL);

	data = g_new0 (TestData, 1);
	data->expected_buffer_content = g_strdup (expected_buffer_content);
	data->expected_error_domain = expected_error_domain;
	data->expected_error_code = expected_error_code;
	data->expected_newline_type = expected_newline_type;
	data->expected_line_count = expected_line_count;

	if (expected_charset != NULL)
	{
		data->expected_encoding = tepl_encoding_new (expected_charset);
	}

	return data;
}

static void
test_data_free (TestData *data)
{
	if (data != NULL)
	{
		tepl_encoding_free (data->expected_encoding);
		g_free (data->expected_buffer_content);
		g_free (data);
	}
}

static gchar *
generate_content (gsize  n_bytes,
		  gint  *line_count)
{
	gchar *content;
	gsize i;

	if (line_count != NULL)
	{
		*line_count = 1;
	}

	content = g_malloc (n_bytes + 1);

	for (i = 0; i < n_bytes; i++)
	{
		if (i % 80 == 0)
		{
			content[i] = '\n';

			if (line_count != NULL)
			{
				(*line_count)++;
			}
		}
		else
		{
			content[i] = 'a';
		}
	}

	content[n_bytes] = '\0';

	return content;
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
	g_assert_true (!modified);
}

static void
check_equal_encodings (const TeplEncoding *received_enc,
		       const TeplEncoding *expected_enc)
{
	if (!tepl_encoding_equals (received_enc, expected_enc))
	{
		g_warning ("Expected encoding '%s' but received '%s'.",
			   tepl_encoding_get_charset (expected_enc),
			   tepl_encoding_get_charset (received_enc));
	}
}

static void
load_cb (GObject      *source_object,
	 GAsyncResult *result,
	 gpointer      user_data)
{
	TeplFileLoader *loader = TEPL_FILE_LOADER (source_object);
	TestData *data = user_data;
	GtkTextBuffer *buffer;
	GtkTextIter start;
	GtkTextIter end;
	gchar *buffer_contents;
	GFile *location;
	GError *error = NULL;

	tepl_file_loader_load_finish (loader, result, &error);

	buffer = GTK_TEXT_BUFFER (tepl_file_loader_get_buffer (loader));

	if (data->expected_error_domain == 0)
	{
		TeplFile *file;

		g_assert_no_error (error);

		file = tepl_file_loader_get_file (loader);

		g_assert_cmpint (tepl_file_get_compression_type (file), ==, TEPL_COMPRESSION_TYPE_NONE);
		g_assert_cmpint (tepl_file_loader_get_newline_type (loader), ==, data->expected_newline_type);
		g_assert_cmpint (tepl_file_get_newline_type (file), ==, data->expected_newline_type);
		g_assert_true (!tepl_file_is_externally_modified (file));
		g_assert_true (!tepl_file_is_deleted (file));

		if (data->expected_line_count != -1)
		{
			gint line_count;

			line_count = gtk_text_buffer_get_line_count (buffer);
			g_assert_cmpint (line_count, ==, data->expected_line_count);
		}

		if (data->expected_encoding != NULL)
		{
			check_equal_encodings (tepl_file_loader_get_encoding (loader), data->expected_encoding);
			check_equal_encodings (tepl_file_get_encoding (file), data->expected_encoding);
		}
	}
	else
	{
		g_assert_true (g_error_matches (error,
						data->expected_error_domain,
						data->expected_error_code));
		g_clear_error (&error);
	}

	gtk_text_buffer_get_bounds (buffer, &start, &end);
	buffer_contents = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
	g_assert_cmpstr (buffer_contents, ==, data->expected_buffer_content);

	check_buffer_state (buffer);

	location = tepl_file_loader_get_location (loader);
	g_file_delete (location, NULL, &error);
	g_assert_no_error (error);

	g_free (buffer_contents);
	g_object_unref (loader);

	gtk_main_quit ();
}

static void
test_loader (const gchar     *contents,
	     const gchar     *expected_buffer_content,
	     GQuark           expected_error_domain,
	     gint             expected_error_code,
	     const gchar     *expected_charset,
	     TeplNewlineType  expected_newline_type,
	     gint             expected_line_count,
	     gboolean         implicit_trailing_newline,
	     gint64           max_size)
{
	TeplBuffer *buffer;
	TeplFile *file;
	gchar *path;
	GFile *location;
	TeplFileLoader *loader;
	TestData *data;
	GError *error = NULL;

	buffer = tepl_buffer_new ();
	gtk_text_buffer_set_text (GTK_TEXT_BUFFER (buffer), "Previous contents, must be emptied.", -1);
	gtk_source_buffer_set_implicit_trailing_newline (GTK_SOURCE_BUFFER (buffer),
							 implicit_trailing_newline);

	file = tepl_buffer_get_file (buffer);

	path = g_build_filename (g_get_tmp_dir (), "tepl-test-file-loader", NULL);
	g_file_set_contents (path, contents, -1, &error);
	g_assert_no_error (error);

	location = g_file_new_for_path (path);
	tepl_file_set_location (file, location);

	data = test_data_new (expected_buffer_content,
			      expected_error_domain,
			      expected_error_code,
			      expected_charset,
			      expected_newline_type,
			      expected_line_count);

	loader = tepl_file_loader_new (buffer, file);
	tepl_file_loader_set_max_size (loader, max_size);
	tepl_file_loader_set_chunk_size (loader, CHUNK_SIZE);

	tepl_file_loader_load_async (loader,
				     G_PRIORITY_DEFAULT,
				     NULL, /* cancellable */
				     NULL, NULL, NULL, /* progress */
				     load_cb,
				     data);

	gtk_main ();

	g_free (path);
	g_object_unref (buffer);
	g_object_unref (location);
	test_data_free (data);
}

static void
test_empty (void)
{
	/* uchardet returns unknown encoding, this relies on the fallback mode
	 * to determine the encoding.
	 * See uchardet bug:
	 * https://bugs.freedesktop.org/show_bug.cgi?id=103280
	 */
	test_loader ("",
		     "",
		     0, 0,
		     "UTF-8",
		     TEPL_NEWLINE_TYPE_DEFAULT,
		     1,
		     TRUE,
		     MAX_SIZE);
}

static void
test_loader_newlines (gboolean         implicit_trailing_newline,
		      const gchar     *contents,
		      const gchar     *expected_buffer_content,
		      TeplNewlineType  expected_newline_type)
{
	test_loader (contents,
		     expected_buffer_content,
		     0, 0,
		     "ASCII",
		     expected_newline_type,
		     -1,
		     implicit_trailing_newline,
		     MAX_SIZE);
}

static void
test_newlines (void)
{
	test_loader_newlines (TRUE, DEFAULT_CONTENTS, DEFAULT_CONTENTS, TEPL_NEWLINE_TYPE_DEFAULT);
	test_loader_newlines (TRUE, DEFAULT_CONTENTS "\n", DEFAULT_CONTENTS, TEPL_NEWLINE_TYPE_LF);
	test_loader_newlines (TRUE, DEFAULT_CONTENTS "\r", DEFAULT_CONTENTS, TEPL_NEWLINE_TYPE_CR);
	test_loader_newlines (TRUE, DEFAULT_CONTENTS "\r\n", DEFAULT_CONTENTS, TEPL_NEWLINE_TYPE_CR_LF);

	test_loader_newlines (FALSE, DEFAULT_CONTENTS, DEFAULT_CONTENTS, TEPL_NEWLINE_TYPE_DEFAULT);
	test_loader_newlines (FALSE, DEFAULT_CONTENTS "\n", DEFAULT_CONTENTS "\n", TEPL_NEWLINE_TYPE_LF);
	test_loader_newlines (FALSE, DEFAULT_CONTENTS "\r", DEFAULT_CONTENTS "\r", TEPL_NEWLINE_TYPE_CR);
	test_loader_newlines (FALSE, DEFAULT_CONTENTS "\r\n", DEFAULT_CONTENTS "\r\n", TEPL_NEWLINE_TYPE_CR_LF);
}

#if 0
static void
test_loader_split_cr_lf (const gchar *content,
			 gint         expected_line_count)
{
	test_loader (content,
		     content,
		     0, 0,
		     "ASCII",
		     TEPL_NEWLINE_TYPE_LF,
		     expected_line_count,
		     FALSE,
		     -1);
}

static void
test_split_cr_lf (void)
{
	gint64 block_size;
	gchar *content;
	gint expected_line_count;

	block_size = _tepl_file_content_get_encoding_converter_buffer_size ();
	/* Remove terminating nul byte */
	block_size--;

	/* Only \n's. */
	content = generate_content (block_size, &expected_line_count);
	test_loader_split_cr_lf (content, expected_line_count);

	/* End block size by \r */
	g_assert_true (content[block_size - 1] != '\n');
	g_assert_true (content[block_size] == '\0');
	content[block_size - 1] = '\r';
	expected_line_count++;
	test_loader_split_cr_lf (content, expected_line_count);
	g_free (content);

	/* End of block 1: \r
	 * Start of block 2: a
	 */
	content = generate_content (block_size + 1, &expected_line_count);
	g_assert_true (content[block_size - 1] != '\n');
	g_assert_true (content[block_size] == 'a');
	g_assert_true (content[block_size + 1] == '\0');
	content[block_size - 1] = '\r';
	expected_line_count++;
	test_loader_split_cr_lf (content, expected_line_count);

	/* Split CR/LF:
	 * End of block 1: \r
	 * Start of block 2: \n
	 */
	g_assert_true (content[block_size - 1] == '\r');
	g_assert_true (content[block_size] == 'a');
	g_assert_true (content[block_size + 1] == '\0');
	content[block_size] = '\n';
	test_loader_split_cr_lf (content, expected_line_count);
	g_free (content);

	/* Split CR/LF:
	 * End of block 1: \r
	 * Start of block 2: \na
	 */
	content = generate_content (block_size + 2, &expected_line_count);
	g_assert_true (content[block_size - 1] != '\n');
	g_assert_true (content[block_size] != '\n');
	g_assert_true (content[block_size + 1] == 'a');
	g_assert_true (content[block_size + 2] == '\0');
	content[block_size - 1] = '\r';
	content[block_size] = '\n';
	expected_line_count++;
	test_loader_split_cr_lf (content, expected_line_count);
	g_free (content);

	/* Two complete blocks:
	 * End of block 1: \r
	 * Start of block 2: \naaaa...
	 * End of block 2: \r
	 */
	content = generate_content (block_size * 2, &expected_line_count);

	g_assert_true (content[block_size - 1] != '\n');
	g_assert_true (content[block_size] != '\n');
	content[block_size - 1] = '\r';
	content[block_size] = '\n';
	expected_line_count++;

	g_assert_true (content[block_size*2 - 1] != '\n');
	g_assert_true (content[block_size*2] == '\0');
	content[block_size*2 - 1] = '\r';
	expected_line_count++;

	test_loader_split_cr_lf (content, expected_line_count);
	g_free (content);
}
#endif

static void
test_loader_max_size (const gchar *contents,
		      const gchar *expected_buffer_content,
		      GQuark       expected_error_domain,
		      gint         expected_error_code,
		      gint64       max_size)
{
	test_loader (contents,
		     expected_buffer_content,
		     expected_error_domain,
		     expected_error_code,
		     "ASCII",
		     TEPL_NEWLINE_TYPE_LF,
		     -1,
		     FALSE,
		     max_size);
}

static void
test_max_size (void)
{
	gchar *content;

	content = generate_content (MAX_SIZE + 10, NULL);

	/* Unlimited */
	test_loader_max_size (content, content, 0, 0, -1);

	/* Too big */
	test_loader_max_size (content,
			      "",
			      TEPL_FILE_LOADER_ERROR,
			      TEPL_FILE_LOADER_ERROR_TOO_BIG,
			      MAX_SIZE);

	g_free (content);

	/* Exactly max size */
	content = generate_content (MAX_SIZE, NULL);
	test_loader_max_size (content, content, 0, 0, MAX_SIZE);
	g_free (content);
}

/* The idea of this unit test is not to test thoroughly all character encodings,
 * a better place for this is uchardet. The idea is just to test something else
 * than ASCII.
 */
static void
test_encoding (void)
{
	/* The expected encoding depends on uchardet, so this unit test might
	 * break with future versions of uchardet. The expected encoding is
	 * ISO-8859-something. E.g. ISO-8859-15 would be correct too.
	 *
	 * French sentence: Un éléphant ça trompe énormément.
	 */
	test_loader ("Un \351l\351phant \347a trompe \351norm\351ment.\n",
		     "Un \303\251l\303\251phant \303\247a trompe \303\251norm\303\251ment.\n",
		     0, 0,
		     "ISO-8859-1",
		     TEPL_NEWLINE_TYPE_LF,
		     2,
		     FALSE,
		     -1);

	/* German word in uppercase: STRAẞE */
	test_loader ("STRA\341\272\236E\n",
		     "STRA\341\272\236E\n",
		     0, 0,
		     "UTF-8",
		     TEPL_NEWLINE_TYPE_LF,
		     2,
		     FALSE,
		     -1);
}

#ifndef G_OS_WIN32
static GFile *
create_writable_file (void)
{
	gchar *path;
	GFile *location;
	GError *error = NULL;

	path = g_build_filename (g_get_tmp_dir (), "tepl-test-file-loader", NULL);
	location = g_file_new_for_path (path);

	g_file_delete (location, NULL, NULL);

	g_file_set_contents (path, "\n", -1, &error);
	g_assert_no_error (error);

	g_free (path);
	return location;
}

static void
check_permissions (GFile *location,
		   guint  permissions)
{
	GError *error = NULL;
	GFileInfo *info;

	info = g_file_query_info (location,
	                          G_FILE_ATTRIBUTE_UNIX_MODE,
	                          G_FILE_QUERY_INFO_NONE,
	                          NULL,
	                          &error);

	g_assert_no_error (error);

	g_assert_cmpint (permissions,
	                 ==,
	                 g_file_info_get_attribute_uint32 (info, G_FILE_ATTRIBUTE_UNIX_MODE) & ACCESSPERMS);

	g_object_unref (info);
}

static GFile *
create_file_with_permissions (guint permissions)
{
	gchar *path;
	GFile *location;
	GFileInfo *info;
	guint mode;
	GError *error = NULL;

	path = g_build_filename (g_get_tmp_dir (), "tepl-test-file-loader", NULL);
	location = g_file_new_for_path (path);

	g_file_delete (location, NULL, NULL);

	g_file_set_contents (path, "\n", -1, &error);
	g_assert_no_error (error);

	info = g_file_query_info (location,
				  G_FILE_ATTRIBUTE_UNIX_MODE,
				  G_FILE_QUERY_INFO_NONE,
				  NULL,
				  &error);
	g_assert_no_error (error);

	mode = g_file_info_get_attribute_uint32 (info, G_FILE_ATTRIBUTE_UNIX_MODE);
	g_object_unref (info);

	g_file_set_attribute_uint32 (location,
				     G_FILE_ATTRIBUTE_UNIX_MODE,
				     (mode & ~ACCESSPERMS) | permissions,
				     G_FILE_QUERY_INFO_NONE,
				     NULL,
				     &error);
	g_assert_no_error (error);

	check_permissions (location, permissions);

	g_free (path);
	return location;
}

static void
check_readonly_cb (GObject      *source_object,
		   GAsyncResult *result,
		   gpointer      user_data)
{
	TeplFileLoader *loader = TEPL_FILE_LOADER (source_object);
	gboolean expected_readonly = GPOINTER_TO_INT (user_data);
	TeplFile *file;
	gboolean readonly;
	GError *error = NULL;

	tepl_file_loader_load_finish (loader, result, &error);
	g_assert_no_error (error);

	file = tepl_file_loader_get_file (loader);
	readonly = tepl_file_is_readonly (file);
	g_assert_cmpint (readonly, ==, expected_readonly);

	gtk_main_quit ();
}

static void
check_readonly (GFile    *location,
		gboolean  expected_readonly)
{
	TeplBuffer *buffer;
	TeplFile *file;
	TeplFileLoader *loader;

	buffer = tepl_buffer_new ();
	file = tepl_buffer_get_file (buffer);

	tepl_file_set_location (file, location);
	loader = tepl_file_loader_new (buffer, file);

	tepl_file_loader_load_async (loader,
				     G_PRIORITY_DEFAULT,
				     NULL,
				     NULL, NULL, NULL,
				     check_readonly_cb,
				     GINT_TO_POINTER (expected_readonly));

	gtk_main ();

	g_object_unref (buffer);
	g_object_unref (loader);
}

static void
test_readonly (void)
{
	GFile *location;

	/* Writable */
	location = create_writable_file ();
	check_readonly (location, FALSE);
	g_object_unref (location);

	location = create_file_with_permissions (0600);
	check_readonly (location, FALSE);
	g_object_unref (location);

	/* Read only */
	location = create_file_with_permissions (0400);
	check_readonly (location, TRUE);
	g_object_unref (location);

	location = create_file_with_permissions (0440);
	check_readonly (location, TRUE);
	g_object_unref (location);

	location = create_file_with_permissions (0444);
	check_readonly (location, TRUE);
	g_object_unref (location);

	/* Read and execute */
	location = create_file_with_permissions (0500);
	check_readonly (location, TRUE);
	g_object_unref (location);
}
#endif /* !G_OS_WIN32 */

int
main (int    argc,
      char **argv)
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/file-loader/empty", test_empty);
	g_test_add_func ("/file-loader/newlines", test_newlines);
#if 0
	g_test_add_func ("/file-loader/split-cr-lf", test_split_cr_lf);
#endif
	g_test_add_func ("/file-loader/max-size", test_max_size);
	g_test_add_func ("/file-loader/encoding", test_encoding);

#ifndef G_OS_WIN32
	g_test_add_func ("/file-loader/readonly", test_readonly);
#endif

	return g_test_run ();
}
