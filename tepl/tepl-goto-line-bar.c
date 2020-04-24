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

#include "tepl-goto-line-bar.h"

struct _TeplGotoLineBarPrivate
{
	gint something;
};

G_DEFINE_TYPE_WITH_PRIVATE (TeplGotoLineBar, tepl_goto_line_bar, GTK_TYPE_GRID)

static void
tepl_goto_line_bar_dispose (GObject *object)
{

	G_OBJECT_CLASS (tepl_goto_line_bar_parent_class)->dispose (object);
}

static void
tepl_goto_line_bar_class_init (TeplGotoLineBarClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = tepl_goto_line_bar_dispose;
}

static void
tepl_goto_line_bar_init (TeplGotoLineBar *bar)
{
	bar->priv = tepl_goto_line_bar_get_instance_private (bar);
}

TeplGotoLineBar *
tepl_goto_line_bar_new (void)
{
	return g_object_new (TEPL_TYPE_GOTO_LINE_BAR, NULL);
}
