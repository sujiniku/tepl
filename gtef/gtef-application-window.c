/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2017 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * Gtef is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * Gtef is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "gtef-application-window.h"
#include "gtef-action-info.h"
#include "gtef-action-info-central-store.h"
#include "gtef-menu-shell.h"

/**
 * SECTION:application-window
 * @Short_description: An extension of GtkApplicationWindow
 * @Title: GtefApplicationWindow
 *
 * #GtefApplicationWindow extends the #GtkApplicationWindow class.
 *
 * Note that #GtefApplicationWindow extends the #GtkApplicationWindow class but
 * without subclassing it, because several libraries might want to extend
 * #GtkApplicationWindow and an application needs to be able to use all those
 * extensions at the same time.
 */

struct _GtefApplicationWindowPrivate
{
	GtkApplicationWindow *gtk_window;
	GtkStatusbar *statusbar;
};

enum
{
	PROP_0,
	PROP_APPLICATION_WINDOW,
	PROP_STATUSBAR,
	N_PROPERTIES
};

#define GTEF_APPLICATION_WINDOW_KEY "gtef-application-window-key"
#define MENU_SHELL_STATUSBAR_CONTEXT_ID_KEY "gtef-menu-shell-statusbar-context-id-key"

static GParamSpec *properties[N_PROPERTIES];

G_DEFINE_TYPE_WITH_PRIVATE (GtefApplicationWindow, gtef_application_window, G_TYPE_OBJECT)

static void
gtef_application_window_get_property (GObject    *object,
				      guint       prop_id,
				      GValue     *value,
				      GParamSpec *pspec)
{
	GtefApplicationWindow *gtef_window = GTEF_APPLICATION_WINDOW (object);

	switch (prop_id)
	{
		case PROP_APPLICATION_WINDOW:
			g_value_set_object (value, gtef_application_window_get_application_window (gtef_window));
			break;

		case PROP_STATUSBAR:
			g_value_set_object (value, gtef_application_window_get_statusbar (gtef_window));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gtef_application_window_set_property (GObject      *object,
				      guint         prop_id,
				      const GValue *value,
				      GParamSpec   *pspec)
{
	GtefApplicationWindow *gtef_window = GTEF_APPLICATION_WINDOW (object);

	switch (prop_id)
	{
		case PROP_APPLICATION_WINDOW:
			g_assert (gtef_window->priv->gtk_window == NULL);
			gtef_window->priv->gtk_window = g_value_get_object (value);
			break;

		case PROP_STATUSBAR:
			gtef_application_window_set_statusbar (gtef_window, g_value_get_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gtef_application_window_dispose (GObject *object)
{
	GtefApplicationWindow *gtef_window = GTEF_APPLICATION_WINDOW (object);

	gtef_window->priv->gtk_window = NULL;

	G_OBJECT_CLASS (gtef_application_window_parent_class)->dispose (object);
}

static void
gtef_application_window_class_init (GtefApplicationWindowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gtef_application_window_get_property;
	object_class->set_property = gtef_application_window_set_property;
	object_class->dispose = gtef_application_window_dispose;

	/**
	 * GtefApplicationWindow:application-window:
	 *
	 * The #GtkApplicationWindow.
	 *
	 * Since: 2.0
	 */
	properties[PROP_APPLICATION_WINDOW] =
		g_param_spec_object ("application-window",
				     "GtkApplicationWindow",
				     "",
				     GTK_TYPE_APPLICATION_WINDOW,
				     G_PARAM_READWRITE |
				     G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS);

	/**
	 * GtefApplicationWindow:statusbar:
	 *
	 * The #GtkStatusbar. %NULL by default.
	 *
	 * Since: 2.0
	 */
	properties[PROP_STATUSBAR] =
		g_param_spec_object ("statusbar",
				     "GtkStatusbar",
				     "",
				     GTK_TYPE_STATUSBAR,
				     G_PARAM_READWRITE |
				     G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
gtef_application_window_init (GtefApplicationWindow *gtef_window)
{
	gtef_window->priv = gtef_application_window_get_instance_private (gtef_window);
}

/**
 * gtef_application_window_get_from_gtk_application_window:
 * @gtk_window: a #GtkApplicationWindow.
 *
 * Returns the #GtefApplicationWindow of @gtk_window. The returned object is
 * guaranteed to be the same for the lifetime of @gtk_window.
 *
 * Returns: (transfer none): the #GtefApplicationWindow of @gtk_window.
 * Since: 2.0
 */
GtefApplicationWindow *
gtef_application_window_get_from_gtk_application_window (GtkApplicationWindow *gtk_window)
{
	GtefApplicationWindow *gtef_window;

	g_return_val_if_fail (GTK_IS_APPLICATION_WINDOW (gtk_window), NULL);

	gtef_window = g_object_get_data (G_OBJECT (gtk_window), GTEF_APPLICATION_WINDOW_KEY);

	if (gtef_window == NULL)
	{
		gtef_window = g_object_new (GTEF_TYPE_APPLICATION_WINDOW,
					    "application-window", gtk_window,
					    NULL);

		g_object_set_data_full (G_OBJECT (gtk_window),
					GTEF_APPLICATION_WINDOW_KEY,
					gtef_window,
					g_object_unref);
	}

	g_return_val_if_fail (GTEF_IS_APPLICATION_WINDOW (gtef_window), NULL);
	return gtef_window;
}

/**
 * gtef_application_window_get_application_window:
 * @gtef_window: a #GtefApplicationWindow.
 *
 * Returns: (transfer none): the #GtkApplicationWindow of @gtef_window.
 * Since: 2.0
 */
GtkApplicationWindow *
gtef_application_window_get_application_window (GtefApplicationWindow *gtef_window)
{
	g_return_val_if_fail (GTEF_IS_APPLICATION_WINDOW (gtef_window), NULL);

	return gtef_window->priv->gtk_window;
}

/**
 * gtef_application_window_get_statusbar:
 * @gtef_window: a #GtefApplicationWindow.
 *
 * Returns: (transfer none) (nullable): the #GtefApplicationWindow:statusbar.
 * Since: 2.0
 */
GtkStatusbar *
gtef_application_window_get_statusbar (GtefApplicationWindow *gtef_window)
{
	g_return_val_if_fail (GTEF_IS_APPLICATION_WINDOW (gtef_window), NULL);

	return gtef_window->priv->statusbar;
}

/**
 * gtef_application_window_set_statusbar:
 * @gtef_window: a #GtefApplicationWindow.
 * @statusbar: (nullable): a #GtkStatusbar, or %NULL.
 *
 * Sets the #GtefApplicationWindow:statusbar property.
 *
 * Since: 2.0
 */
void
gtef_application_window_set_statusbar (GtefApplicationWindow *gtef_window,
				       GtkStatusbar          *statusbar)
{
	g_return_if_fail (GTEF_IS_APPLICATION_WINDOW (gtef_window));
	g_return_if_fail (statusbar == NULL || GTK_IS_STATUSBAR (statusbar));

	if (gtef_window->priv->statusbar == statusbar)
	{
		return;
	}

	if (statusbar != NULL)
	{
		g_object_ref_sink (statusbar);
	}

	if (gtef_window->priv->statusbar != NULL)
	{
		g_object_unref (gtef_window->priv->statusbar);
	}

	gtef_window->priv->statusbar = statusbar;
	g_object_notify_by_pspec (G_OBJECT (gtef_window), properties[PROP_STATUSBAR]);
}

static const gchar *
get_menu_item_tooltip (GtkMenuItem *menu_item)
{
	const gchar *action_name;
	GtefActionInfoCentralStore *central_store;
	const GtefActionInfo *action_info;

	action_name = gtk_actionable_get_action_name (GTK_ACTIONABLE (menu_item));
	if (action_name == NULL)
	{
		return NULL;
	}

	central_store = gtef_action_info_central_store_get_instance ();
	action_info = gtef_action_info_central_store_lookup (central_store, action_name);
	if (action_info == NULL)
	{
		return NULL;
	}

	return gtef_action_info_get_tooltip (action_info);
}

/* Returns: %TRUE if a context ID exists and has been set to @context_id. */
static gboolean
get_statusbar_context_id_for_menu_shell (GtefApplicationWindow *gtef_window,
					 GtefMenuShell         *gtef_menu_shell,
					 gboolean               create,
					 guint                 *context_id)
{
	gpointer data;

	g_assert (gtef_window->priv->statusbar != NULL);
	g_assert (context_id != NULL);

	data = g_object_get_data (G_OBJECT (gtef_menu_shell), MENU_SHELL_STATUSBAR_CONTEXT_ID_KEY);

	if (data == NULL && !create)
	{
		return FALSE;
	}

	if (data == NULL)
	{
		*context_id = gtk_statusbar_get_context_id (gtef_window->priv->statusbar,
							    "Show long description of menu items.");

		g_object_set_data (G_OBJECT (gtef_menu_shell),
				   MENU_SHELL_STATUSBAR_CONTEXT_ID_KEY,
				   GUINT_TO_POINTER (*context_id));
	}
	else
	{
		*context_id = GPOINTER_TO_UINT (data);
	}

	return TRUE;
}

static void
menu_item_selected_cb (GtefMenuShell *gtef_menu_shell,
		       GtkMenuItem   *menu_item,
		       gpointer       user_data)
{
	GtefApplicationWindow *gtef_window = GTEF_APPLICATION_WINDOW (user_data);
	const gchar *tooltip;
	guint context_id;

	if (gtef_window->priv->statusbar == NULL)
	{
		return;
	}

	tooltip = get_menu_item_tooltip (menu_item);
	if (tooltip == NULL)
	{
		return;
	}

	get_statusbar_context_id_for_menu_shell (gtef_window,
						 gtef_menu_shell,
						 TRUE,
						 &context_id);

	gtk_statusbar_push (gtef_window->priv->statusbar,
			    context_id,
			    tooltip);
}

static void
menu_item_deselected_cb (GtefMenuShell *gtef_menu_shell,
			 GtkMenuItem   *menu_item,
			 gpointer       user_data)
{
	GtefApplicationWindow *gtef_window = GTEF_APPLICATION_WINDOW (user_data);
	const gchar *tooltip;
	guint context_id;

	if (gtef_window->priv->statusbar == NULL)
	{
		return;
	}

	tooltip = get_menu_item_tooltip (menu_item);
	if (tooltip == NULL)
	{
		return;
	}

	if (get_statusbar_context_id_for_menu_shell (gtef_window,
						     gtef_menu_shell,
						     FALSE,
						     &context_id))
	{
		gtk_statusbar_pop (gtef_window->priv->statusbar, context_id);
	}
}

static void
statusbar_notify_cb (GtefApplicationWindow *gtef_window,
		     GParamSpec            *pspec,
		     gpointer               user_data)
{
	GtefMenuShell *gtef_menu_shell = GTEF_MENU_SHELL (user_data);

	g_object_set_data (G_OBJECT (gtef_menu_shell),
			   MENU_SHELL_STATUSBAR_CONTEXT_ID_KEY,
			   NULL);
}

/**
 * gtef_application_window_connect_menu_to_statusbar:
 * @gtef_window: a #GtefApplicationWindow.
 * @gtef_menu_shell: a #GtefMenuShell.
 *
 * Connect to the #GtefMenuShell::menu-item-selected and
 * #GtefMenuShell::menu-item-deselected signals of @gtef_menu_shell to push/pop
 * the tooltip (i.e. long description) of #GtkMenuItem's to the
 * #GtefApplicationWindow:statusbar.
 *
 * The tooltip is retrieved from the #GtefActionInfoCentralStore. The
 * @action_name is get from the #GtkMenuItem, by calling
 * gtk_actionable_get_action_name(). This will work fine if you've created the
 * menu items with the functions available in #GtefActionInfoStore.
 *
 * Since: 2.0
 */
void
gtef_application_window_connect_menu_to_statusbar (GtefApplicationWindow *gtef_window,
						   GtefMenuShell         *gtef_menu_shell)
{
	g_return_if_fail (GTEF_IS_APPLICATION_WINDOW (gtef_window));
	g_return_if_fail (GTEF_IS_MENU_SHELL (gtef_menu_shell));

	g_signal_connect_object (gtef_menu_shell,
				 "menu-item-selected",
				 G_CALLBACK (menu_item_selected_cb),
				 gtef_window,
				 0);

	g_signal_connect_object (gtef_menu_shell,
				 "menu-item-deselected",
				 G_CALLBACK (menu_item_deselected_cb),
				 gtef_window,
				 0);

	g_signal_connect_object (gtef_window,
				 "notify::statusbar",
				 G_CALLBACK (statusbar_notify_cb),
				 gtef_menu_shell,
				 0);
}

/* ex:set ts=8 noet: */
