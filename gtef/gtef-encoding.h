/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2002-2005 - Paolo Maggi
 * Copyright 2014, 2015, 2017 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef GTEF_ENCODING_H
#define GTEF_ENCODING_H

#if !defined (GTEF_H_INSIDE) && !defined (GTEF_COMPILATION)
#error "Only <gtef/gtef.h> can be included directly."
#endif

#include <glib-object.h>
#include <gtef/gtef-types.h>

G_BEGIN_DECLS

#define GTEF_TYPE_ENCODING (gtef_encoding_get_type ())

GType			gtef_encoding_get_type			(void) G_GNUC_CONST;

GtefEncoding *		gtef_encoding_new			(const gchar *charset);

GtefEncoding *		gtef_encoding_new_utf8			(void);

GtefEncoding *		gtef_encoding_new_from_locale		(void);

GtefEncoding *		gtef_encoding_copy			(const GtefEncoding *enc);

void			gtef_encoding_free			(GtefEncoding *enc);

const gchar *		gtef_encoding_get_charset		(const GtefEncoding *enc);

const gchar *		gtef_encoding_get_name			(const GtefEncoding *enc);

gchar *			gtef_encoding_to_string			(const GtefEncoding *enc);

gboolean		gtef_encoding_is_utf8			(const GtefEncoding *enc);

gboolean		gtef_encoding_equals			(const GtefEncoding *enc1,
								 const GtefEncoding *enc2);

GSList *		gtef_encoding_get_all			(void);

GSList *		gtef_encoding_get_default_candidates	(void);

G_END_DECLS

#endif  /* GTEF_ENCODING_H */
