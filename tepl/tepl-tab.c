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
#include "tepl-buffer.h"
#include "tepl-info-bar.h"
#include "tepl-tab-list.h"

/**
 * SECTION:tab
 * @Short_description: Contains a TeplView and GtkInfoBars
 * @Title: TeplTab
 *
 * #TeplTab is meant to be the content of one tab in the text editor (if the
 * text editor has a Tabbed Document Interface). It is a #GtkGrid container that
 * contains the #TeplView and can contain one or several #GtkInfoBar's.
 *
 * By default:
 * - #TeplTab has a vertical #GtkOrientation.
 * - The main child widget of #TeplTab is a #GtkScrolledWindow which contains
 *   the #TeplView.
 * - #GtkInfoBar's are added on top of the #GtkScrolledWindow.
 *
 * The way that the #TeplView is packed into the #TeplTab is customizable with
 * the ::pack_view virtual function. Similarly, the way that #GtkInfoBar's are
 * added can be customized with ::pack_info_bar.
 *
 * #TeplTab implements the #TeplTabList interface, for a list of only one tab.
 * It is useful for text editors that open each file in a separate window.
 */

struct _TeplTabPrivate
{
	TeplView *view;
};

enum
{
	PROP_0,
	PROP_VIEW,
	N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

static void tepl_tab_list_interface_init (gpointer g_iface,
					  gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE (TeplTab,
			 tepl_tab,
			 GTK_TYPE_GRID,
			 G_ADD_PRIVATE (TeplTab)
			 G_IMPLEMENT_INTERFACE (TEPL_TYPE_TAB_LIST,
						tepl_tab_list_interface_init))

static GtkScrolledWindow *
create_scrolled_window (void)
{
	GtkScrolledWindow *scrolled_window;

	scrolled_window = GTK_SCROLLED_WINDOW (gtk_scrolled_window_new (NULL, NULL));

	/* Disable overlay scrolling, it doesn't work well with GtkTextView. For
	 * example to place the cursor with the mouse on the last character of a
	 * line.
	 */
	gtk_scrolled_window_set_overlay_scrolling (scrolled_window, FALSE);

	g_object_set (scrolled_window,
		      "expand", TRUE,
		      NULL);

	gtk_widget_show (GTK_WIDGET (scrolled_window));

	return scrolled_window;
}

static void
tepl_tab_pack_view_default (TeplTab  *tab,
			    TeplView *view)
{
	GtkScrolledWindow *scrolled_window;

	scrolled_window = create_scrolled_window ();

	gtk_container_add (GTK_CONTAINER (scrolled_window),
			   GTK_WIDGET (view));

	gtk_container_add (GTK_CONTAINER (tab),
			   GTK_WIDGET (scrolled_window));
}

static void
tepl_tab_pack_info_bar_default (TeplTab    *tab,
				GtkInfoBar *info_bar)
{
	GList *children;
	GList *l;
	GtkWidget *sibling = NULL;

	children = gtk_container_get_children (GTK_CONTAINER (tab));

	for (l = children; l != NULL; l = l->next)
	{
		GtkWidget *child = l->data;

		if (!GTK_IS_INFO_BAR (child))
		{
			sibling = child;
			break;
		}
	}

	g_list_free (children);

	if (sibling != NULL)
	{
		gtk_grid_insert_next_to (GTK_GRID (tab), sibling, GTK_POS_TOP);

		gtk_grid_attach_next_to (GTK_GRID (tab),
					 GTK_WIDGET (info_bar),
					 sibling,
					 GTK_POS_TOP,
					 1, 1);
	}
	else
	{
		gtk_container_add (GTK_CONTAINER (tab), GTK_WIDGET (info_bar));
	}
}

static void
set_view (TeplTab  *tab,
	  TeplView *view)
{
	if (view == NULL)
	{
		/* For tepl_tab_new(). */
		view = TEPL_VIEW (tepl_view_new ());
		gtk_widget_show (GTK_WIDGET (view));
	}

	g_return_if_fail (TEPL_IS_VIEW (view));

	g_assert (tab->priv->view == NULL);
	tab->priv->view = g_object_ref_sink (view);

	TEPL_TAB_GET_CLASS (tab)->pack_view (tab, view);

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

	G_OBJECT_CLASS (tepl_tab_parent_class)->dispose (object);
}

static void
tepl_tab_class_init (TeplTabClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = tepl_tab_get_property;
	object_class->set_property = tepl_tab_set_property;
	object_class->dispose = tepl_tab_dispose;

	klass->pack_view = tepl_tab_pack_view_default;
	klass->pack_info_bar = tepl_tab_pack_info_bar_default;

	/**
	 * TeplTab:view:
	 *
	 * The #TeplView contained in the tab. When this property is set, the
	 * ::pack_view virtual function is called.
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

static GList *
tepl_tab_get_tabs (TeplTabList *tab_list)
{
	return g_list_append (NULL, TEPL_TAB (tab_list));
}

static TeplTab *
tepl_tab_get_active_tab (TeplTabList *tab_list)
{
	return TEPL_TAB (tab_list);
}

static void
tepl_tab_list_interface_init (gpointer g_iface,
			      gpointer iface_data)
{
	TeplTabListInterface *interface = g_iface;

	interface->get_tabs = tepl_tab_get_tabs;
	interface->get_active_tab = tepl_tab_get_active_tab;
}

static void
tepl_tab_init (TeplTab *tab)
{
	tab->priv = tepl_tab_get_instance_private (tab);

	gtk_orientable_set_orientation (GTK_ORIENTABLE (tab), GTK_ORIENTATION_VERTICAL);
}

/**
 * tepl_tab_new:
 *
 * Creates a new #TeplTab with a new #TeplView. The new #TeplView can be
 * retrieved afterwards with tepl_tab_get_view().
 *
 * Returns: a new #TeplTab.
 * Since: 3.0
 */
TeplTab *
tepl_tab_new (void)
{
	return g_object_new (TEPL_TYPE_TAB, NULL);
}

/**
 * tepl_tab_new_with_view:
 * @view: the #TeplView that will be contained in the tab.
 *
 * Returns: a new #TeplTab.
 * Since: 3.0
 */
TeplTab *
tepl_tab_new_with_view (TeplView *view)
{
	g_return_val_if_fail (TEPL_IS_VIEW (view), NULL);

	return g_object_new (TEPL_TYPE_TAB,
			     "view", view,
			     NULL);
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
 * tepl_tab_get_buffer:
 * @tab: a #TeplTab.
 *
 * A convenience function that calls gtk_text_view_get_buffer() on the
 * #TeplTab:view associated with the @tab.
 *
 * Returns: (transfer none): the #TeplBuffer of the #TeplTab:view.
 * Since: 3.0
 */
TeplBuffer *
tepl_tab_get_buffer (TeplTab *tab)
{
	g_return_val_if_fail (TEPL_IS_TAB (tab), NULL);

	return TEPL_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (tab->priv->view)));
}

/**
 * tepl_tab_add_info_bar:
 * @tab: a #TeplTab.
 * @info_bar: a #GtkInfoBar.
 *
 * Attaches @info_bar to @tab.
 *
 * This function calls the ::pack_info_bar virtual function.
 *
 * Since: 1.0
 */
void
tepl_tab_add_info_bar (TeplTab    *tab,
		       GtkInfoBar *info_bar)
{
	g_return_if_fail (TEPL_IS_TAB (tab));
	g_return_if_fail (GTK_IS_INFO_BAR (info_bar));

	_tepl_info_bar_set_size_request (info_bar);

	TEPL_TAB_GET_CLASS (tab)->pack_info_bar (tab, info_bar);
}
