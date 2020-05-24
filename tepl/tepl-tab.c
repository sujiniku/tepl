/* SPDX-FileCopyrightText: 2016-2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "config.h"
#include "tepl-tab.h"
#include <glib/gi18n-lib.h>
#include "tepl-close-confirm-dialog-single.h"
#include "tepl-info-bar.h"
#include "tepl-tab-group.h"

/**
 * SECTION:tab
 * @Short_description: Contains a TeplView and GtkInfoBars
 * @Title: TeplTab
 * @See_also: #TeplInfoBar
 *
 * #TeplTab is meant to be the content of one tab in the text editor (if the
 * text editor has a Tabbed Document Interface). It is a #GtkGrid container that
 * contains the #TeplView and can contain one or several #GtkInfoBar's. Since it
 * is a #GtkGrid, an application can of course add any other widget to it.
 *
 * To create a new #GtkInfoBar, it is recommended to use #TeplInfoBar (but
 * #TeplTab doesn't enforce it).
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
 * # TeplTabGroup implementation
 *
 * #TeplTab implements the #TeplTabGroup interface, for a group of only one tab.
 * It is useful for text editors that open each file in a separate window, or
 * for applications that don't require to open more than one file. But the
 * tepl_tab_group_append_tab() operation is not supported, so some higher-level
 * features of Tepl don't work with #TeplTab as the #TeplTabGroup of the window.
 * This will maybe be improved in the future by creating automatically a new
 * window.
 */

struct _TeplTabPrivate
{
	GtkScrolledWindow *scrolled_window;
	TeplView *view;
	TeplGotoLineBar *goto_line_bar;
};

enum
{
	PROP_0,
	PROP_VIEW,
	PROP_ACTIVE_TAB,
	PROP_ACTIVE_VIEW,
	PROP_ACTIVE_BUFFER,
};

enum
{
	SIGNAL_CLOSE_REQUEST,
	N_SIGNALS
};

static guint signals[N_SIGNALS];

static void tepl_tab_group_interface_init (gpointer g_iface,
					   gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE (TeplTab,
			 tepl_tab,
			 GTK_TYPE_GRID,
			 G_ADD_PRIVATE (TeplTab)
			 G_IMPLEMENT_INTERFACE (TEPL_TYPE_TAB_GROUP,
						tepl_tab_group_interface_init))

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
	if (tab->priv->scrolled_window != NULL)
	{
		g_warning ("The TeplTab::pack_view virtual function can be called only once.");
		return;
	}

	tab->priv->scrolled_window = create_scrolled_window ();
	g_object_ref_sink (tab->priv->scrolled_window);

	gtk_container_add (GTK_CONTAINER (tab->priv->scrolled_window),
			   GTK_WIDGET (view));

	gtk_container_add (GTK_CONTAINER (tab),
			   GTK_WIDGET (tab->priv->scrolled_window));
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
tepl_tab_pack_goto_line_bar_default (TeplTab         *tab,
				     TeplGotoLineBar *goto_line_bar)
{
	gtk_container_add (GTK_CONTAINER (tab), GTK_WIDGET (goto_line_bar));
}

static void
close_confirm_dialog_single_cb (GObject      *source_object,
				GAsyncResult *result,
				gpointer      user_data)
{
	TeplTab *tab = TEPL_TAB (source_object);

	if (_tepl_close_confirm_dialog_single_finish (tab, result))
	{
		gtk_widget_destroy (GTK_WIDGET (tab));
	}
}

static void
tepl_tab_close_request_default (TeplTab *tab)
{
	_tepl_close_confirm_dialog_single_async (tab, close_confirm_dialog_single_cb, NULL);
}

static void
buffer_notify_cb (GtkTextView *view,
		  GParamSpec  *pspec,
		  TeplTab     *tab)
{
	g_object_notify (G_OBJECT (tab), "active-buffer");
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

	g_signal_connect_object (view,
				 "notify::buffer",
				 G_CALLBACK (buffer_notify_cb),
				 tab,
				 0);

	g_object_notify (G_OBJECT (tab), "view");
}

static void
tepl_tab_get_property (GObject    *object,
                       guint       prop_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
	TeplTab *tab = TEPL_TAB (object);
	TeplTabGroup *tab_group = TEPL_TAB_GROUP (object);

	switch (prop_id)
	{
		case PROP_VIEW:
			g_value_set_object (value, tepl_tab_get_view (tab));
			break;

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
tepl_tab_set_property (GObject      *object,
                       guint         prop_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
	TeplTab *tab = TEPL_TAB (object);
	TeplTabGroup *tab_group = TEPL_TAB_GROUP (object);

	switch (prop_id)
	{
		case PROP_VIEW:
			set_view (tab, g_value_get_object (value));
			break;

		case PROP_ACTIVE_TAB:
			tepl_tab_group_set_active_tab (tab_group, g_value_get_object (value));
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

	g_clear_object (&tab->priv->scrolled_window);
	g_clear_object (&tab->priv->view);
	g_clear_object (&tab->priv->goto_line_bar);

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
	klass->pack_goto_line_bar = tepl_tab_pack_goto_line_bar_default;
	klass->close_request = tepl_tab_close_request_default;

	/**
	 * TeplTab:view:
	 *
	 * The #TeplView contained in the tab. When this property is set, the
	 * ::pack_view virtual function is called.
	 *
	 * Since: 3.0
	 */
	g_object_class_install_property (object_class,
					 PROP_VIEW,
					 g_param_spec_object ("view",
							      "View",
							      "",
							      TEPL_TYPE_VIEW,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY |
							      G_PARAM_STATIC_STRINGS));

	g_object_class_override_property (object_class, PROP_ACTIVE_TAB, "active-tab");
	g_object_class_override_property (object_class, PROP_ACTIVE_VIEW, "active-view");
	g_object_class_override_property (object_class, PROP_ACTIVE_BUFFER, "active-buffer");

	/**
	 * TeplTab::close-request:
	 * @tab: the #TeplTab emitting the signal.
	 *
	 * The ::close-request signal is emitted when there is a request to
	 * close the #TeplTab, for example if the user clicks on a close button.
	 *
	 * The default object method handler does the following:
	 * - If the buffer is not modified (according to
	 *   gtk_text_buffer_get_modified()), close the tab.
	 * - Else, show a message dialog to propose to save the file before
	 *   closing.
	 *
	 * To override the default object method handler, either override the
	 * virtual function in a #TeplTab subclass or connect to the signal and
	 * call g_signal_stop_emission_by_name().
	 *
	 * Since: 3.0
	 */
	signals[SIGNAL_CLOSE_REQUEST] =
		g_signal_new ("close-request",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (TeplTabClass, close_request),
			      NULL, NULL, NULL,
			      G_TYPE_NONE, 0);
}

static GList *
tepl_tab_get_tabs (TeplTabGroup *tab_group)
{
	return g_list_append (NULL, TEPL_TAB (tab_group));
}

static TeplTab *
tepl_tab_get_active_tab (TeplTabGroup *tab_group)
{
	return TEPL_TAB (tab_group);
}

static void
tepl_tab_group_interface_init (gpointer g_iface,
			       gpointer iface_data)
{
	TeplTabGroupInterface *interface = g_iface;

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

	if (tab->priv->view == NULL)
	{
		return NULL;
	}

	return TEPL_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (tab->priv->view)));
}

/**
 * tepl_tab_get_goto_line_bar:
 * @tab: a #TeplTab.
 *
 * Gets the #TeplGotoLineBar widget belonging to @tab. The #TeplGotoLineBar must
 * not be destroyed by the application, the purpose of this function is to
 * show/hide the widget.
 *
 * Returns: (transfer none): the #TeplGotoLineBar widget belonging to @tab.
 * Since: 5.0
 */
TeplGotoLineBar *
tepl_tab_get_goto_line_bar (TeplTab *tab)
{
	g_return_val_if_fail (TEPL_IS_TAB (tab), NULL);

	if (tab->priv->goto_line_bar == NULL)
	{
		tab->priv->goto_line_bar = tepl_goto_line_bar_new ();
		g_object_ref_sink (tab->priv->goto_line_bar);

		/* The TeplGotoLineBar needs to be explicitly shown/hidden. */
		gtk_widget_set_no_show_all (GTK_WIDGET (tab->priv->goto_line_bar), TRUE);

		tepl_goto_line_bar_set_view (tab->priv->goto_line_bar,
					     tab->priv->view);

		TEPL_TAB_GET_CLASS (tab)->pack_goto_line_bar (tab, tab->priv->goto_line_bar);
	}

	return tab->priv->goto_line_bar;
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
