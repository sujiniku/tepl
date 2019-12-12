/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2019 - Sébastien Wilmet <swilmet@gnome.org>
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
#include "tepl-iconv.h"
#include <errno.h>
#include <glib/gi18n-lib.h>

/* TeplIconv:
 *
 * Small wrapper for `g_iconv*()` functions: return GErrors and enum values for
 * the different situations.
 *
 * Call the functions in this order:
 * - _tepl_iconv_new();
 * - _tepl_iconv_open();
 * - _tepl_iconv_feed() in a loop;
 * - _tepl_iconv_feed() with @inbuf and @inbytes_left set to %NULL (in a loop
 *   too if the output buffer is full).
 * - _tepl_iconv_close_and_free().
 */

struct _TeplIconv
{
	GIConv conv_descriptor;
};

static gboolean
is_opened (TeplIconv *conv)
{
	return conv->conv_descriptor != (GIConv)-1;
}

TeplIconv *
_tepl_iconv_new (void)
{
	TeplIconv *conv;

	conv = g_new0 (TeplIconv, 1);
	conv->conv_descriptor = (GIConv)-1;

	return conv;
}

/* Returns: %TRUE on success, %FALSE otherwise. */
gboolean
_tepl_iconv_open (TeplIconv    *conv,
		  const gchar  *to_codeset,
		  const gchar  *from_codeset,
		  GError      **error)
{
	g_return_val_if_fail (conv != NULL, FALSE);
	g_return_val_if_fail (to_codeset != NULL, FALSE);
	g_return_val_if_fail (from_codeset != NULL, FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail (!is_opened (conv), FALSE);

	conv->conv_descriptor = g_iconv_open (to_codeset, from_codeset);

	if (conv->conv_descriptor == (GIConv)-1)
	{
		gint saved_errno = errno;
		errno = 0;

		if (saved_errno == EINVAL)
		{
			g_set_error (error,
				     G_CONVERT_ERROR,
				     G_CONVERT_ERROR_NO_CONVERSION,
				     _("Conversion from character set “%s” to “%s” is not supported."),
				     from_codeset,
				     to_codeset);
		}
		else
		{
			g_set_error (error,
				     G_CONVERT_ERROR,
				     G_CONVERT_ERROR_FAILED,
				     _("Failed to open a character set converter from “%s” to “%s”: %s"),
				     from_codeset,
				     to_codeset,
				     g_strerror (saved_errno));
		}

		return FALSE;
	}

	return TRUE;
}

/* @error is set only when TEPL_ICONV_RESULT_ERROR is returned. */
TeplIconvResult
_tepl_iconv_feed (TeplIconv  *conv,
		  gchar     **inbuf,
		  gsize      *inbytes_left,
		  gchar     **outbuf,
		  gsize      *outbytes_left,
		  GError    **error)
{
	gsize iconv_ret;

	g_return_val_if_fail (conv != NULL, FALSE);

	/* It's not exactly the same as in the iconv(3) manpage.
	 * If inbuf != NULL it corresponds to the main case.
	 * If inbuf == NULL it corresponds to the second case described in the
	 * manpage.
	 */
	g_return_val_if_fail ((inbuf != NULL && *inbuf != NULL && inbytes_left != NULL) ||
			      (inbuf == NULL && inbytes_left == NULL), FALSE);

	g_return_val_if_fail (outbuf != NULL && *outbuf != NULL, FALSE);
	g_return_val_if_fail (outbytes_left != NULL, FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	iconv_ret = g_iconv (conv->conv_descriptor,
			     inbuf, inbytes_left,
			     outbuf, outbytes_left);

	if (iconv_ret == (gsize)-1)
	{
		gint saved_errno = errno;
		errno = 0;

		if (saved_errno == EILSEQ)
		{
			return TEPL_ICONV_RESULT_INVALID_INPUT_CHAR;
		}
		else if (saved_errno == EINVAL)
		{
			return TEPL_ICONV_RESULT_INCOMPLETE_INPUT;
		}
		else if (saved_errno == E2BIG)
		{
			return TEPL_ICONV_RESULT_OUTPUT_BUFFER_FULL;
		}
		else
		{
			g_set_error (error,
				     G_CONVERT_ERROR,
				     G_CONVERT_ERROR_FAILED,
				     _("Error during character set conversion: %s"),
				     g_strerror (saved_errno));

			return TEPL_ICONV_RESULT_ERROR;
		}
	}

	if (inbytes_left != NULL)
	{
		g_warn_if_fail (*inbytes_left == 0);
	}

	return TEPL_ICONV_RESULT_OK;
}

/* Returns: %TRUE on success, %FALSE otherwise. */
gboolean
_tepl_iconv_close_and_free (TeplIconv  *conv,
			    GError    **error)
{
	gboolean success = TRUE;

	g_return_val_if_fail (conv != NULL, FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	if (is_opened (conv))
	{
		gint close_ret = g_iconv_close (conv->conv_descriptor);

		if (close_ret == -1)
		{
			gint saved_errno = errno;
			errno = 0;

			g_set_error (error,
				     G_CONVERT_ERROR,
				     G_CONVERT_ERROR_FAILED,
				     _("Failed to close the character set converter: %s"),
				     g_strerror (saved_errno));

			success = FALSE;
		}
	}

	g_free (conv);
	return success;
}
