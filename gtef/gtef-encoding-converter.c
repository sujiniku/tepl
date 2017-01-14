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

#include "config.h"
#include "gtef-encoding-converter.h"
#include <errno.h>
#include <string.h>
#include <gio/gio.h>
#include <glib/gi18n-lib.h>

/* A higher-level, more convenient API for character encoding streaming
 * conversion based on iconv.
 *
 * A #GtefEncodingConverter object can be opened/closed several times, for
 * different conversions.
 *
 * Even if from_codeset and to_codeset are the same, this class can be useful
 * for several reasons:
 * - the output string doesn't end in-between a multi-byte character, while a
 *   passed-in chunk can.
 * - the output string is nul-terminated.
 * - the buffer size of this object can be adjusted, to control how often the
 *   callback is called.
 */

struct _GtefEncodingConverterPrivate
{
	GIConv conv;

	/* - outbuf_size is the full size of outbuf (if outbuf is non-NULL),
	 *   *including* the additional byte to nul-terminate the string.
	 * - The following condition must be met:
	 *   0 <= outbytes_left < outbuf_size
	 *   In other words, outbytes_left *doesn't include* the byte to
	 *   nul-terminate the string.
	 */
	gchar *outbuf;
	gint64 outbuf_size;
	gsize outbytes_left;

	GtefEncodingConversionCallback callback;
	gpointer callback_user_data;

	/* On incomplete input, store the remaining inbuf so that it can be used
	 * for the next chunk.
	 */
	GString *remaining_inbuf;
};

enum
{
	PROP_0,
	PROP_BUFFER_SIZE,
	N_PROPERTIES
};

typedef enum _Result
{
	RESULT_OK,
	RESULT_INCOMPLETE_INPUT,
	RESULT_ERROR
} Result;

/* 1 MB */
#define DEFAULT_OUTBUF_SIZE (1024 * 1024)

/* One byte of data, one byte to nul-terminate the string. */
#define MIN_OUTBUF_SIZE 2

static GParamSpec *properties[N_PROPERTIES];

G_DEFINE_TYPE_WITH_PRIVATE (GtefEncodingConverter, _gtef_encoding_converter, G_TYPE_OBJECT)

static void
check_invariants (GtefEncodingConverter *converter)
{
	g_assert_cmpint (converter->priv->outbuf_size, >=, MIN_OUTBUF_SIZE);
	g_assert_cmpint (converter->priv->outbytes_left, <, converter->priv->outbuf_size);
}

static gboolean
is_opened (GtefEncodingConverter *converter)
{
	return converter->priv->conv != (GIConv)-1;
}

static gboolean
outbuf_is_empty (GtefEncodingConverter *converter)
{
	check_invariants (converter);

	return (converter->priv->outbuf == NULL ||
		(gint64)converter->priv->outbytes_left == (converter->priv->outbuf_size - 1));
}

static gsize
get_outbuf_used_length (GtefEncodingConverter *converter)
{
	check_invariants (converter);

	return (converter->priv->outbuf_size - 1) - converter->priv->outbytes_left;
}

static void
flush_outbuf (GtefEncodingConverter *converter)
{
	if (outbuf_is_empty (converter))
	{
		return;
	}

	if (converter->priv->callback != NULL)
	{
		gsize length;

		length = get_outbuf_used_length (converter);
		converter->priv->outbuf[length] = '\0';

		converter->priv->callback (converter->priv->outbuf,
					   length,
					   converter->priv->callback_user_data);
	}

	converter->priv->outbytes_left = (converter->priv->outbuf_size - 1);
}

static void
close_conv (GtefEncodingConverter *converter)
{
	if (converter->priv->conv != (GIConv)-1)
	{
		g_iconv_close (converter->priv->conv);
		converter->priv->conv = (GIConv)-1;
	}

	if (converter->priv->remaining_inbuf != NULL)
	{
		g_string_free (converter->priv->remaining_inbuf, TRUE);
		converter->priv->remaining_inbuf = NULL;
	}
}

static void
_gtef_encoding_converter_get_property (GObject    *object,
				       guint       prop_id,
				       GValue     *value,
				       GParamSpec *pspec)
{
	GtefEncodingConverter *converter = GTEF_ENCODING_CONVERTER (object);

	switch (prop_id)
	{
		case PROP_BUFFER_SIZE:
			g_value_set_int64 (value, _gtef_encoding_converter_get_buffer_size (converter));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
_gtef_encoding_converter_set_property (GObject      *object,
				       guint         prop_id,
				       const GValue *value,
				       GParamSpec   *pspec)
{
	GtefEncodingConverter *converter = GTEF_ENCODING_CONVERTER (object);

	switch (prop_id)
	{
		case PROP_BUFFER_SIZE:
			converter->priv->outbuf_size = g_value_get_int64 (value);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
_gtef_encoding_converter_finalize (GObject *object)
{
	GtefEncodingConverter *converter = GTEF_ENCODING_CONVERTER (object);

	close_conv (converter);
	g_free (converter->priv->outbuf);

	G_OBJECT_CLASS (_gtef_encoding_converter_parent_class)->finalize (object);
}

static void
_gtef_encoding_converter_class_init (GtefEncodingConverterClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = _gtef_encoding_converter_get_property;
	object_class->set_property = _gtef_encoding_converter_set_property;
	object_class->finalize = _gtef_encoding_converter_finalize;

	/**
	 * GtefEncodingConverter:buffer-size:
	 *
	 * The buffer size, in bytes. When the buffer is full, the callback is
	 * called to empty the buffer.
	 */
	properties[PROP_BUFFER_SIZE] =
		g_param_spec_int64 ("buffer-size",
				    "Buffer Size",
				    "",
				    MIN_OUTBUF_SIZE,
				    G_MAXINT64,
				    DEFAULT_OUTBUF_SIZE,
				    G_PARAM_READWRITE |
				    G_PARAM_CONSTRUCT_ONLY |
				    G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
_gtef_encoding_converter_init (GtefEncodingConverter *converter)
{
	converter->priv = _gtef_encoding_converter_get_instance_private (converter);

	converter->priv->conv = (GIConv)-1;
}

GtefEncodingConverter *
_gtef_encoding_converter_new (gint64 buffer_size)
{
	g_return_val_if_fail (buffer_size == -1 || buffer_size >= MIN_OUTBUF_SIZE, NULL);

	if (buffer_size == -1)
	{
		buffer_size = DEFAULT_OUTBUF_SIZE;
	}

	return g_object_new (GTEF_TYPE_ENCODING_CONVERTER,
			     "buffer-size", buffer_size,
			     NULL);
}

gint64
_gtef_encoding_converter_get_buffer_size (GtefEncodingConverter *converter)
{
	g_return_val_if_fail (GTEF_IS_ENCODING_CONVERTER (converter), 0);

	return converter->priv->outbuf_size;
}

void
_gtef_encoding_converter_set_callback (GtefEncodingConverter          *converter,
				       GtefEncodingConversionCallback  callback,
				       gpointer                        user_data)
{
	g_return_if_fail (GTEF_IS_ENCODING_CONVERTER (converter));

	converter->priv->callback = callback;
	converter->priv->callback_user_data = user_data;
}

gboolean
_gtef_encoding_converter_open (GtefEncodingConverter  *converter,
			       const gchar            *to_codeset,
			       const gchar            *from_codeset,
			       GError                **error)
{
	g_return_val_if_fail (GTEF_IS_ENCODING_CONVERTER (converter), FALSE);
	g_return_val_if_fail (to_codeset != NULL, FALSE);
	g_return_val_if_fail (from_codeset != NULL, FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail (!is_opened (converter), FALSE);

	converter->priv->conv = g_iconv_open (to_codeset, from_codeset);

	if (converter->priv->conv == (GIConv)-1)
	{
		if (errno == EINVAL)
		{
			g_set_error (error,
				     G_CONVERT_ERROR,
				     G_CONVERT_ERROR_NO_CONVERSION,
				     _("Conversion from character set '%s' to '%s' is not supported."),
				     from_codeset,
				     to_codeset);
		}
		else
		{
			g_set_error (error,
				     G_IO_ERROR,
				     G_IO_ERROR_FAILED,
				     _("Could not open converter from '%s' to '%s': %s."),
				     from_codeset,
				     to_codeset,
				     g_strerror (errno));
		}

		return FALSE;
	}

	if (converter->priv->outbuf == NULL)
	{
		converter->priv->outbuf = g_malloc (converter->priv->outbuf_size);
	}

	converter->priv->outbytes_left = (converter->priv->outbuf_size - 1);

	return TRUE;
}

static Result
read_inbuf (GtefEncodingConverter  *converter,
	    gchar                 **inbuf,
	    gsize                  *inbytes_left,
	    GError                **error)
{
	g_assert (inbytes_left != NULL);

	while (*inbytes_left > 0 || inbuf == NULL)
	{
		gchar *outbuf;
		gsize iconv_ret;

		outbuf = converter->priv->outbuf + get_outbuf_used_length (converter);

		iconv_ret = g_iconv (converter->priv->conv,
				     inbuf,
				     inbytes_left,
				     &outbuf,
				     &converter->priv->outbytes_left);

		if (iconv_ret == (gsize)-1)
		{
			/* outbuf full */
			if (errno == E2BIG)
			{
				flush_outbuf (converter);
				continue;
			}
			else if (errno == EINVAL)
			{
				return RESULT_INCOMPLETE_INPUT;
			}
			else if (errno == EILSEQ)
			{
				g_set_error (error,
					     G_CONVERT_ERROR,
					     G_CONVERT_ERROR_ILLEGAL_SEQUENCE,
					     "The input data contains an invalid sequence.");

				return RESULT_ERROR;
			}
			else
			{
				g_set_error (error,
					     G_IO_ERROR,
					     G_IO_ERROR_FAILED,
					     "Error when converting data: %s.",
					     g_strerror (errno));

				return RESULT_ERROR;
			}
		}
		else if (inbuf == NULL)
		{
			return RESULT_OK;
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
handle_remaining_inbuf (GtefEncodingConverter  *converter,
			gchar                 **inbuf,
			gsize                  *inbytes_left,
			GError                **error)
{
	if (converter->priv->remaining_inbuf == NULL)
	{
		return RESULT_OK;
	}

	if (converter->priv->remaining_inbuf->len == 0)
	{
		g_string_free (converter->priv->remaining_inbuf, TRUE);
		converter->priv->remaining_inbuf = NULL;
		return RESULT_OK;
	}

	while (*inbytes_left > 0)
	{
		gchar *my_inbuf;
		gsize my_inbytes_left;
		gboolean my_result;

		g_string_append_len (converter->priv->remaining_inbuf, *inbuf, 1);
		(*inbuf)++;
		(*inbytes_left)--;

		my_inbuf = converter->priv->remaining_inbuf->str;
		my_inbytes_left = converter->priv->remaining_inbuf->len;

		my_result = read_inbuf (converter,
					&my_inbuf,
					&my_inbytes_left,
					error);

		switch (my_result)
		{
			case RESULT_OK:
				g_string_free (converter->priv->remaining_inbuf, TRUE);
				converter->priv->remaining_inbuf = NULL;
				return RESULT_OK;

			case RESULT_INCOMPLETE_INPUT:
				/* I think my_inbytes_left cannot have been
				 * changed here, but it's anyway safer to handle
				 * the case.
				 */
				g_string_erase (converter->priv->remaining_inbuf,
						0,
						converter->priv->remaining_inbuf->len - my_inbytes_left);
				break;

			case RESULT_ERROR:
				return RESULT_ERROR;

			default:
				g_assert_not_reached ();
		}
	}

	return RESULT_INCOMPLETE_INPUT;
}

/*
 * The callback is called when the internal buffer is filled, it doesn't
 * necessarily happen each time _gtef_encoding_converter_feed() is called, and
 * the callback can be called several times during a single feed.
 *
 * Returns: %TRUE on success, %FALSE on error.
 */
gboolean
_gtef_encoding_converter_feed (GtefEncodingConverter  *converter,
			       const gchar            *chunk,
			       gssize                  size,
			       GError                **error)
{
	gchar *inbuf;
	gsize inbytes_left;
	Result result;

	g_return_val_if_fail (GTEF_IS_ENCODING_CONVERTER (converter), FALSE);
	g_return_val_if_fail (size >= -1, FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail (is_opened (converter), FALSE);

	if (chunk == NULL || size == 0)
	{
		return TRUE;
	}

	inbuf = (gchar *)chunk;
	inbytes_left = size == -1 ? strlen (chunk) : (gsize)size;

	result = handle_remaining_inbuf (converter,
					 &inbuf,
					 &inbytes_left,
					 error);

	switch (result)
	{
		case RESULT_OK:
			break;

		case RESULT_INCOMPLETE_INPUT:
			return TRUE;

		case RESULT_ERROR:
			return FALSE;

		default:
			g_assert_not_reached ();
	}

	g_assert (converter->priv->remaining_inbuf == NULL);

	result = read_inbuf (converter,
			     &inbuf,
			     &inbytes_left,
			     error);

	switch (result)
	{
		case RESULT_OK:
			break;

		case RESULT_INCOMPLETE_INPUT:
			converter->priv->remaining_inbuf = g_string_new_len (inbuf, inbytes_left);
			break;

		case RESULT_ERROR:
			return FALSE;

		default:
			g_assert_not_reached ();
	}

	return TRUE;
}

/* This function can trigger the callback a last time. There can be an error if
 * the last chunk ended with an incomplete multi-byte char.
 */
gboolean
_gtef_encoding_converter_close (GtefEncodingConverter  *converter,
				GError                **error)
{
	gboolean ok = TRUE;

	g_return_val_if_fail (GTEF_IS_ENCODING_CONVERTER (converter), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail (is_opened (converter), FALSE);

	if (converter->priv->remaining_inbuf != NULL &&
	    converter->priv->remaining_inbuf->len > 0)
	{
		g_set_error_literal (error,
				     G_CONVERT_ERROR,
				     G_CONVERT_ERROR_PARTIAL_INPUT,
				     _("The content ends with an incomplete multi-byte sequence."));
		ok = FALSE;
	}
	else
	{
		gchar **inbuf = NULL;
		gsize inbytes_left = 0;
		Result result;

		result = read_inbuf (converter,
				     inbuf,
				     &inbytes_left,
				     error);

		switch (result)
		{
			case RESULT_OK:
				break;

			case RESULT_INCOMPLETE_INPUT:
				g_set_error_literal (error,
						     G_CONVERT_ERROR,
						     G_CONVERT_ERROR_PARTIAL_INPUT,
						     _("The content ends with incomplete data."));
				ok = FALSE;
				break;

			case RESULT_ERROR:
				ok = FALSE;
				break;

			default:
				g_assert_not_reached ();
		}
	}

	flush_outbuf (converter);

	/* We must call this even on error, because the converter can be
	 * opened/closed several times.
	 */
	close_conv (converter);

	return ok;
}
