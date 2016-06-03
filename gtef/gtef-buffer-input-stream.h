/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2010 - Ignacio Casal Quinteiro
 * Copyright 2014, 2016 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GTEF_BUFFER_INPUT_STREAM_H
#define GTEF_BUFFER_INPUT_STREAM_H

#include <gtksourceview/gtksource.h>

G_BEGIN_DECLS

#define GTEF_TYPE_BUFFER_INPUT_STREAM			(_gtef_buffer_input_stream_get_type ())
#define GTEF_BUFFER_INPUT_STREAM(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTEF_TYPE_BUFFER_INPUT_STREAM, GtefBufferInputStream))
#define GTEF_BUFFER_INPUT_STREAM_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GTEF_TYPE_BUFFER_INPUT_STREAM, GtefBufferInputStreamClass))
#define GTEF_IS_BUFFER_INPUT_STREAM(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTEF_TYPE_BUFFER_INPUT_STREAM))
#define GTEF_IS_BUFFER_INPUT_STREAM_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GTEF_TYPE_BUFFER_INPUT_STREAM))
#define GTEF_BUFFER_INPUT_STREAM_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), GTEF_TYPE_BUFFER_INPUT_STREAM, GtefBufferInputStreamClass))

typedef struct _GtefBufferInputStream		GtefBufferInputStream;
typedef struct _GtefBufferInputStreamClass	GtefBufferInputStreamClass;
typedef struct _GtefBufferInputStreamPrivate	GtefBufferInputStreamPrivate;

struct _GtefBufferInputStream
{
	GInputStream parent;

	GtefBufferInputStreamPrivate *priv;
};

struct _GtefBufferInputStreamClass
{
	GInputStreamClass parent_class;
};

G_GNUC_INTERNAL
GType		_gtef_buffer_input_stream_get_type		(void);

G_GNUC_INTERNAL
GtefBufferInputStream *
		_gtef_buffer_input_stream_new			(GtkTextBuffer        *buffer,
								 GtkSourceNewlineType  type,
								 gboolean              add_trailing_newline);

G_GNUC_INTERNAL
gsize		_gtef_buffer_input_stream_get_total_size	(GtefBufferInputStream *stream);

G_GNUC_INTERNAL
gsize		_gtef_buffer_input_stream_tell			(GtefBufferInputStream *stream);

G_END_DECLS

#endif /* GTEF_BUFFER_INPUT_STREAM_H */
