/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
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

#include "gtef-buffer.h"

/**
 * SECTION:buffer
 * @Short_description: Subclass of GtkSourceBuffer
 * @Title: GtefBuffer
 */

typedef struct _GtefBufferPrivate GtefBufferPrivate;

struct _GtefBufferPrivate
{
	GtefFile *file;
};

G_DEFINE_TYPE_WITH_PRIVATE (GtefBuffer, gtef_buffer, GTK_SOURCE_TYPE_BUFFER)

static void
gtef_buffer_dispose (GObject *object)
{
	GtefBufferPrivate *priv = gtef_buffer_get_instance_private (GTEF_BUFFER (object));

	g_clear_object (&priv->file);

	G_OBJECT_CLASS (gtef_buffer_parent_class)->dispose (object);
}

static void
gtef_buffer_class_init (GtefBufferClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = gtef_buffer_dispose;
}

static void
gtef_buffer_init (GtefBuffer *buffer)
{
	GtefBufferPrivate *priv;

	priv = gtef_buffer_get_instance_private (buffer);

	priv->file = gtef_file_new ();
}

/**
 * gtef_buffer_get_file:
 * @buffer: a #GtefBuffer.
 *
 * Returns: (transfer none): the associated #GtefFile.
 * Since: 1.0
 */
GtefFile *
gtef_buffer_get_file (GtefBuffer *buffer)
{
	GtefBufferPrivate *priv;

	g_return_val_if_fail (GTEF_IS_BUFFER (buffer), NULL);

	priv = gtef_buffer_get_instance_private (buffer);
	return priv->file;
}
