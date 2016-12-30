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

#ifndef GTEF_ENCODING_CONVERTER_H
#define GTEF_ENCODING_CONVERTER_H

#include <glib-object.h>

G_BEGIN_DECLS

#define GTEF_TYPE_ENCODING_CONVERTER             (_gtef_encoding_converter_get_type ())
#define GTEF_ENCODING_CONVERTER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTEF_TYPE_ENCODING_CONVERTER, GtefEncodingConverter))
#define GTEF_ENCODING_CONVERTER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GTEF_TYPE_ENCODING_CONVERTER, GtefEncodingConverterClass))
#define GTEF_IS_ENCODING_CONVERTER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTEF_TYPE_ENCODING_CONVERTER))
#define GTEF_IS_ENCODING_CONVERTER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GTEF_TYPE_ENCODING_CONVERTER))
#define GTEF_ENCODING_CONVERTER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GTEF_TYPE_ENCODING_CONVERTER, GtefEncodingConverterClass))

typedef struct _GtefEncodingConverter         GtefEncodingConverter;
typedef struct _GtefEncodingConverterClass    GtefEncodingConverterClass;
typedef struct _GtefEncodingConverterPrivate  GtefEncodingConverterPrivate;

struct _GtefEncodingConverter
{
	GObject parent;

	GtefEncodingConverterPrivate *priv;
};

struct _GtefEncodingConverterClass
{
	GObjectClass parent_class;
};

/**
 * GtefEncodingConversionCallback:
 * @str: nul-terminated converted contents.
 * @length: length of @str, without the terminating nul-byte.
 * @user_data: user data set when the callback was connected.
 */
typedef void (*GtefEncodingConversionCallback) (const gchar *str,
						gsize        length,
						gpointer     user_data);

G_GNUC_INTERNAL
GType		_gtef_encoding_converter_get_type		(void);

G_GNUC_INTERNAL
GtefEncodingConverter *
		_gtef_encoding_converter_new			(gint64 buffer_size);

G_GNUC_INTERNAL
gint64		_gtef_encoding_converter_get_buffer_size	(GtefEncodingConverter *converter);

G_GNUC_INTERNAL
void		_gtef_encoding_converter_set_callback		(GtefEncodingConverter          *converter,
								 GtefEncodingConversionCallback  callback,
								 gpointer                        user_data);

G_GNUC_INTERNAL
gboolean	_gtef_encoding_converter_open			(GtefEncodingConverter  *converter,
								 const gchar            *to_codeset,
								 const gchar            *from_codeset,
								 GError                **error);

G_GNUC_INTERNAL
gboolean	_gtef_encoding_converter_feed			(GtefEncodingConverter  *converter,
								 const gchar            *chunk,
								 gssize                  size,
								 GError                **error);

G_GNUC_INTERNAL
void		_gtef_encoding_converter_close			(GtefEncodingConverter *converter);

G_END_DECLS

#endif /* GTEF_ENCODING_CONVERTER_H */
