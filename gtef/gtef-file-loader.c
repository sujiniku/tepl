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

#include "gtef-file-loader.h"

/**
 * SECTION:file-loader
 * @Short_description: Load a file into a GtkSourceBuffer
 * @Title: GtefFileLoader
 */

typedef struct _GtefFileLoaderPrivate GtefFileLoaderPrivate;

struct _GtefFileLoaderPrivate
{
	gint something;
};

G_DEFINE_TYPE_WITH_PRIVATE (GtefFileLoader, gtef_file_loader, G_TYPE_OBJECT)

static void
gtef_file_loader_dispose (GObject *object)
{

	G_OBJECT_CLASS (gtef_file_loader_parent_class)->dispose (object);
}

static void
gtef_file_loader_finalize (GObject *object)
{

	G_OBJECT_CLASS (gtef_file_loader_parent_class)->finalize (object);
}

static void
gtef_file_loader_class_init (GtefFileLoaderClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = gtef_file_loader_dispose;
	object_class->finalize = gtef_file_loader_finalize;
}

static void
gtef_file_loader_init (GtefFileLoader *loader)
{
}

/**
 * gtef_file_loader_new:
 *
 * Returns: a new #GtefFileLoader object.
 * Since: 1.0
 */
GtefFileLoader *
gtef_file_loader_new (void)
{
	return g_object_new (GTEF_TYPE_FILE_LOADER, NULL);
}
