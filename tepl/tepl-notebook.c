/* SPDX-FileCopyrightText: 2017 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "tepl-notebook.h"
#include "tepl-abstract-factory.h"
#include "tepl-tab-group.h"
#include "tepl-tab.h"
#include "tepl-signal-group.h"

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
	TeplSignalGroup *view_signal_group;

	/* Not used for tepl_tab_group_get_active_tab(), used to avoid sending
	 * unnecessary notify signals.
	 * Unowned.
	 */
	TeplTab *active_tab;
};

enum
{
	PROP_0,
	PROP_ACTIVE_TAB,
	PROP_ACTIVE_VIEW,
	PROP_ACTIVE_BUFFER,
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

		case PROP_ACTIVE_VIEW:
			g_value_set_object (value, tepl_tab_group_get_active_view (tab_group));
			break;

		case PROP_ACTIVE_BUFFER:
			g_value_set_object (value, tepl_tab_group_get_active_buffer (tab_group));
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
	TeplTabGroup *tab_group = TEPL_TAB_GROUP (object);

	switch (prop_id)
	{
		case PROP_ACTIVE_TAB:
			tepl_tab_group_set_active_tab (tab_group, g_value_get_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_notebook_dispose (GObject *object)
{
	TeplNotebook *notebook = TEPL_NOTEBOOK (object);

	tepl_signal_group_clear (&notebook->priv->view_signal_group);

	G_OBJECT_CLASS (tepl_notebook_parent_class)->dispose (object);
}

static void
buffer_notify_cb (GtkTextView  *view,
		  GParamSpec   *pspec,
		  TeplNotebook *notebook)
{
	g_object_notify (G_OBJECT (notebook), "active-buffer");
}

static void
check_active_tab_changed (TeplNotebook *notebook)
{
	TeplTab *active_tab;
	TeplView *active_view;

	active_tab = tepl_tab_group_get_active_tab (TEPL_TAB_GROUP (notebook));
	if (notebook->priv->active_tab == active_tab)
	{
		return;
	}

	notebook->priv->active_tab = active_tab;

	tepl_signal_group_clear (&notebook->priv->view_signal_group);

	active_view = tepl_tab_group_get_active_view (TEPL_TAB_GROUP (notebook));

	if (active_view != NULL)
	{
		notebook->priv->view_signal_group = tepl_signal_group_new (G_OBJECT (active_view));

		tepl_signal_group_add (notebook->priv->view_signal_group,
				       g_signal_connect (active_view,
							 "notify::buffer",
							 G_CALLBACK (buffer_notify_cb),
							 notebook));
	}

	g_object_notify (G_OBJECT (notebook), "active-tab");
	g_object_notify (G_OBJECT (notebook), "active-view");
	g_object_notify (G_OBJECT (notebook), "active-buffer");
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

	check_active_tab_changed (TEPL_NOTEBOOK (notebook));
}

static void
tepl_notebook_page_removed (GtkNotebook *notebook,
			    GtkWidget   *child,
			    guint        page_num)
{
	if (GTK_NOTEBOOK_CLASS (tepl_notebook_parent_class)->page_removed != NULL)
	{
		GTK_NOTEBOOK_CLASS (tepl_notebook_parent_class)->page_removed (notebook,
									       child,
									       page_num);
	}

	check_active_tab_changed (TEPL_NOTEBOOK (notebook));
}

static void
tepl_notebook_class_init (TeplNotebookClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkNotebookClass *notebook_class = GTK_NOTEBOOK_CLASS (klass);

	object_class->get_property = tepl_notebook_get_property;
	object_class->set_property = tepl_notebook_set_property;
	object_class->dispose = tepl_notebook_dispose;

	notebook_class->switch_page = tepl_notebook_switch_page;
	notebook_class->page_removed = tepl_notebook_page_removed;

	g_object_class_override_property (object_class, PROP_ACTIVE_TAB, "active-tab");
	g_object_class_override_property (object_class, PROP_ACTIVE_VIEW, "active-view");
	g_object_class_override_property (object_class, PROP_ACTIVE_BUFFER, "active-buffer");
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
tepl_notebook_set_active_tab (TeplTabGroup *tab_group,
			      TeplTab      *tab)
{
	GtkNotebook *notebook = GTK_NOTEBOOK (tab_group);
	gint page_num;

	page_num = gtk_notebook_page_num (notebook, GTK_WIDGET (tab));
	g_return_if_fail (page_num != -1);

	if (!gtk_widget_get_visible (GTK_WIDGET (tab)))
	{
		g_warning ("Calling gtk_notebook_set_current_page() on an "
			   "invisible TeplTab. This won't work, make the "
			   "TeplTab visible first.");
	}

	gtk_notebook_set_current_page (notebook, page_num);
}

static void
tepl_notebook_append_tab_vfunc (TeplTabGroup *tab_group,
				TeplTab      *tab)
{
	GtkNotebook *notebook = GTK_NOTEBOOK (tab_group);
	TeplAbstractFactory *factory;
	GtkWidget *tab_label;

	factory = tepl_abstract_factory_get_singleton ();
	tab_label = tepl_abstract_factory_create_tab_label (factory, tab);

	gtk_notebook_append_page (notebook, GTK_WIDGET (tab), tab_label);
}

static void
tepl_tab_group_interface_init (gpointer g_iface,
			       gpointer iface_data)
{
	TeplTabGroupInterface *interface = g_iface;

	interface->get_tabs = tepl_notebook_get_tabs;
	interface->get_active_tab = tepl_notebook_get_active_tab;
	interface->set_active_tab = tepl_notebook_set_active_tab;
	interface->append_tab_vfunc = tepl_notebook_append_tab_vfunc;
}

static void
tepl_notebook_init (TeplNotebook *notebook)
{
	notebook->priv = tepl_notebook_get_instance_private (notebook);

	/* The statusbar must always be at the bottom of the window (if there is
	 * a statusbar). More generally, the notebook is the main part of the
	 * window, so it needs to be expanded, to push other widgets on the
	 * sides, even if the notebook is empty.
	 */
	gtk_widget_set_hexpand (GTK_WIDGET (notebook), TRUE);
	gtk_widget_set_vexpand (GTK_WIDGET (notebook), TRUE);

	gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook), TRUE);
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
