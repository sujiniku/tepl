/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2016, 2017 - Sébastien Wilmet <swilmet@gnome.org>
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

#include "tepl-tab.h"
#include "tepl-view.h"

/**
 * SECTION:tab
 * @Short_description: Contains a TeplView with GtkInfoBars on top
 * @Title: TeplTab
 */

struct _TeplTabPrivate
{
	TeplView *view;
	GtkWidget *main_widget;
};

enum
{
	PROP_0,
	PROP_VIEW,
	N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

G_DEFINE_TYPE_WITH_PRIVATE (TeplTab, tepl_tab, GTK_TYPE_GRID)

static void
set_view (TeplTab  *tab,
	  TeplView *view)
{
	g_return_if_fail (TEPL_IS_VIEW (view));

	g_assert (tab->priv->view == NULL);
	tab->priv->view = g_object_ref_sink (view);

	g_object_notify_by_pspec (G_OBJECT (tab), properties[PROP_VIEW]);
}

static void
tepl_tab_get_property (GObject    *object,
                       guint       prop_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
	TeplTab *tab = TEPL_TAB (object);

	switch (prop_id)
	{
		case PROP_VIEW:
			g_value_set_object (value, tepl_tab_get_view (tab));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_tab_set_property (GObject      *object,
                       guint         prop_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
	TeplTab *tab = TEPL_TAB (object);

	switch (prop_id)
	{
		case PROP_VIEW:
			set_view (tab, g_value_get_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_tab_dispose (GObject *object)
{
	TeplTab *tab = TEPL_TAB (object);

	g_clear_object (&tab->priv->view);
	g_clear_object (&tab->priv->main_widget);

	G_OBJECT_CLASS (tepl_tab_parent_class)->dispose (object);
}

static void
tepl_tab_class_init (TeplTabClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = tepl_tab_get_property;
	object_class->set_property = tepl_tab_set_property;
	object_class->dispose = tepl_tab_dispose;

	/**
	 * TeplTab:view:
	 *
	 * The #TeplView contained in the tab.
	 *
	 * Since: 3.0
	 */
	properties[PROP_VIEW] =
		g_param_spec_object ("view",
				     "View",
				     "",
				     TEPL_TYPE_VIEW,
				     G_PARAM_READWRITE |
				     G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
tepl_tab_init (TeplTab *tab)
{
	tab->priv = tepl_tab_get_instance_private (tab);

	gtk_orientable_set_orientation (GTK_ORIENTABLE (tab), GTK_ORIENTATION_VERTICAL);
}

/**
 * tepl_tab_new:
 * @main_widget: the main #GtkWidget that will be contained in the #TeplTab.
 *
 * Returns: a new #TeplTab.
 * Since: 1.0
 */
TeplTab *
tepl_tab_new (GtkWidget *main_widget)
{
	TeplTab *tab;

	g_return_val_if_fail (GTK_IS_WIDGET (main_widget), NULL);

	tab = g_object_new (TEPL_TYPE_TAB, NULL);

	gtk_container_add (GTK_CONTAINER (tab), main_widget);
	tab->priv->main_widget = g_object_ref_sink (main_widget);

	return tab;
}

/**
 * tepl_tab_get_view:
 * @tab: a #TeplTab.
 *
 * Returns: (transfer none): the #TeplView contained in @tab.
 * Since: 3.0
 */
TeplView *
tepl_tab_get_view (TeplTab *tab)
{
	g_return_val_if_fail (TEPL_IS_TAB (tab), NULL);

	return tab->priv->view;
}

/**
 * tepl_tab_add_info_bar:
 * @tab: a #TeplTab.
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
tepl_tab_add_info_bar (TeplTab    *tab,
		       GtkInfoBar *info_bar)
{
	g_return_if_fail (TEPL_IS_TAB (tab));
	g_return_if_fail (GTK_IS_INFO_BAR (info_bar));

	gtk_grid_insert_next_to (GTK_GRID (tab),
				 tab->priv->main_widget,
				 GTK_POS_TOP);

	gtk_grid_attach_next_to (GTK_GRID (tab),
				 GTK_WIDGET (info_bar),
				 tab->priv->main_widget,
				 GTK_POS_TOP,
				 1, 1);
}
