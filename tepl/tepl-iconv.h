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

#ifndef TEPL_ICONV_H
#define TEPL_ICONV_H

#include <glib.h>

G_BEGIN_DECLS

typedef struct _TeplIconv TeplIconv;

/*
 * TeplIconvResult:
 * @TEPL_ICONV_RESULT_INVALID_INPUT_CHAR: stopped at an invalid character in the
 *   @inbuf. `*inbuf` is left pointing to the beginning of the invalid sequence.
 * @TEPL_ICONV_RESULT_INCOMPLETE_INPUT: the input byte sequence ends with an
 *   incomplete multi-byte character. `*inbuf` is left pointing to the beginning
 *   of the incomplete multi-byte character.
 * @TEPL_ICONV_RESULT_OUTPUT_BUFFER_FULL: the output buffer has no more room for
 *   the next converted character.
 */
typedef enum
{
	TEPL_ICONV_RESULT_OK,
	TEPL_ICONV_RESULT_ERROR,
	TEPL_ICONV_RESULT_INVALID_INPUT_CHAR,
	TEPL_ICONV_RESULT_INCOMPLETE_INPUT,
	TEPL_ICONV_RESULT_OUTPUT_BUFFER_FULL
} TeplIconvResult;

TeplIconv *	_tepl_iconv_new			(void);

gboolean	_tepl_iconv_open		(TeplIconv    *conv,
						 const gchar  *to_codeset,
						 const gchar  *from_codeset,
						 GError      **error);

TeplIconvResult	_tepl_iconv_feed		(TeplIconv  *conv,
						 gchar     **inbuf,
						 gsize      *inbytes_left,
						 gchar     **outbuf,
						 gsize      *outbytes_left,
						 GError    **error);

TeplIconvResult	_tepl_iconv_feed_finish		(TeplIconv  *conv,
						 gchar     **outbuf,
						 gsize      *outbytes_left,
						 GError    **error);

gboolean	_tepl_iconv_close_and_free	(TeplIconv  *conv,
						 GError    **error);

G_END_DECLS

#endif /* TEPL_ICONV_H */
