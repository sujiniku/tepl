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
 * - this object can have a bigger buffer, so the callback can be called less
 *   often (which might or might not be a good thing).
 */

struct _GtefEncodingConverterPrivate
{
	GIConv conv;

	gchar *outbuf;
	gsize outbytes_left;

	GtefEncodingConversionCallback callback;
	gpointer callback_user_data;

	/* On incomplete input, store the remaining inbuf so that it can be used
	 * for the next chunk.
	 */
	GString *remaining_inbuf;
};

typedef enum _Result
{
	RESULT_OK,
	RESULT_INCOMPLETE_INPUT,
	RESULT_ERROR
} Result;

/* 1 MB */
#define OUTBUF_SIZE (1024 * 1024 - 1)

G_DEFINE_TYPE_WITH_PRIVATE (GtefEncodingConverter, _gtef_encoding_converter, G_TYPE_OBJECT)

static gboolean
is_opened (GtefEncodingConverter *converter)
{
	return converter->priv->conv != (GIConv)-1;
}

static void
flush_outbuf (GtefEncodingConverter *converter)
{
	g_assert_cmpint (converter->priv->outbytes_left, <=, OUTBUF_SIZE);

	if (converter->priv->outbuf == NULL ||
	    converter->priv->outbytes_left == OUTBUF_SIZE)
	{
		return;
	}

	if (converter->priv->callback != NULL)
	{
		gsize length;

		length = OUTBUF_SIZE - converter->priv->outbytes_left;
		converter->priv->outbuf[length] = '\0';

		converter->priv->callback (converter->priv->outbuf,
					   length,
					   converter->priv->callback_user_data);
	}

	converter->priv->outbytes_left = OUTBUF_SIZE;
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

	object_class->finalize = _gtef_encoding_converter_finalize;
}

static void
_gtef_encoding_converter_init (GtefEncodingConverter *converter)
{
	converter->priv = _gtef_encoding_converter_get_instance_private (converter);

	converter->priv->conv = (GIConv)-1;
}

GtefEncodingConverter *
_gtef_encoding_converter_new (void)
{
	return g_object_new (GTEF_TYPE_ENCODING_CONVERTER, NULL);
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
				     G_IO_ERROR,
				     G_IO_ERROR_NOT_SUPPORTED,
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
				     strerror (errno));
		}

		return FALSE;
	}

	if (converter->priv->outbuf == NULL)
	{
		/* +1, so we are able to nul-terminate the string. */
		converter->priv->outbuf = g_malloc (OUTBUF_SIZE + 1);
	}

	converter->priv->outbytes_left = OUTBUF_SIZE;

	return TRUE;
}

static Result
read_inbuf (GtefEncodingConverter  *converter,
	    gchar                 **inbuf,
	    gsize                  *inbytes_left,
	    GError                **error)
{
	while (*inbytes_left > 0)
	{
		gchar *outbuf;
		gsize iconv_ret;

		g_assert_cmpint (converter->priv->outbytes_left, <=, OUTBUF_SIZE);
		outbuf = converter->priv->outbuf + OUTBUF_SIZE - converter->priv->outbytes_left;

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
					     G_IO_ERROR,
					     G_IO_ERROR_INVALID_DATA,
					     "The input data contains an invalid sequence.");

				return RESULT_ERROR;
			}
			else
			{
				g_set_error (error,
					     G_IO_ERROR,
					     G_IO_ERROR_FAILED,
					     "Error when converting data: %s.",
					     strerror (errno));

				return RESULT_ERROR;
			}
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

/* This function can trigger the callback a last time. */
void
_gtef_encoding_converter_close (GtefEncodingConverter *converter)
{
	g_return_if_fail (GTEF_IS_ENCODING_CONVERTER (converter));
	g_return_if_fail (is_opened (converter));

	flush_outbuf (converter);
	close_conv (converter);
}
