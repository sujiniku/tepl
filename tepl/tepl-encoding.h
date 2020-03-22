/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2002-2005 - Paolo Maggi
 * Copyright 2014, 2015, 2017 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef TEPL_ENCODING_H
#define TEPL_ENCODING_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <glib-object.h>

G_BEGIN_DECLS

#define TEPL_TYPE_ENCODING (tepl_encoding_get_type ())

typedef struct _TeplEncoding TeplEncoding;

GType			tepl_encoding_get_type			(void) G_GNUC_CONST;

TeplEncoding *		tepl_encoding_new			(const gchar *charset);

TeplEncoding *		tepl_encoding_new_utf8			(void);

TeplEncoding *		tepl_encoding_new_from_locale		(void);

TeplEncoding *		tepl_encoding_copy			(const TeplEncoding *enc);

void			tepl_encoding_free			(TeplEncoding *enc);

const gchar *		tepl_encoding_get_charset		(const TeplEncoding *enc);

const gchar *		tepl_encoding_get_name			(const TeplEncoding *enc);

gchar *			tepl_encoding_to_string			(const TeplEncoding *enc);

gboolean		tepl_encoding_is_utf8			(const TeplEncoding *enc);

gboolean		tepl_encoding_equals			(const TeplEncoding *enc1,
								 const TeplEncoding *enc2);

GSList *		tepl_encoding_get_all			(void);

GSList *		tepl_encoding_get_default_candidates	(void);

G_END_DECLS

#endif  /* TEPL_ENCODING_H */
