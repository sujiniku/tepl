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

#include "gtef-tab.h"

/**
 * SECTION:tab
 * @Short_description: Contains a GtefView with GtkInfoBars on top
 * @Title: GtefTab
 */

typedef struct _GtefTabPrivate GtefTabPrivate;

struct _GtefTabPrivate
{
	GtkWidget *main_widget;
};

G_DEFINE_TYPE_WITH_PRIVATE (GtefTab, gtef_tab, GTK_TYPE_GRID)

static void
gtef_tab_dispose (GObject *object)
{
	GtefTabPrivate *priv = gtef_tab_get_instance_private (GTEF_TAB (object));

	g_clear_object (&priv->main_widget);

	G_OBJECT_CLASS (gtef_tab_parent_class)->dispose (object);
}

static void
gtef_tab_class_init (GtefTabClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = gtef_tab_dispose;
}

static void
gtef_tab_init (GtefTab *tab)
{
	gtk_orientable_set_orientation (GTK_ORIENTABLE (tab), GTK_ORIENTATION_VERTICAL);
}

/**
 * gtef_tab_new:
 * @main_widget: the main #GtkWidget that will be contained in the #GtefTab.
 *
 * Returns: a new #GtefTab.
 * Since: 1.0
 */
GtefTab *
gtef_tab_new (GtkWidget *main_widget)
{
	GtefTab *tab;
	GtefTabPrivate *priv;

	g_return_val_if_fail (GTK_IS_WIDGET (main_widget), NULL);

	tab = g_object_new (GTEF_TYPE_TAB, NULL);
	priv = gtef_tab_get_instance_private (tab);

	gtk_container_add (GTK_CONTAINER (tab), main_widget);
	priv->main_widget = g_object_ref_sink (main_widget);

	return tab;
}

/**
 * gtef_tab_add_info_bar:
 * @tab: a #GtefTab.
 * @info_bar: a #GtkInfoBar.
 *
 * Attaches @info_bar to @tab, above the main widget.
 *
 * If several info bars are added, the first one will be at the top, the second
 * one below the first info bar, etc. With the main widget of @tab at the
 * bottom.
 *
 * Since: 1.0
 */
void
gtef_tab_add_info_bar (GtefTab    *tab,
		       GtkInfoBar *info_bar)
{
	GtefTabPrivate *priv;

	g_return_if_fail (GTEF_IS_TAB (tab));
	g_return_if_fail (GTK_IS_INFO_BAR (info_bar));

	priv = gtef_tab_get_instance_private (tab);

	gtk_grid_insert_next_to (GTK_GRID (tab),
				 priv->main_widget,
				 GTK_POS_TOP);

	gtk_grid_attach_next_to (GTK_GRID (tab),
				 GTK_WIDGET (info_bar),
				 priv->main_widget,
				 GTK_POS_TOP,
				 1, 1);
}
