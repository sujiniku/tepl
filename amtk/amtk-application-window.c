/*
 * This file is part of Amtk - Actions, Menus and Toolbars Kit
 *
 * Copyright 2017 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * Amtk is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * Amtk is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "amtk-application-window.h"
#include <glib/gi18n-lib.h>
#include "amtk-action-info.h"
#include "amtk-action-info-central-store.h"
#include "amtk-menu-item.h"
#include "amtk-menu-shell.h"
#include "amtk-utils.h"

/**
 * SECTION:amtk-application-window
 * @Short_description: An extension of GtkApplicationWindow
 * @Title: AmtkApplicationWindow
 *
 * #AmtkApplicationWindow extends the #GtkApplicationWindow class with a
 * #AmtkApplicationWindow:statusbar property and functions to show longer
 * descriptions of #GtkMenuItem's to the #GtkStatusbar.
 *
 * Note that #AmtkApplicationWindow extends the #GtkApplicationWindow class but
 * without subclassing it, because several libraries might want to extend
 * #GtkApplicationWindow and an application needs to be able to use all those
 * extensions at the same time.
 */

/* API design:
 *
 * AmtkApplicationWindow was first implemented in Tepl, and
 * TeplApplicationWindow needs to access the GtkApplicationWindow for the
 * GActionMap. Currently AmtkApplicationWindow could be renamed to AmtkWindow
 * and be an extension of GtkWindow instead of GtkApplicationWindow, it would
 * have the advantage to have shorter function names and be a little more
 * re-usable. But I think it's not a big problem to keep AmtkApplicationWindow
 * as is, because (1) the A in Amtk is all about GActions, so normally the app
 * already uses GtkApplicationWindow, (2) it is easy to port an application to
 * use GtkApplicationWindow, most of the time it's just changing the parent
 * class when subclassing GtkWindow, (3) it is more future-proof for Amtk to
 * have access to the GtkApplicationWindow, in case we want to add some features
 * that require the GActionMap or whatever. -- swilmet
 */

struct _AmtkApplicationWindowPrivate
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

#define AMTK_APPLICATION_WINDOW_KEY "amtk-application-window-key"
#define MENU_SHELL_STATUSBAR_CONTEXT_ID_KEY "amtk-menu-shell-statusbar-context-id-key"
#define MENU_SHELL_FOR_RECENT_CHOOSER_KEY "amtk-menu-shell-for-recent-chooser-key"

static GParamSpec *properties[N_PROPERTIES];

G_DEFINE_TYPE_WITH_PRIVATE (AmtkApplicationWindow, amtk_application_window, G_TYPE_OBJECT)

static void
amtk_application_window_get_property (GObject    *object,
				      guint       prop_id,
				      GValue     *value,
				      GParamSpec *pspec)
{
	AmtkApplicationWindow *amtk_window = AMTK_APPLICATION_WINDOW (object);

	switch (prop_id)
	{
		case PROP_APPLICATION_WINDOW:
			g_value_set_object (value, amtk_application_window_get_application_window (amtk_window));
			break;

		case PROP_STATUSBAR:
			g_value_set_object (value, amtk_application_window_get_statusbar (amtk_window));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
amtk_application_window_set_property (GObject      *object,
				      guint         prop_id,
				      const GValue *value,
				      GParamSpec   *pspec)
{
	AmtkApplicationWindow *amtk_window = AMTK_APPLICATION_WINDOW (object);

	switch (prop_id)
	{
		case PROP_APPLICATION_WINDOW:
			g_assert (amtk_window->priv->gtk_window == NULL);
			amtk_window->priv->gtk_window = g_value_get_object (value);
			break;

		case PROP_STATUSBAR:
			amtk_application_window_set_statusbar (amtk_window, g_value_get_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
amtk_application_window_dispose (GObject *object)
{
	AmtkApplicationWindow *amtk_window = AMTK_APPLICATION_WINDOW (object);

	amtk_window->priv->gtk_window = NULL;
	g_clear_object (&amtk_window->priv->statusbar);

	G_OBJECT_CLASS (amtk_application_window_parent_class)->dispose (object);
}

static void
amtk_application_window_class_init (AmtkApplicationWindowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = amtk_application_window_get_property;
	object_class->set_property = amtk_application_window_set_property;
	object_class->dispose = amtk_application_window_dispose;

	/**
	 * AmtkApplicationWindow:application-window:
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
	 * AmtkApplicationWindow:statusbar:
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
amtk_application_window_init (AmtkApplicationWindow *amtk_window)
{
	amtk_window->priv = amtk_application_window_get_instance_private (amtk_window);
}

/**
 * amtk_application_window_get_from_gtk_application_window:
 * @gtk_window: a #GtkApplicationWindow.
 *
 * Returns the #AmtkApplicationWindow of @gtk_window. The returned object is
 * guaranteed to be the same for the lifetime of @gtk_window.
 *
 * Returns: (transfer none): the #AmtkApplicationWindow of @gtk_window.
 * Since: 2.0
 */
AmtkApplicationWindow *
amtk_application_window_get_from_gtk_application_window (GtkApplicationWindow *gtk_window)
{
	AmtkApplicationWindow *amtk_window;

	g_return_val_if_fail (GTK_IS_APPLICATION_WINDOW (gtk_window), NULL);

	amtk_window = g_object_get_data (G_OBJECT (gtk_window), AMTK_APPLICATION_WINDOW_KEY);

	if (amtk_window == NULL)
	{
		amtk_window = g_object_new (AMTK_TYPE_APPLICATION_WINDOW,
					    "application-window", gtk_window,
					    NULL);

		g_object_set_data_full (G_OBJECT (gtk_window),
					AMTK_APPLICATION_WINDOW_KEY,
					amtk_window,
					g_object_unref);
	}

	g_return_val_if_fail (AMTK_IS_APPLICATION_WINDOW (amtk_window), NULL);
	return amtk_window;
}

/**
 * amtk_application_window_get_application_window:
 * @amtk_window: an #AmtkApplicationWindow.
 *
 * Returns: (transfer none): the #GtkApplicationWindow of @amtk_window.
 * Since: 2.0
 */
GtkApplicationWindow *
amtk_application_window_get_application_window (AmtkApplicationWindow *amtk_window)
{
	g_return_val_if_fail (AMTK_IS_APPLICATION_WINDOW (amtk_window), NULL);

	return amtk_window->priv->gtk_window;
}

/**
 * amtk_application_window_get_statusbar:
 * @amtk_window: an #AmtkApplicationWindow.
 *
 * Returns: (transfer none) (nullable): the #AmtkApplicationWindow:statusbar.
 * Since: 2.0
 */
GtkStatusbar *
amtk_application_window_get_statusbar (AmtkApplicationWindow *amtk_window)
{
	g_return_val_if_fail (AMTK_IS_APPLICATION_WINDOW (amtk_window), NULL);

	return amtk_window->priv->statusbar;
}

/**
 * amtk_application_window_set_statusbar:
 * @amtk_window: an #AmtkApplicationWindow.
 * @statusbar: (nullable): a #GtkStatusbar, or %NULL.
 *
 * Sets the #AmtkApplicationWindow:statusbar property.
 *
 * Since: 2.0
 */
void
amtk_application_window_set_statusbar (AmtkApplicationWindow *amtk_window,
				       GtkStatusbar          *statusbar)
{
	g_return_if_fail (AMTK_IS_APPLICATION_WINDOW (amtk_window));
	g_return_if_fail (statusbar == NULL || GTK_IS_STATUSBAR (statusbar));

	if (amtk_window->priv->statusbar == statusbar)
	{
		return;
	}

	if (statusbar != NULL)
	{
		g_object_ref_sink (statusbar);
	}

	if (amtk_window->priv->statusbar != NULL)
	{
		g_object_unref (amtk_window->priv->statusbar);
	}

	amtk_window->priv->statusbar = statusbar;
	g_object_notify_by_pspec (G_OBJECT (amtk_window), properties[PROP_STATUSBAR]);
}

/* Returns: %TRUE if a context ID exists and has been set to @context_id. */
static gboolean
get_statusbar_context_id_for_menu_shell (AmtkApplicationWindow *amtk_window,
					 AmtkMenuShell         *amtk_menu_shell,
					 gboolean               create,
					 guint                 *context_id)
{
	gpointer data;

	g_assert (amtk_window->priv->statusbar != NULL);
	g_assert (context_id != NULL);

	data = g_object_get_data (G_OBJECT (amtk_menu_shell), MENU_SHELL_STATUSBAR_CONTEXT_ID_KEY);

	if (data == NULL && !create)
	{
		return FALSE;
	}

	if (data == NULL)
	{
		*context_id = gtk_statusbar_get_context_id (amtk_window->priv->statusbar,
							    "Show long description of menu items.");

		g_object_set_data (G_OBJECT (amtk_menu_shell),
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
get_menu_item_long_description (AmtkMenuShell *amtk_menu_shell,
				GtkMenuItem   *menu_item)
{
	const gchar *long_description;
	gpointer data;
	gboolean is_for_recent_chooser;

	long_description = amtk_menu_item_get_long_description (menu_item);
	if (long_description != NULL)
	{
		return g_strdup (long_description);
	}

	data = g_object_get_data (G_OBJECT (amtk_menu_shell), MENU_SHELL_FOR_RECENT_CHOOSER_KEY);
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

		gtk_menu_shell = amtk_menu_shell_get_menu_shell (amtk_menu_shell);
		recent_chooser_menu = GTK_RECENT_CHOOSER_MENU (gtk_menu_shell);
		uri = amtk_utils_recent_chooser_menu_get_item_uri (recent_chooser_menu, menu_item);

		if (uri == NULL)
		{
			return NULL;
		}

		file = g_file_new_for_uri (uri);
		g_free (uri);

		parse_name = g_file_get_parse_name (file);
		g_object_unref (file);

		nicer_filename = _amtk_utils_replace_home_dir_with_tilde (parse_name);
		g_free (parse_name);

		/* Translators: %s is a filename. */
		ret = g_strdup_printf (_("Open “%s”"), nicer_filename);
		g_free (nicer_filename);

		return ret;
	}

	return NULL;
}

static void
menu_item_selected_cb (AmtkMenuShell *amtk_menu_shell,
		       GtkMenuItem   *menu_item,
		       gpointer       user_data)
{
	AmtkApplicationWindow *amtk_window = AMTK_APPLICATION_WINDOW (user_data);
	gchar *long_description;
	guint context_id;

	if (amtk_window->priv->statusbar == NULL)
	{
		return;
	}

	long_description = get_menu_item_long_description (amtk_menu_shell, menu_item);
	if (long_description == NULL)
	{
		return;
	}

	get_statusbar_context_id_for_menu_shell (amtk_window,
						 amtk_menu_shell,
						 TRUE,
						 &context_id);

	gtk_statusbar_push (amtk_window->priv->statusbar,
			    context_id,
			    long_description);

	g_free (long_description);
}

static void
menu_item_deselected_cb (AmtkMenuShell *amtk_menu_shell,
			 GtkMenuItem   *menu_item,
			 gpointer       user_data)
{
	AmtkApplicationWindow *amtk_window = AMTK_APPLICATION_WINDOW (user_data);
	const gchar *long_description;
	gpointer data;
	gboolean is_for_recent_chooser;
	guint context_id;

	if (amtk_window->priv->statusbar == NULL)
	{
		return;
	}

	long_description = amtk_menu_item_get_long_description (menu_item);

	data = g_object_get_data (G_OBJECT (amtk_menu_shell), MENU_SHELL_FOR_RECENT_CHOOSER_KEY);
	is_for_recent_chooser = data != NULL ? GPOINTER_TO_INT (data) : FALSE;

	if (long_description == NULL && !is_for_recent_chooser)
	{
		return;
	}

	if (get_statusbar_context_id_for_menu_shell (amtk_window,
						     amtk_menu_shell,
						     FALSE,
						     &context_id))
	{
		gtk_statusbar_pop (amtk_window->priv->statusbar, context_id);
	}
}

static void
statusbar_notify_cb (AmtkApplicationWindow *amtk_window,
		     GParamSpec            *pspec,
		     gpointer               user_data)
{
	AmtkMenuShell *amtk_menu_shell = AMTK_MENU_SHELL (user_data);

	g_object_set_data (G_OBJECT (amtk_menu_shell),
			   MENU_SHELL_STATUSBAR_CONTEXT_ID_KEY,
			   NULL);
}

/**
 * amtk_application_window_connect_menu_to_statusbar:
 * @amtk_window: an #AmtkApplicationWindow.
 * @menu_shell: a #GtkMenuShell.
 *
 * Connects to the #AmtkMenuShell::menu-item-selected and
 * #AmtkMenuShell::menu-item-deselected signals of @menu_shell to push/pop the
 * long description of #GtkMenuItem's to the #AmtkApplicationWindow:statusbar.
 *
 * The long description is retrieved with amtk_menu_item_get_long_description().
 * So amtk_menu_item_set_long_description() must have been called, which is the
 * case if the #GtkMenuItem has been created with #AmtkFactory.
 *
 * Since: 2.0
 */
void
amtk_application_window_connect_menu_to_statusbar (AmtkApplicationWindow *amtk_window,
						   GtkMenuShell          *menu_shell)
{
	AmtkMenuShell *amtk_menu_shell;

	g_return_if_fail (AMTK_IS_APPLICATION_WINDOW (amtk_window));
	g_return_if_fail (GTK_IS_MENU_SHELL (menu_shell));

	amtk_menu_shell = amtk_menu_shell_get_from_gtk_menu_shell (menu_shell);

	g_signal_connect_object (amtk_menu_shell,
				 "menu-item-selected",
				 G_CALLBACK (menu_item_selected_cb),
				 amtk_window,
				 0);

	g_signal_connect_object (amtk_menu_shell,
				 "menu-item-deselected",
				 G_CALLBACK (menu_item_deselected_cb),
				 amtk_window,
				 0);

	g_signal_connect_object (amtk_window,
				 "notify::statusbar",
				 G_CALLBACK (statusbar_notify_cb),
				 amtk_menu_shell,
				 0);
}

/**
 * amtk_application_window_connect_recent_chooser_menu_to_statusbar:
 * @amtk_window: an #AmtkApplicationWindow.
 * @menu: a #GtkRecentChooserMenu.
 *
 * An alternative to gtk_recent_chooser_set_show_tips(). Shows the full path in
 * the #AmtkApplicationWindow:statusbar when a #GtkMenuItem of @menu is
 * selected.
 *
 * The full path is retrieved with
 * amtk_utils_recent_chooser_menu_get_item_uri().
 *
 * Since: 2.0
 */
void
amtk_application_window_connect_recent_chooser_menu_to_statusbar (AmtkApplicationWindow *amtk_window,
								  GtkRecentChooserMenu  *menu)
{
	AmtkMenuShell *amtk_menu_shell;

	g_return_if_fail (AMTK_IS_APPLICATION_WINDOW (amtk_window));
	g_return_if_fail (GTK_IS_RECENT_CHOOSER_MENU (menu));

	amtk_menu_shell = amtk_menu_shell_get_from_gtk_menu_shell (GTK_MENU_SHELL (menu));

	g_object_set_data (G_OBJECT (amtk_menu_shell),
			   MENU_SHELL_FOR_RECENT_CHOOSER_KEY,
			   GINT_TO_POINTER (TRUE));

	amtk_application_window_connect_menu_to_statusbar (amtk_window, GTK_MENU_SHELL (menu));
}

static void
open_recent_file_cb (GtkRecentChooser *recent_chooser,
		     gpointer          user_data)
{
	AmtkApplicationWindow *amtk_window = AMTK_APPLICATION_WINDOW (user_data);
	gchar *uri;
	GFile *files[1];
	GtkApplication *app;

	uri = gtk_recent_chooser_get_current_uri (recent_chooser);
	files[0] = g_file_new_for_uri (uri);

	app = gtk_window_get_application (GTK_WINDOW (amtk_window->priv->gtk_window));
	g_application_open (G_APPLICATION (app), files, 1, "");

	g_free (uri);
	g_object_unref (files[0]);
}

/**
 * amtk_application_window_create_open_recent_menu:
 * @amtk_window: an #AmtkApplicationWindow.
 *
 * Creates a simple and generic #GtkRecentChooserMenu.
 *
 * The #GtkRecentChooser is configured to show files only recently used with the
 * current application, as returned by g_get_application_name(). If recent files
 * are added to the default #GtkRecentManager with
 * gtk_recent_manager_add_item(), the files will normally show up in the
 * #GtkRecentChooserMenu.
 *
 * The #GtkRecentChooserMenu is connected to the statusbar with
 * amtk_application_window_connect_recent_chooser_menu_to_statusbar().
 *
 * When the #GtkRecentChooser::item-activated signal is emitted,
 * g_application_open() is called (with an empty hint), so the #GApplication
 * must have the %G_APPLICATION_HANDLES_OPEN flag set.
 *
 * Returns: (transfer floating): a new #GtkRecentChooserMenu.
 * Since: 3.0
 */
GtkWidget *
amtk_application_window_create_open_recent_menu (AmtkApplicationWindow *amtk_window)
{
	GtkRecentChooserMenu *recent_chooser_menu;
	GtkRecentChooser *recent_chooser;
	GtkRecentFilter *filter;

	g_return_val_if_fail (AMTK_IS_APPLICATION_WINDOW (amtk_window), NULL);

	recent_chooser_menu = GTK_RECENT_CHOOSER_MENU (gtk_recent_chooser_menu_new ());

	recent_chooser = GTK_RECENT_CHOOSER (recent_chooser_menu);
	gtk_recent_chooser_set_local_only (recent_chooser, FALSE);
	gtk_recent_chooser_set_sort_type (recent_chooser, GTK_RECENT_SORT_MRU);

	filter = gtk_recent_filter_new ();
	gtk_recent_filter_add_application (filter, g_get_application_name ());
	gtk_recent_chooser_set_filter (recent_chooser, filter);

	amtk_application_window_connect_recent_chooser_menu_to_statusbar (amtk_window, recent_chooser_menu);

	g_signal_connect_object (recent_chooser,
				 "item-activated",
				 G_CALLBACK (open_recent_file_cb),
				 amtk_window,
				 0);

	return GTK_WIDGET (recent_chooser_menu);
}

/**
 * amtk_application_window_create_open_recent_menu_item:
 * @amtk_window: an #AmtkApplicationWindow.
 *
 * Creates a #GtkMenuItem with a simple and generic #GtkRecentChooserMenu as
 * submenu. The #GtkRecentChooserMenu is created with
 * amtk_application_window_create_open_recent_menu().
 *
 * Returns: (transfer floating): a new #GtkMenuItem.
 * Since: 2.0
 */
GtkWidget *
amtk_application_window_create_open_recent_menu_item (AmtkApplicationWindow *amtk_window)
{
	GtkMenuItem *menu_item;
	gchar *long_description;
	GtkWidget *recent_chooser_menu;

	g_return_val_if_fail (AMTK_IS_APPLICATION_WINDOW (amtk_window), NULL);

	menu_item = GTK_MENU_ITEM (gtk_menu_item_new_with_mnemonic (_("Open _Recent")));

	/* Translators: %s is the application name. */
	long_description = g_strdup_printf (_("Open a file recently used with %s"),
					    g_get_application_name ());
	amtk_menu_item_set_long_description (menu_item, long_description);
	g_free (long_description);

	recent_chooser_menu = amtk_application_window_create_open_recent_menu (amtk_window);
	gtk_menu_item_set_submenu (menu_item, recent_chooser_menu);

	return GTK_WIDGET (menu_item);
}

/* ex:set ts=8 noet: */
