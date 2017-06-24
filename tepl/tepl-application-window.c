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

#include "config.h"
#include "tepl-application-window.h"
#include <glib/gi18n-lib.h>
#include "tepl-application.h"
#include "tepl-action-info.h"
#include "tepl-action-info-central-store.h"
#include "tepl-menu-item.h"
#include "tepl-menu-shell.h"
#include "tepl-tab-group.h"
#include "tepl-utils.h"

/**
 * SECTION:application-window
 * @Short_description: An extension of GtkApplicationWindow
 * @Title: TeplApplicationWindow
 *
 * #TeplApplicationWindow extends the #GtkApplicationWindow class.
 *
 * Note that #TeplApplicationWindow extends the #GtkApplicationWindow class but
 * without subclassing it, because several libraries might want to extend
 * #GtkApplicationWindow and an application needs to be able to use all those
 * extensions at the same time.
 */

struct _TeplApplicationWindowPrivate
{
	GtkApplicationWindow *gtk_window;
	TeplTabGroup *tab_group;
	GtkStatusbar *statusbar;
};

enum
{
	PROP_0,
	PROP_APPLICATION_WINDOW,
	PROP_STATUSBAR,
	N_PROPERTIES
};

#define TEPL_APPLICATION_WINDOW_KEY "tepl-application-window-key"
#define MENU_SHELL_STATUSBAR_CONTEXT_ID_KEY "tepl-menu-shell-statusbar-context-id-key"
#define MENU_SHELL_FOR_RECENT_CHOOSER_KEY "tepl-menu-shell-for-recent-chooser-key"

static GParamSpec *properties[N_PROPERTIES];

static void tepl_tab_group_interface_init (gpointer g_iface,
					   gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE (TeplApplicationWindow,
			 tepl_application_window,
			 G_TYPE_OBJECT,
			 G_ADD_PRIVATE (TeplApplicationWindow)
			 G_IMPLEMENT_INTERFACE (TEPL_TYPE_TAB_GROUP,
						tepl_tab_group_interface_init))

static void
tepl_application_window_get_property (GObject    *object,
				      guint       prop_id,
				      GValue     *value,
				      GParamSpec *pspec)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (object);

	switch (prop_id)
	{
		case PROP_APPLICATION_WINDOW:
			g_value_set_object (value, tepl_application_window_get_application_window (tepl_window));
			break;

		case PROP_STATUSBAR:
			g_value_set_object (value, tepl_application_window_get_statusbar (tepl_window));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_application_window_set_property (GObject      *object,
				      guint         prop_id,
				      const GValue *value,
				      GParamSpec   *pspec)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (object);

	switch (prop_id)
	{
		case PROP_APPLICATION_WINDOW:
			g_assert (tepl_window->priv->gtk_window == NULL);
			tepl_window->priv->gtk_window = g_value_get_object (value);
			break;

		case PROP_STATUSBAR:
			tepl_application_window_set_statusbar (tepl_window, g_value_get_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_application_window_dispose (GObject *object)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (object);

	tepl_window->priv->gtk_window = NULL;
	g_clear_object (&tepl_window->priv->tab_group);
	g_clear_object (&tepl_window->priv->statusbar);

	G_OBJECT_CLASS (tepl_application_window_parent_class)->dispose (object);
}

static void
tepl_application_window_class_init (TeplApplicationWindowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = tepl_application_window_get_property;
	object_class->set_property = tepl_application_window_set_property;
	object_class->dispose = tepl_application_window_dispose;

	/**
	 * TeplApplicationWindow:application-window:
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
	 * TeplApplicationWindow:statusbar:
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

static GList *
tepl_application_window_get_tabs (TeplTabGroup *tab_group)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (tab_group);

	if (tepl_window->priv->tab_group == NULL)
	{
		return NULL;
	}

	return tepl_tab_group_get_tabs (tepl_window->priv->tab_group);
}

static TeplTab *
tepl_application_window_get_active_tab (TeplTabGroup *tab_group)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (tab_group);

	if (tepl_window->priv->tab_group == NULL)
	{
		return NULL;
	}

	return tepl_tab_group_get_active_tab (tepl_window->priv->tab_group);
}

static void
tepl_tab_group_interface_init (gpointer g_iface,
			       gpointer iface_data)
{
	TeplTabGroupInterface *interface = g_iface;

	interface->get_tabs = tepl_application_window_get_tabs;
	interface->get_active_tab = tepl_application_window_get_active_tab;
}

static void
tepl_application_window_init (TeplApplicationWindow *tepl_window)
{
	tepl_window->priv = tepl_application_window_get_instance_private (tepl_window);
}

/**
 * tepl_application_window_get_from_gtk_application_window:
 * @gtk_window: a #GtkApplicationWindow.
 *
 * Returns the #TeplApplicationWindow of @gtk_window. The returned object is
 * guaranteed to be the same for the lifetime of @gtk_window.
 *
 * Returns: (transfer none): the #TeplApplicationWindow of @gtk_window.
 * Since: 2.0
 */
TeplApplicationWindow *
tepl_application_window_get_from_gtk_application_window (GtkApplicationWindow *gtk_window)
{
	TeplApplicationWindow *tepl_window;

	g_return_val_if_fail (GTK_IS_APPLICATION_WINDOW (gtk_window), NULL);

	tepl_window = g_object_get_data (G_OBJECT (gtk_window), TEPL_APPLICATION_WINDOW_KEY);

	if (tepl_window == NULL)
	{
		tepl_window = g_object_new (TEPL_TYPE_APPLICATION_WINDOW,
					    "application-window", gtk_window,
					    NULL);

		g_object_set_data_full (G_OBJECT (gtk_window),
					TEPL_APPLICATION_WINDOW_KEY,
					tepl_window,
					g_object_unref);
	}

	g_return_val_if_fail (TEPL_IS_APPLICATION_WINDOW (tepl_window), NULL);
	return tepl_window;
}

/**
 * tepl_application_window_get_application_window:
 * @tepl_window: a #TeplApplicationWindow.
 *
 * Returns: (transfer none): the #GtkApplicationWindow of @tepl_window.
 * Since: 2.0
 */
GtkApplicationWindow *
tepl_application_window_get_application_window (TeplApplicationWindow *tepl_window)
{
	g_return_val_if_fail (TEPL_IS_APPLICATION_WINDOW (tepl_window), NULL);

	return tepl_window->priv->gtk_window;
}

/**
 * tepl_application_window_set_tab_group:
 * @tepl_window: a #TeplApplicationWindow.
 * @tab_group: a #TeplTabGroup.
 *
 * Sets the #TeplTabGroup of @tepl_window. This function can be called only once,
 * it is not possible to change the #TeplTabGroup (this restriction may be lifted
 * in the future if there is a compelling use-case).
 *
 * #TeplApplicationWindow implements the #TeplTabGroup interface by delegating
 * the requests to @tab_group.
 *
 * Since: 3.0
 */
void
tepl_application_window_set_tab_group (TeplApplicationWindow *tepl_window,
				       TeplTabGroup          *tab_group)
{
	g_return_if_fail (TEPL_IS_APPLICATION_WINDOW (tepl_window));
	g_return_if_fail (TEPL_IS_TAB_GROUP (tab_group));

	if (tepl_window->priv->tab_group != NULL)
	{
		g_warning ("%s(): the TeplTabGroup has already been set, it can be set only once.",
			   G_STRFUNC);
		return;
	}

	tepl_window->priv->tab_group = g_object_ref_sink (tab_group);
}

/**
 * tepl_application_window_get_statusbar:
 * @tepl_window: a #TeplApplicationWindow.
 *
 * Returns: (transfer none) (nullable): the #TeplApplicationWindow:statusbar.
 * Since: 2.0
 */
GtkStatusbar *
tepl_application_window_get_statusbar (TeplApplicationWindow *tepl_window)
{
	g_return_val_if_fail (TEPL_IS_APPLICATION_WINDOW (tepl_window), NULL);

	return tepl_window->priv->statusbar;
}

/**
 * tepl_application_window_set_statusbar:
 * @tepl_window: a #TeplApplicationWindow.
 * @statusbar: (nullable): a #GtkStatusbar, or %NULL.
 *
 * Sets the #TeplApplicationWindow:statusbar property.
 *
 * Since: 2.0
 */
void
tepl_application_window_set_statusbar (TeplApplicationWindow *tepl_window,
				       GtkStatusbar          *statusbar)
{
	g_return_if_fail (TEPL_IS_APPLICATION_WINDOW (tepl_window));
	g_return_if_fail (statusbar == NULL || GTK_IS_STATUSBAR (statusbar));

	if (tepl_window->priv->statusbar == statusbar)
	{
		return;
	}

	if (statusbar != NULL)
	{
		g_object_ref_sink (statusbar);
	}

	if (tepl_window->priv->statusbar != NULL)
	{
		g_object_unref (tepl_window->priv->statusbar);
	}

	tepl_window->priv->statusbar = statusbar;
	g_object_notify_by_pspec (G_OBJECT (tepl_window), properties[PROP_STATUSBAR]);
}

/* Returns: %TRUE if a context ID exists and has been set to @context_id. */
static gboolean
get_statusbar_context_id_for_menu_shell (TeplApplicationWindow *tepl_window,
					 TeplMenuShell         *tepl_menu_shell,
					 gboolean               create,
					 guint                 *context_id)
{
	gpointer data;

	g_assert (tepl_window->priv->statusbar != NULL);
	g_assert (context_id != NULL);

	data = g_object_get_data (G_OBJECT (tepl_menu_shell), MENU_SHELL_STATUSBAR_CONTEXT_ID_KEY);

	if (data == NULL && !create)
	{
		return FALSE;
	}

	if (data == NULL)
	{
		*context_id = gtk_statusbar_get_context_id (tepl_window->priv->statusbar,
							    "Show long description of menu items.");

		g_object_set_data (G_OBJECT (tepl_menu_shell),
				   MENU_SHELL_STATUSBAR_CONTEXT_ID_KEY,
				   GUINT_TO_POINTER (*context_id));
	}
	else
	{
		*context_id = GPOINTER_TO_UINT (data);
	}

	return TRUE;
}

/* Free the return value with g_free(). */
static gchar *
get_menu_item_long_description (TeplMenuShell *tepl_menu_shell,
				GtkMenuItem   *menu_item)
{
	const gchar *long_description;
	gpointer data;
	gboolean is_for_recent_chooser;

	long_description = tepl_menu_item_get_long_description (menu_item);
	if (long_description != NULL)
	{
		return g_strdup (long_description);
	}

	data = g_object_get_data (G_OBJECT (tepl_menu_shell), MENU_SHELL_FOR_RECENT_CHOOSER_KEY);
	is_for_recent_chooser = data != NULL ? GPOINTER_TO_INT (data) : FALSE;

	if (is_for_recent_chooser)
	{
		GtkMenuShell *gtk_menu_shell;
		GtkRecentChooserMenu *recent_chooser_menu;
		gchar *uri;
		GFile *file;
		gchar *parse_name;
		gchar *nicer_filename;
		gchar *ret;

		gtk_menu_shell = tepl_menu_shell_get_menu_shell (tepl_menu_shell);
		recent_chooser_menu = GTK_RECENT_CHOOSER_MENU (gtk_menu_shell);
		uri = tepl_utils_recent_chooser_menu_get_item_uri (recent_chooser_menu, menu_item);

		if (uri == NULL)
		{
			return NULL;
		}

		file = g_file_new_for_uri (uri);
		g_free (uri);

		parse_name = g_file_get_parse_name (file);
		g_object_unref (file);

		nicer_filename = _tepl_utils_replace_home_dir_with_tilde (parse_name);
		g_free (parse_name);

		/* Translators: %s is a filename. */
		ret = g_strdup_printf (_("Open “%s”"), nicer_filename);
		g_free (nicer_filename);

		return ret;
	}

	return NULL;
}

static void
menu_item_selected_cb (TeplMenuShell *tepl_menu_shell,
		       GtkMenuItem   *menu_item,
		       gpointer       user_data)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (user_data);
	gchar *long_description;
	guint context_id;

	if (tepl_window->priv->statusbar == NULL)
	{
		return;
	}

	long_description = get_menu_item_long_description (tepl_menu_shell, menu_item);
	if (long_description == NULL)
	{
		return;
	}

	get_statusbar_context_id_for_menu_shell (tepl_window,
						 tepl_menu_shell,
						 TRUE,
						 &context_id);

	gtk_statusbar_push (tepl_window->priv->statusbar,
			    context_id,
			    long_description);

	g_free (long_description);
}

static void
menu_item_deselected_cb (TeplMenuShell *tepl_menu_shell,
			 GtkMenuItem   *menu_item,
			 gpointer       user_data)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (user_data);
	const gchar *long_description;
	gpointer data;
	gboolean is_for_recent_chooser;
	guint context_id;

	if (tepl_window->priv->statusbar == NULL)
	{
		return;
	}

	long_description = tepl_menu_item_get_long_description (menu_item);

	data = g_object_get_data (G_OBJECT (tepl_menu_shell), MENU_SHELL_FOR_RECENT_CHOOSER_KEY);
	is_for_recent_chooser = data != NULL ? GPOINTER_TO_INT (data) : FALSE;

	if (long_description == NULL && !is_for_recent_chooser)
	{
		return;
	}

	if (get_statusbar_context_id_for_menu_shell (tepl_window,
						     tepl_menu_shell,
						     FALSE,
						     &context_id))
	{
		gtk_statusbar_pop (tepl_window->priv->statusbar, context_id);
	}
}

static void
statusbar_notify_cb (TeplApplicationWindow *tepl_window,
		     GParamSpec            *pspec,
		     gpointer               user_data)
{
	TeplMenuShell *tepl_menu_shell = TEPL_MENU_SHELL (user_data);

	g_object_set_data (G_OBJECT (tepl_menu_shell),
			   MENU_SHELL_STATUSBAR_CONTEXT_ID_KEY,
			   NULL);
}

/**
 * tepl_application_window_connect_menu_to_statusbar:
 * @tepl_window: a #TeplApplicationWindow.
 * @tepl_menu_shell: a #TeplMenuShell.
 *
 * Connect to the #TeplMenuShell::menu-item-selected and
 * #TeplMenuShell::menu-item-deselected signals of @tepl_menu_shell to push/pop
 * the long description of #GtkMenuItem's to the
 * #TeplApplicationWindow:statusbar.
 *
 * The long description is retrieved with tepl_menu_item_get_long_description().
 * So tepl_menu_item_set_long_description() must have been called, which is the
 * case if the #GtkMenuItem has been created with the functions available in
 * #TeplActionInfoStore.
 *
 * Since: 2.0
 */
void
tepl_application_window_connect_menu_to_statusbar (TeplApplicationWindow *tepl_window,
						   TeplMenuShell         *tepl_menu_shell)
{
	g_return_if_fail (TEPL_IS_APPLICATION_WINDOW (tepl_window));
	g_return_if_fail (TEPL_IS_MENU_SHELL (tepl_menu_shell));

	g_signal_connect_object (tepl_menu_shell,
				 "menu-item-selected",
				 G_CALLBACK (menu_item_selected_cb),
				 tepl_window,
				 0);

	g_signal_connect_object (tepl_menu_shell,
				 "menu-item-deselected",
				 G_CALLBACK (menu_item_deselected_cb),
				 tepl_window,
				 0);

	g_signal_connect_object (tepl_window,
				 "notify::statusbar",
				 G_CALLBACK (statusbar_notify_cb),
				 tepl_menu_shell,
				 0);
}

/**
 * tepl_application_window_connect_recent_chooser_menu_to_statusbar:
 * @tepl_window: a #TeplApplicationWindow.
 * @menu: a #GtkRecentChooserMenu.
 *
 * An alternative to gtk_recent_chooser_set_show_tips(). Shows the full path in
 * the #TeplApplicationWindow:statusbar when a #GtkMenuItem of @menu is
 * selected.
 *
 * The full path is retrieved with
 * tepl_utils_recent_chooser_menu_get_item_uri().
 *
 * Since: 2.0
 */
void
tepl_application_window_connect_recent_chooser_menu_to_statusbar (TeplApplicationWindow *tepl_window,
								  GtkRecentChooserMenu  *menu)
{
	TeplMenuShell *tepl_menu_shell;

	g_return_if_fail (TEPL_IS_APPLICATION_WINDOW (tepl_window));
	g_return_if_fail (GTK_IS_RECENT_CHOOSER_MENU (menu));

	tepl_menu_shell = tepl_menu_shell_get_from_gtk_menu_shell (GTK_MENU_SHELL (menu));

	g_object_set_data (G_OBJECT (tepl_menu_shell),
			   MENU_SHELL_FOR_RECENT_CHOOSER_KEY,
			   GINT_TO_POINTER (TRUE));

	tepl_application_window_connect_menu_to_statusbar (tepl_window, tepl_menu_shell);
}

static void
open_recent_file_cb (GtkRecentChooser *recent_chooser,
		     gpointer          user_data)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (user_data);
	GtkApplication *gtk_app;
	TeplApplication *tepl_app;
	gchar *uri;
	GFile *file;

	gtk_app = gtk_window_get_application (GTK_WINDOW (tepl_window->priv->gtk_window));
	tepl_app = tepl_application_get_from_gtk_application (gtk_app);

	uri = gtk_recent_chooser_get_current_uri (recent_chooser);
	file = g_file_new_for_uri (uri);

	tepl_application_open_simple (tepl_app, file);

	g_free (uri);
	g_object_unref (file);
}

/**
 * tepl_application_window_create_open_recent_menu_item:
 * @tepl_window: a #TeplApplicationWindow.
 *
 * Creates a #GtkMenuItem with a simple and generic #GtkRecentChooserMenu as
 * submenu.
 *
 * The #GtkRecentChooser is configured to show files only recently used with the
 * current application, as returned by g_get_application_name(). If recent files
 * are added to the default #GtkRecentManager with
 * gtk_recent_manager_add_item(), the files will normally show up in the
 * #GtkRecentChooserMenu.
 *
 * The #GtkRecentChooserMenu is connected to the statusbar with
 * tepl_application_window_connect_recent_chooser_menu_to_statusbar().
 *
 * When the #GtkRecentChooser::item-activated signal is emitted,
 * tepl_application_open_simple() is called, so the #GApplication must have the
 * %G_APPLICATION_HANDLES_OPEN flag set.
 *
 * Returns: (transfer floating): a new #GtkMenuItem.
 * Since: 2.0
 */
GtkWidget *
tepl_application_window_create_open_recent_menu_item (TeplApplicationWindow *tepl_window)
{
	GtkMenuItem *menu_item;
	gchar *long_description;
	GtkRecentChooserMenu *recent_chooser_menu;
	GtkRecentChooser *recent_chooser;
	GtkRecentFilter *filter;

	g_return_val_if_fail (TEPL_IS_APPLICATION_WINDOW (tepl_window), NULL);

	menu_item = GTK_MENU_ITEM (gtk_menu_item_new_with_mnemonic (_("Open _Recent")));

	/* Translators: %s is the application name. */
	long_description = g_strdup_printf (_("Open a file recently used with %s"),
					    g_get_application_name ());
	tepl_menu_item_set_long_description (menu_item, long_description);
	g_free (long_description);

	recent_chooser_menu = GTK_RECENT_CHOOSER_MENU (gtk_recent_chooser_menu_new ());
	gtk_menu_item_set_submenu (menu_item, GTK_WIDGET (recent_chooser_menu));

	recent_chooser = GTK_RECENT_CHOOSER (recent_chooser_menu);
	gtk_recent_chooser_set_local_only (recent_chooser, FALSE);
	gtk_recent_chooser_set_sort_type (recent_chooser, GTK_RECENT_SORT_MRU);

	filter = gtk_recent_filter_new ();
	gtk_recent_filter_add_application (filter, g_get_application_name ());
	gtk_recent_chooser_set_filter (recent_chooser, filter);

	tepl_application_window_connect_recent_chooser_menu_to_statusbar (tepl_window, recent_chooser_menu);

	g_signal_connect_object (recent_chooser,
				 "item-activated",
				 G_CALLBACK (open_recent_file_cb),
				 tepl_window,
				 0);

	return GTK_WIDGET (menu_item);
}

/* ex:set ts=8 noet: */
