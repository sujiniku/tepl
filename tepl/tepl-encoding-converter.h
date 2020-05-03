/* Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_ENCODING_CONVERTER_H
#define TEPL_ENCODING_CONVERTER_H

#include <glib-object.h>

G_BEGIN_DECLS

#define TEPL_TYPE_ENCODING_CONVERTER             (_tepl_encoding_converter_get_type ())
#define TEPL_ENCODING_CONVERTER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_ENCODING_CONVERTER, TeplEncodingConverter))
#define TEPL_ENCODING_CONVERTER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_ENCODING_CONVERTER, TeplEncodingConverterClass))
#define TEPL_IS_ENCODING_CONVERTER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_ENCODING_CONVERTER))
#define TEPL_IS_ENCODING_CONVERTER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_ENCODING_CONVERTER))
#define TEPL_ENCODING_CONVERTER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_ENCODING_CONVERTER, TeplEncodingConverterClass))

typedef struct _TeplEncodingConverter         TeplEncodingConverter;
typedef struct _TeplEncodingConverterClass    TeplEncodingConverterClass;
typedef struct _TeplEncodingConverterPrivate  TeplEncodingConverterPrivate;

struct _TeplEncodingConverter
{
	GObject parent;

	TeplEncodingConverterPrivate *priv;
};

struct _TeplEncodingConverterClass
{
	GObjectClass parent_class;
};

/**
 * TeplEncodingConversionCallback:
 * @str: nul-terminated converted contents.
 * @length: length of @str, without the terminating nul-byte.
 * @user_data: user data set when the callback was connected.
 *
 * @str must not be freed, it is owned by the #TeplEncodingConverter. But to
 * avoid re-allocation, @str can be modified if needed, for example to set a
 * nul-byte at a different place; as long as you modify and access @str inside
 * its [0, @length] bounds.
 */
/* TODO: when there is an invalid char, call a callback instead of returning an
 * error. By extending this callback, or by creating another callback function.
 */
typedef void (*TeplEncodingConversionCallback) (const gchar *str,
						gsize        length,
						gpointer     user_data);

G_GNUC_INTERNAL
GType		_tepl_encoding_converter_get_type		(void);

G_GNUC_INTERNAL
TeplEncodingConverter *
		_tepl_encoding_converter_new			(gint64 buffer_size);

G_GNUC_INTERNAL
gint64		_tepl_encoding_converter_get_buffer_size	(TeplEncodingConverter *converter);

G_GNUC_INTERNAL
void		_tepl_encoding_converter_set_callback		(TeplEncodingConverter          *converter,
								 TeplEncodingConversionCallback  callback,
								 gpointer                        user_data);

G_GNUC_INTERNAL
gboolean	_tepl_encoding_converter_open			(TeplEncodingConverter  *converter,
								 const gchar            *to_codeset,
								 const gchar            *from_codeset,
								 GError                **error);

G_GNUC_INTERNAL
gboolean	_tepl_encoding_converter_feed			(TeplEncodingConverter  *converter,
								 const gchar            *chunk,
								 gssize                  size,
								 GError                **error);

G_GNUC_INTERNAL
gboolean	_tepl_encoding_converter_close			(TeplEncodingConverter  *converter,
								 GError                **error);

G_END_DECLS

#endif /* TEPL_ENCODING_CONVERTER_H */
