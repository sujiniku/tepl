/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2017 - Sébastien Wilmet <swilmet@gnome.org>
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

#include "tepl-file-content.h"

struct _TeplFileContentPrivate
{
	/* List of GBytes*. */
	GQueue *chunks;
};

G_DEFINE_TYPE_WITH_PRIVATE (TeplFileContent, _tepl_file_content, G_TYPE_OBJECT)

static void
_tepl_file_content_finalize (GObject *object)
{
	TeplFileContent *content = TEPL_FILE_CONTENT (object);

	if (content->priv->chunks != NULL)
	{
		g_queue_free_full (content->priv->chunks, (GDestroyNotify)g_bytes_unref);
		content->priv->chunks = NULL;
	}

	G_OBJECT_CLASS (_tepl_file_content_parent_class)->finalize (object);
}

static void
_tepl_file_content_class_init (TeplFileContentClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = _tepl_file_content_finalize;
}

static void
_tepl_file_content_init (TeplFileContent *content)
{
	content->priv = _tepl_file_content_get_instance_private (content);

	content->priv->chunks = g_queue_new ();
}

TeplFileContent *
_tepl_file_content_new (void)
{
	return g_object_new (TEPL_TYPE_FILE_CONTENT, NULL);
}

void
_tepl_file_content_add_chunk (TeplFileContent *content,
			      GBytes          *chunk)
{
	g_return_if_fail (TEPL_IS_FILE_CONTENT (content));
	g_return_if_fail (chunk != NULL);

	g_queue_push_tail (content->priv->chunks, g_bytes_ref (chunk));
}

GQueue *
_tepl_file_content_get_chunks (TeplFileContent *content)
{
	g_return_val_if_fail (TEPL_IS_FILE_CONTENT (content), NULL);

	return content->priv->chunks;
}
