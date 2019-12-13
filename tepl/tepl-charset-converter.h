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

#ifndef TEPL_CHARSET_CONVERTER_H
#define TEPL_CHARSET_CONVERTER_H

#include <glib.h>

G_BEGIN_DECLS

typedef struct _TeplCharsetConverter TeplCharsetConverter;

TeplCharsetConverter *	_tepl_charset_converter_new		(gssize   buffer_size,
								 gboolean discard_output);

gboolean		_tepl_charset_converter_open		(TeplCharsetConverter  *charset_converter,
								 const gchar           *from_charset,
								 const gchar           *to_charset,
								 GError               **error);

gboolean		_tepl_charset_converter_close		(TeplCharsetConverter  *charset_converter,
								 GError               **error);

void			_tepl_charset_converter_free		(TeplCharsetConverter *charset_converter);

G_END_DECLS

#endif /* TEPL_CHARSET_CONVERTER_H */
