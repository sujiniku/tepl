/* SPDX-FileCopyrightText: 2010 - Ignacio Casal Quinteiro
 * SPDX-FileCopyrightText: 2014, 2016 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_BUFFER_INPUT_STREAM_H
#define TEPL_BUFFER_INPUT_STREAM_H

#include <gtk/gtk.h>
#include "tepl-file.h"

G_BEGIN_DECLS

#define TEPL_TYPE_BUFFER_INPUT_STREAM			(_tepl_buffer_input_stream_get_type ())
#define TEPL_BUFFER_INPUT_STREAM(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_BUFFER_INPUT_STREAM, TeplBufferInputStream))
#define TEPL_BUFFER_INPUT_STREAM_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_BUFFER_INPUT_STREAM, TeplBufferInputStreamClass))
#define TEPL_IS_BUFFER_INPUT_STREAM(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_BUFFER_INPUT_STREAM))
#define TEPL_IS_BUFFER_INPUT_STREAM_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_BUFFER_INPUT_STREAM))
#define TEPL_BUFFER_INPUT_STREAM_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_BUFFER_INPUT_STREAM, TeplBufferInputStreamClass))

typedef struct _TeplBufferInputStream		TeplBufferInputStream;
typedef struct _TeplBufferInputStreamClass	TeplBufferInputStreamClass;
typedef struct _TeplBufferInputStreamPrivate	TeplBufferInputStreamPrivate;

struct _TeplBufferInputStream
{
	GInputStream parent;

	TeplBufferInputStreamPrivate *priv;
};

struct _TeplBufferInputStreamClass
{
	GInputStreamClass parent_class;
};

G_GNUC_INTERNAL
GType		_tepl_buffer_input_stream_get_type		(void);

G_GNUC_INTERNAL
TeplBufferInputStream *
		_tepl_buffer_input_stream_new			(GtkTextBuffer        *buffer,
								 TeplNewlineType       type,
								 gboolean              add_trailing_newline);

G_GNUC_INTERNAL
gsize		_tepl_buffer_input_stream_get_total_size	(TeplBufferInputStream *stream);

G_GNUC_INTERNAL
gsize		_tepl_buffer_input_stream_tell			(TeplBufferInputStream *stream);

G_END_DECLS

#endif /* TEPL_BUFFER_INPUT_STREAM_H */
