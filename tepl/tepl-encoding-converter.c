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

#include "config.h"
#include "tepl-encoding-converter.h"
#include <glib/gi18n-lib.h>
#include "tepl-encoding.h"
#include "tepl-iconv.h"

typedef struct _Buffer Buffer;
struct _Buffer
{
	/* Never NULL during normal use. */
	gchar *data;

	/* The total number of bytes allocated to @data. */
	gsize total_size;

	/* The number of bytes not yet written into @data. Those bytes are
	 * always at the end of @data.
	 */
	gsize n_remaining_bytes;
};

typedef struct _TaskData TaskData;
struct _TaskData
{
	/* To avoid creating huge areas of contiguous memory, the output is
	 * split in chunks.
	 */
	gsize max_output_chunk_size;

	TeplIconv *converter;

	Buffer *output_buffer;

	/* On incomplete input, store the remaining inbuf so that it can be used
	 * with the next input chunk.
	 */
	GByteArray *remaining_inbuf;

	/* To collect consecutive invalid chars before creating a new output
	 * chunk.
	 */
	Buffer *invalid_chars;

	/* (element-type TeplEncodingConverterOutputChunk) */
	GQueue *output_chunks;
};

typedef enum
{
	RESULT_OK,
	RESULT_ERROR,
	RESULT_INCOMPLETE_INPUT
} Result;

/* 1 MiB */
#define MAX_OUTPUT_CHUNK_SIZE_DEFAULT_VALUE (1024 * 1024)

/* 32 bytes are most probably enough for any character set with multi-byte
 * characters.
 */
#define MAX_OUTPUT_CHUNK_SIZE_MIN_VALUE (32)

/*****************************************************************************/
/* Buffer mini-class. */

static Buffer *
buffer_new (gsize total_size)
{
	Buffer *buffer;

	buffer = g_new0 (Buffer, 1);
	buffer->data = malloc (total_size);
	buffer->total_size = total_size;
	buffer->n_remaining_bytes = total_size;
}

static void
buffer_free (Buffer *buffer)
{
	g_free (buffer->data);
	g_free (buffer);
}

static void
buffer_check_bounds (Buffer *buffer)
{
	g_assert (buffer->n_remaining_bytes <= buffer->total_size);
}

static gsize
buffer_get_written_length (Buffer *buffer)
{
	buffer_check_bounds (buffer);
	return buffer->total_size - buffer->n_remaining_bytes;
}

static gboolean
buffer_is_empty (Buffer *buffer)
{
	return buffer_get_written_length (buffer) == 0;
}

static gboolean
buffer_is_near_to_full (Buffer *buffer)
{
	/* Takes into account multi-byte chars (not yet written). */
	return buffer->n_remaining_bytes < MAX_OUTPUT_CHUNK_SIZE_MIN_VALUE;
}

static GBytes *
buffer_flush (Buffer *buffer)
{
	GBytes *bytes;

	if (buffer_is_empty (buffer))
	{
		return NULL;
	}

	if (buffer_is_near_to_full (buffer))
	{
		/* Avoid a copy. */
		bytes = g_bytes_new_take (buffer->data, buffer_get_written_length (buffer));

		buffer->data = g_malloc (buffer->total_size);
		buffer->n_remaining_bytes = buffer->total_size;
	}
	else
	{
		bytes = g_bytes_new (buffer->data, buffer_get_written_length (buffer));
		buffer->n_remaining_bytes = buffer->total_size;
	}

	return bytes;
}

/*****************************************************************************/

static gboolean
input_chunk_is_valid (GBytes *input_chunk)
{
	return (input_chunk != NULL &&
		g_bytes_get_size (input_chunk) > 0);
}

static gboolean
input_chunks_list_is_valid (GList *input_chunks)
{
	GList *l;

	for (l = input_chunks; l != NULL; l = l->next)
	{
		GBytes *cur_input_chunk = l->data;

		if (!input_chunk_is_valid (cur_input_chunk))
		{
			return FALSE;
		}
	}

	return TRUE;
}

static void
append_output_chunk (GQueue   *output_chunks,
		     GBytes   *bytes,
		     gboolean  is_valid)
{
	TeplEncodingConverterOutputChunk *new_output_chunk;

	new_output_chunk = g_new0 (TeplEncodingConverterOutputChunk, 1);
	new_output_chunk->bytes = bytes;
	new_output_chunk->is_valid = is_valid;

	g_queue_push_tail (output_chunks, new_output_chunk);
}

/*****************************************************************************/

static TaskData *
task_data_new (gsize max_output_chunk_size)
{
	TaskData *task_data;

	task_data = g_new0 (TaskData, 1);
	task_data->max_output_chunk_size = max_output_chunk_size;

	task_data->output_chunks = g_queue_new ();

	return task_data;
}

static void
data_finalize (Data *data)
{
	/* close_converter() must be called first. */
	g_assert (data->converter == NULL);

	g_free (data->outbuf);

	if (data->remaining_inbuf != NULL)
	{
		g_byte_array_unref (data->remaining_inbuf);
	}
	if (data->invalid_chars != NULL)
	{
		g_byte_array_unref (data->invalid_chars);
	}

	g_queue_free_full (data->output_chunks,
			   (GDestroyNotify) _tepl_encoding_converter_output_chunk_free);
}

static gboolean
open_converter (Data          *data,
		TeplEncoding  *from_encoding,
		TeplEncoding  *to_encoding,
		GError       **error)
{
	g_assert (data->converter == NULL);
	data->converter = _tepl_iconv_new ();

	return _tepl_iconv_open (data->converter,
				 tepl_encoding_get_charset (to_encoding),
				 tepl_encoding_get_charset (from_encoding),
				 error);
}

static Result
read_inbuf (Data    *data,
	    gchar  **inbuf,
	    gsize   *inbytes_left,
	    GError **error)
{
	g_assert ((inbuf != NULL && *inbuf != NULL && inbytes_left != NULL) ||
		  (inbuf == NULL && inbytes_left == NULL));

	while (inbuf == NULL || *inbytes_left > 0)
	{
		gchar *outbuf;
		TeplIconvResult iconv_result;

		if (data->outbuf == NULL)
		{
			allocate_new_outbuf (data);
		}

		outbuf = data->outbuf + get_outbuf_used_length (data);

		iconv_result = _tepl_iconv_feed (data->converter,
						 inbuf,
						 inbytes_left,
						 &outbuf,
						 &data->outbytes_left,
						 error);

		switch (iconv_result)
		{
			case TEPL_ICONV_RESULT_OK:
				return RESULT_OK;

			case TEPL_ICONV_RESULT_ERROR:
				return RESULT_ERROR;

			case TEPL_ICONV_RESULT_INCOMPLETE_INPUT:
				return RESULT_INCOMPLETE_INPUT;

			case TEPL_ICONV_RESULT_INVALID_INPUT_CHAR:
				g_return_val_if_fail (inbuf != NULL, RESULT_ERROR);
				g_return_val_if_fail (*inbytes_left > 0, RESULT_ERROR);

				append_invalid_chars (data, (guint8 *) *inbuf, 1);
				(*inbuf)++;
				(*inbytes_left)--;
				break;

			case TEPL_ICONV_RESULT_OUTPUT_BUFFER_FULL:
				flush_outbuf (data);
				break;

			default:
				g_assert_not_reached ();
		}
	}

	return RESULT_OK;
}

/* One possible implementation would be to concatenate remaining_inbuf with the
 * new inbuf, but it would need a complete re-allocation.
 * Instead, only one char of inbuf is appended at a time to remaining_inbuf,
 * until it succeeds. That way, it's just tiny allocations.
 */
static Result
handle_remaining_inbuf (Data    *data,
			gchar  **inbuf,
			gsize   *inbytes_left,
			GError **error)
{
	if (data->remaining_inbuf == NULL)
	{
		return RESULT_OK;
	}

	if (data->remaining_inbuf->len == 0)
	{
		g_byte_array_unref (data->remaining_inbuf);
		data->remaining_inbuf = NULL;
		return RESULT_OK;
	}

	while (*inbytes_left > 0)
	{
		gchar *my_inbuf;
		gsize my_inbytes_left;
		Result my_result;

		g_byte_array_append (data->remaining_inbuf, (guint8 *) *inbuf, 1);
		(*inbuf)++;
		(*inbytes_left)--;

		my_inbuf = (gchar *) data->remaining_inbuf->data;
		my_inbytes_left = data->remaining_inbuf->len;

		my_result = read_inbuf (data,
					&my_inbuf,
					&my_inbytes_left,
					error);

		switch (my_result)
		{
			case RESULT_OK:
				g_byte_array_unref (data->remaining_inbuf);
				data->remaining_inbuf = NULL;
				return RESULT_OK;

			case RESULT_ERROR:
				return RESULT_ERROR;

			case RESULT_INCOMPLETE_INPUT:
				/* I think my_inbytes_left cannot have been
				 * changed here, but it's anyway safer to handle
				 * the case.
				 */
				g_byte_array_remove_range (data->remaining_inbuf,
							   0,
							   data->remaining_inbuf->len - my_inbytes_left);
				break;

			default:
				g_assert_not_reached ();
		}
	}

	return RESULT_INCOMPLETE_INPUT;
}

static gboolean
feed_input_chunk (Data    *data,
		  GBytes  *input_bytes,
		  GError **error)
{
	gchar *inbuf;
	gsize inbytes_left;
	Result result;

	inbuf = (gchar *) g_bytes_get_data (input_bytes, &inbytes_left);

	result = handle_remaining_inbuf (data,
					 &inbuf,
					 &inbytes_left,
					 error);

	switch (result)
	{
		case RESULT_OK:
			break;

		case RESULT_ERROR:
			return FALSE;

		case RESULT_INCOMPLETE_INPUT:
			return TRUE;

		default:
			g_assert_not_reached ();
	}

	g_assert (data->remaining_inbuf == NULL);

	result = read_inbuf (data,
			     &inbuf,
			     &inbytes_left,
			     error);

	switch (result)
	{
		case RESULT_OK:
			break;

		case RESULT_ERROR:
			return FALSE;

		case RESULT_INCOMPLETE_INPUT:
			data->remaining_inbuf = g_byte_array_new ();
			g_byte_array_append (data->remaining_inbuf, (guint8 *)inbuf, inbytes_left);
			break;

		default:
			g_assert_not_reached ();
	}

	return TRUE;
}

static gboolean
close_converter (Data    *data,
		 GError **error)
{
	Result result;
	gboolean ok = TRUE;

	if (data->remaining_inbuf != NULL &&
	    data->remaining_inbuf->len > 0)
	{
		append_invalid_chars (data,
				      data->remaining_inbuf->data,
				      data->remaining_inbuf->len);

		g_byte_array_unref (data->remaining_inbuf);
		data->remaining_inbuf = NULL;
	}

	result = read_inbuf (data, NULL, NULL, error);

	switch (result)
	{
		case RESULT_OK:
			break;

		case RESULT_ERROR:
			ok = FALSE;
			break;

		case RESULT_INCOMPLETE_INPUT:
			g_set_error_literal (error,
					     G_CONVERT_ERROR,
					     G_CONVERT_ERROR_PARTIAL_INPUT,
					     _("The input content ends with incomplete data."));
			ok = FALSE;
			break;

		default:
			g_assert_not_reached ();
	}

	flush_outbuf (data);

	if (error != NULL && *error != NULL)
	{
		ok = FALSE;

		/* Keep only the first error. */
		_tepl_iconv_close_and_free (data->converter, NULL);
	}
	else if (!_tepl_iconv_close_and_free (data->converter, error))
	{
		ok = FALSE;
	}

	data->converter = NULL;

	return ok;
}

gboolean
_tepl_encoding_converter_convert (GList         *input_chunks,
				  TeplEncoding  *from_encoding,
				  TeplEncoding  *to_encoding,
				  gint64         max_output_chunk_size,
				  GList        **output_chunks,
				  GError       **error)
{
	TaskData *task_data;

	g_return_val_if_fail ((max_output_chunk_size == -1 ||
			       max_output_chunk_size >= MAX_OUTPUT_CHUNK_SIZE_MIN_VALUE), FALSE);

	if (max_output_chunk_size == -1)
	{
		max_output_chunk_size = MAX_OUTPUT_CHUNK_SIZE_DEFAULT_VALUE;
	}

	task_data = task_data_new (max_output_chunk_size);

	return TRUE;
}

gboolean
_tepl_encoding_converter_test_conversion (GList        *input_chunks,
					  TeplEncoding *from_encoding,
					  TeplEncoding *to_encoding,
					  gint         *n_invalid_input_chars)
{
	return TRUE;
}

void
_tepl_encoding_converter_output_chunk_free (TeplEncodingConverterOutputChunk *output_chunk)
{
	if (output_chunk != NULL)
	{
		g_bytes_unref (output_chunk->bytes);
		g_free (output_chunk);
	}
}
