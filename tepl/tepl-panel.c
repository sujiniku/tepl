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

#include "tepl-panel.h"

/**
 * SECTION:panel
 * @Title: TeplPanel
 * @Short_description: Side or bottom panel
 *
 * #TeplPanel permits to create a side or bottom panel with several components.
 */

struct _TeplPanelPrivate
{
	gint something;
};

G_DEFINE_TYPE_WITH_PRIVATE (TeplPanel, tepl_panel, GTK_TYPE_GRID)

static void
tepl_panel_finalize (GObject *object)
{

	G_OBJECT_CLASS (tepl_panel_parent_class)->finalize (object);
}

static void
tepl_panel_class_init (TeplPanelClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = tepl_panel_finalize;
}

static void
tepl_panel_init (TeplPanel *panel)
{
	panel->priv = tepl_panel_get_instance_private (panel);
}

/**
 * tepl_panel_new:
 *
 * Returns: (transfer floating): a new #TeplPanel.
 * Since: 5.0
 */
TeplPanel *
tepl_panel_new (void)
{
	return g_object_new (TEPL_TYPE_PANEL, NULL);
}
