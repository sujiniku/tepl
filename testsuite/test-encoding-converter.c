/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
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

#include "tepl/tepl-encoding-converter.h"
#include <gio/gio.h> /* For G_IO_ERROR */

static void
converter_cb (const gchar *str,
	      gsize        length,
	      gpointer     user_data)
{
	GQueue *received_output = user_data;

	g_queue_push_tail (received_output, g_strdup (str));
}

static void
compare_outputs (GQueue *received_output,
		 GQueue *expected_output)
{
	GList *received_node;
	GList *expected_node;

	for (received_node = received_output->head, expected_node = expected_output->head;
	     received_node != NULL && expected_node != NULL;
	     received_node = received_node->next, expected_node = expected_node->next)
	{
		const gchar *received_chunk = received_node->data;
		const gchar *expected_chunk = expected_node->data;

		g_assert_cmpstr (received_chunk, ==, expected_chunk);
	}

	g_assert (received_node == NULL);
	g_assert (expected_node == NULL);
}

static void
test_iso_8859_15_to_utf8 (void)
{
	TeplEncodingConverter *converter;
	GQueue *received_output;
	GQueue *expected_output;
	GError *error = NULL;

	converter = _tepl_encoding_converter_new (-1);

	received_output = g_queue_new ();
	_tepl_encoding_converter_set_callback (converter, converter_cb, received_output);

	_tepl_encoding_converter_open (converter, "UTF-8", "ISO-8859-15", &error);
	g_assert_no_error (error);

	_tepl_encoding_converter_feed (converter,
				       "Hello S\351bastien.",
				       -1,
				       &error);
	g_assert_no_error (error);

	_tepl_encoding_converter_close (converter, &error);
	g_assert_no_error (error);

	expected_output = g_queue_new ();
	g_queue_push_tail (expected_output, g_strdup ("Hello S\303\251bastien."));

	compare_outputs (received_output, expected_output);

	g_queue_free_full (received_output, g_free);
	g_queue_free_full (expected_output, g_free);
	g_object_unref (converter);
}

static void
test_utf8_to_utf8 (void)
{
	TeplEncodingConverter *converter;
	GQueue *received_output;
	GQueue *expected_output;
	GError *error = NULL;

	converter = _tepl_encoding_converter_new (-1);

	received_output = g_queue_new ();
	_tepl_encoding_converter_set_callback (converter, converter_cb, received_output);

	_tepl_encoding_converter_open (converter, "UTF-8", "UTF-8", &error);
	g_assert_no_error (error);

	_tepl_encoding_converter_feed (converter,
				       "Hello S\303\251bastien.",
				       -1,
				       &error);
	g_assert_no_error (error);

	_tepl_encoding_converter_close (converter, &error);
	g_assert_no_error (error);

	expected_output = g_queue_new ();
	g_queue_push_tail (expected_output, g_strdup ("Hello S\303\251bastien."));

	compare_outputs (received_output, expected_output);

	g_queue_free_full (received_output, g_free);
	g_queue_free_full (expected_output, g_free);
	g_object_unref (converter);
}

static void
test_buffer_full (void)
{
	TeplEncodingConverter *converter;
	GQueue *received_output;
	GQueue *expected_output;
	GError *error = NULL;

	/* 10 + terminated nul byte. */
	converter = _tepl_encoding_converter_new (11);

	received_output = g_queue_new ();
	_tepl_encoding_converter_set_callback (converter, converter_cb, received_output);

	_tepl_encoding_converter_open (converter, "UTF-8", "UTF-8", &error);
	g_assert_no_error (error);

	_tepl_encoding_converter_feed (converter,
				       "Hello S\303\251bastien.",
				       -1,
				       &error);
	g_assert_no_error (error);

	_tepl_encoding_converter_close (converter, &error);
	g_assert_no_error (error);

	expected_output = g_queue_new ();
	g_queue_push_tail (expected_output, g_strdup ("Hello S\303\251b"));
	g_queue_push_tail (expected_output, g_strdup ("astien."));

	compare_outputs (received_output, expected_output);

	g_queue_free_full (received_output, g_free);
	g_queue_free_full (expected_output, g_free);
	g_object_unref (converter);
}

static void
test_incomplete_input (void)
{
	TeplEncodingConverter *converter;
	GQueue *received_output;
	GQueue *expected_output;
	GError *error = NULL;

	converter = _tepl_encoding_converter_new (-1);

	/* Split a 2-byte character */
	received_output = g_queue_new ();
	_tepl_encoding_converter_set_callback (converter, converter_cb, received_output);

	_tepl_encoding_converter_open (converter, "UTF-8", "UTF-8", &error);
	g_assert_no_error (error);

	_tepl_encoding_converter_feed (converter, "Hello S\303", -1, &error);
	g_assert_no_error (error);
	_tepl_encoding_converter_feed (converter, "\251bastien.", -1, &error);
	g_assert_no_error (error);

	_tepl_encoding_converter_close (converter, &error);
	g_assert_no_error (error);

	expected_output = g_queue_new ();
	g_queue_push_tail (expected_output, g_strdup ("Hello S\303\251bastien."));

	compare_outputs (received_output, expected_output);

	g_queue_free_full (received_output, g_free);
	g_queue_free_full (expected_output, g_free);
	received_output = NULL;
	expected_output = NULL;

	/* Split the 3-byte character ẞ: [1, 2] */
	received_output = g_queue_new ();
	_tepl_encoding_converter_set_callback (converter, converter_cb, received_output);

	_tepl_encoding_converter_open (converter, "UTF-8", "UTF-8", &error);
	g_assert_no_error (error);

	_tepl_encoding_converter_feed (converter, "\341", -1, &error);
	g_assert_no_error (error);
	_tepl_encoding_converter_feed (converter, "\272\236", -1, &error);
	g_assert_no_error (error);

	_tepl_encoding_converter_close (converter, &error);
	g_assert_no_error (error);

	expected_output = g_queue_new ();
	g_queue_push_tail (expected_output, g_strdup ("\341\272\236"));

	compare_outputs (received_output, expected_output);

	g_queue_free_full (received_output, g_free);
	g_queue_free_full (expected_output, g_free);
	received_output = NULL;
	expected_output = NULL;

	/* Split the 3-byte character ẞ: [2, 1] */
	received_output = g_queue_new ();
	_tepl_encoding_converter_set_callback (converter, converter_cb, received_output);

	_tepl_encoding_converter_open (converter, "UTF-8", "UTF-8", &error);
	g_assert_no_error (error);

	_tepl_encoding_converter_feed (converter, "\341\272", -1, &error);
	g_assert_no_error (error);
	_tepl_encoding_converter_feed (converter, "\236", -1, &error);
	g_assert_no_error (error);

	_tepl_encoding_converter_close (converter, &error);
	g_assert_no_error (error);

	expected_output = g_queue_new ();
	g_queue_push_tail (expected_output, g_strdup ("\341\272\236"));

	compare_outputs (received_output, expected_output);

	g_queue_free_full (received_output, g_free);
	g_queue_free_full (expected_output, g_free);
	received_output = NULL;
	expected_output = NULL;

	/* Split the 3-byte character ẞ: [1, 1, 1] */
	received_output = g_queue_new ();
	_tepl_encoding_converter_set_callback (converter, converter_cb, received_output);

	_tepl_encoding_converter_open (converter, "UTF-8", "UTF-8", &error);
	g_assert_no_error (error);

	_tepl_encoding_converter_feed (converter, "\341", -1, &error);
	g_assert_no_error (error);
	_tepl_encoding_converter_feed (converter, "\272", -1, &error);
	g_assert_no_error (error);
	_tepl_encoding_converter_feed (converter, "\236", -1, &error);
	g_assert_no_error (error);

	_tepl_encoding_converter_close (converter, &error);
	g_assert_no_error (error);

	expected_output = g_queue_new ();
	g_queue_push_tail (expected_output, g_strdup ("\341\272\236"));

	compare_outputs (received_output, expected_output);

	g_queue_free_full (received_output, g_free);
	g_queue_free_full (expected_output, g_free);
	received_output = NULL;
	expected_output = NULL;

	g_object_unref (converter);
}

static void
test_invalid_sequence (void)
{
	TeplEncodingConverter *converter;
	GError *error = NULL;

	converter = _tepl_encoding_converter_new (-1);

	_tepl_encoding_converter_open (converter, "UTF-8", "UTF-8", &error);
	g_assert_no_error (error);

	/* \251 is the second byte of "é". */
	_tepl_encoding_converter_feed (converter,
				       "Hello S\251bastien.",
				       -1,
				       &error);

	g_assert (g_error_matches (error, G_CONVERT_ERROR, G_CONVERT_ERROR_ILLEGAL_SEQUENCE));
	g_clear_error (&error);

	_tepl_encoding_converter_close (converter, &error);
	g_assert_no_error (error);

	g_object_unref (converter);
}

static void
test_end_with_incomplete_input (void)
{
	TeplEncodingConverter *converter;
	GError *error = NULL;

	converter = _tepl_encoding_converter_new (-1);

	_tepl_encoding_converter_open (converter, "UTF-8", "UTF-8", &error);
	g_assert_no_error (error);

	/* \303 is the start of a two-byte character. */
	_tepl_encoding_converter_feed (converter, "So far so \303", -1, &error);
	g_assert_no_error (error);

	_tepl_encoding_converter_close (converter, &error);
	g_assert (g_error_matches (error, G_CONVERT_ERROR, G_CONVERT_ERROR_PARTIAL_INPUT));
	g_clear_error (&error);

	g_object_unref (converter);
}

gint
main (gint   argc,
      gchar *argv[])
{
	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/encoding-converter/ISO-8859-15-to-UTF-8", test_iso_8859_15_to_utf8);
	g_test_add_func ("/encoding-converter/UTF-8-to-UTF-8", test_utf8_to_utf8);
	g_test_add_func ("/encoding-converter/buffer-full", test_buffer_full);
	g_test_add_func ("/encoding-converter/incomplete-input", test_incomplete_input);
	g_test_add_func ("/encoding-converter/invalid-sequence", test_invalid_sequence);
	g_test_add_func ("/encoding-converter/end-with-incomplete-input", test_end_with_incomplete_input);

	return g_test_run ();
}
