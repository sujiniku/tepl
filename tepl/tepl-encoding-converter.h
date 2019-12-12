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

#ifndef TEPL_ENCODING_CONVERTER_H
#define TEPL_ENCODING_CONVERTER_H

#include <glib.h>
#include "tepl-types.h"

G_BEGIN_DECLS

typedef struct _TeplEncodingConverterOutputChunk TeplEncodingConverterOutputChunk;

/*
 * TeplEncodingConverterOutputChunk:
 * @is_valid: if %TRUE, @bytes contains successfully converted characters. If
 *   %FALSE, @bytes contains unconverted input bytes for which the conversion
 *   failed.
 */
struct _TeplEncodingConverterOutputChunk
{
	GBytes *bytes;
	guint is_valid : 1;
};

gboolean	_tepl_encoding_converter_convert		(GList         *input_chunks,
								 TeplEncoding  *from_encoding,
								 TeplEncoding  *to_encoding,
								 gint64         max_output_chunk_size,
								 GList        **output_chunks,
								 GError       **error);

gboolean	_tepl_encoding_converter_test_conversion	(GList        *input_chunks,
								 TeplEncoding *from_encoding,
								 TeplEncoding *to_encoding,
								 gint         *n_invalid_input_chars);

void		_tepl_encoding_converter_output_chunk_free	(TeplEncodingConverterOutputChunk *output_chunk);

G_END_DECLS

#endif /* TEPL_ENCODING_CONVERTER_H */
