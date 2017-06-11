/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2010 - Ignacio Casal Quinteiro
 * Copyright 2014, 2016 - Sébastien Wilmet <swilmet@gnome.org>
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
