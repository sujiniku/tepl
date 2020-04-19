/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2020 - Sébastien Wilmet <swilmet@gnome.org>
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

#include "tepl-statusbar.h"

/**
 * SECTION:statusbar
 * @Title: TeplStatusbar
 * @Short_description: Subclass of #GtkStatusbar
 */

struct _TeplStatusbarPrivate
{
	gint something;
};

G_DEFINE_TYPE_WITH_PRIVATE (TeplStatusbar, tepl_statusbar, GTK_TYPE_STATUSBAR)

static void
tepl_statusbar_finalize (GObject *object)
{

	G_OBJECT_CLASS (tepl_statusbar_parent_class)->finalize (object);
}

static void
tepl_statusbar_class_init (TeplStatusbarClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = tepl_statusbar_finalize;
}

static void
tepl_statusbar_init (TeplStatusbar *statusbar)
{
	statusbar->priv = tepl_statusbar_get_instance_private (statusbar);
}

/**
 * tepl_statusbar_new:
 *
 * Returns: (transfer floating): a new #TeplStatusbar.
 * Since: 5.0
 */
TeplStatusbar *
tepl_statusbar_new (void)
{
	return g_object_new (TEPL_TYPE_STATUSBAR, NULL);
}
