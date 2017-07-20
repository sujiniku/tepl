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

#include "tepl-notebook.h"
#include "tepl-tab-group.h"
#include "tepl-tab.h"

/**
 * SECTION:notebook
 * @Short_description: Subclass of #GtkNotebook implementing the #TeplTabGroup
 * interface
 * @Title: TeplNotebook
 *
 * #TeplNotebook is a subclass of #GtkNotebook that implements the #TeplTabGroup
 * interface.
 */

struct _TeplNotebookPrivate
{
	gint something;
};

enum
{
	PROP_0,
	PROP_ACTIVE_TAB,
};

static void tepl_tab_group_interface_init (gpointer g_iface,
					   gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE (TeplNotebook,
			 tepl_notebook,
			 GTK_TYPE_NOTEBOOK,
			 G_ADD_PRIVATE (TeplNotebook)
			 G_IMPLEMENT_INTERFACE (TEPL_TYPE_TAB_GROUP,
						tepl_tab_group_interface_init))

static void
tepl_notebook_get_property (GObject    *object,
			    guint       prop_id,
			    GValue     *value,
			    GParamSpec *pspec)
{
	TeplTabGroup *tab_group = TEPL_TAB_GROUP (object);

	switch (prop_id)
	{
		case PROP_ACTIVE_TAB:
			g_value_set_object (value, tepl_tab_group_get_active_tab (tab_group));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_notebook_set_property (GObject      *object,
			    guint         prop_id,
			    const GValue *value,
			    GParamSpec   *pspec)
{
	switch (prop_id)
	{
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_notebook_switch_page (GtkNotebook *notebook,
			   GtkWidget   *page,
			   guint        page_num)
{
	if (GTK_NOTEBOOK_CLASS (tepl_notebook_parent_class)->switch_page != NULL)
	{
		GTK_NOTEBOOK_CLASS (tepl_notebook_parent_class)->switch_page (notebook,
									      page,
									      page_num);
	}

	/* FIXME: we connect only to the switch-page signal to notify the
	 * active-tab property. Is it enough? Do we also need to connect to
	 * other GtkNotebook signals?
	 */
	g_object_notify (G_OBJECT (notebook), "active-tab");
}

static void
tepl_notebook_class_init (TeplNotebookClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkNotebookClass *notebook_class = GTK_NOTEBOOK_CLASS (klass);

	object_class->get_property = tepl_notebook_get_property;
	object_class->set_property = tepl_notebook_set_property;

	notebook_class->switch_page = tepl_notebook_switch_page;

	g_object_class_override_property (object_class, PROP_ACTIVE_TAB, "active-tab");
}

static GList *
tepl_notebook_get_tabs (TeplTabGroup *tab_group)
{
	GtkNotebook *notebook = GTK_NOTEBOOK (tab_group);
	GList *tabs = NULL;
	gint n_pages;
	gint page_num;

	n_pages = gtk_notebook_get_n_pages (notebook);
	for (page_num = n_pages - 1; page_num >= 0; page_num--)
	{
		GtkWidget *page_widget;

		page_widget = gtk_notebook_get_nth_page (notebook, page_num);
		if (TEPL_IS_TAB (page_widget))
		{
			tabs = g_list_prepend (tabs, TEPL_TAB (page_widget));
		}
	}

	return tabs;
}

static TeplTab *
tepl_notebook_get_active_tab (TeplTabGroup *tab_group)
{
	GtkNotebook *notebook = GTK_NOTEBOOK (tab_group);
	gint cur_page_num;
	GtkWidget *cur_page_widget;

	cur_page_num = gtk_notebook_get_current_page (notebook);
	if (cur_page_num == -1)
	{
		return NULL;
	}

	cur_page_widget = gtk_notebook_get_nth_page (notebook, cur_page_num);
	return TEPL_IS_TAB (cur_page_widget) ? TEPL_TAB (cur_page_widget) : NULL;
}

static void
tepl_tab_group_interface_init (gpointer g_iface,
			       gpointer iface_data)
{
	TeplTabGroupInterface *interface = g_iface;

	interface->get_tabs = tepl_notebook_get_tabs;
	interface->get_active_tab = tepl_notebook_get_active_tab;
}

static void
tepl_notebook_init (TeplNotebook *notebook)
{
	notebook->priv = tepl_notebook_get_instance_private (notebook);
}

/**
 * tepl_notebook_new:
 *
 * Returns: (transfer floating): a new #TeplNotebook.
 * Since: 3.0
 */
GtkWidget *
tepl_notebook_new (void)
{
	return g_object_new (TEPL_TYPE_NOTEBOOK, NULL);
}
