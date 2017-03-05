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

#include "config.h"
#include "gtef-application-window.h"
#include <glib/gi18n-lib.h>
#include "gtef-action-info.h"
#include "gtef-action-info-central-store.h"
#include "gtef-menu-item.h"
#include "gtef-menu-shell.h"
#include "gtef-utils.h"

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
#define MENU_SHELL_FOR_RECENT_CHOOSER_KEY "gtef-menu-shell-for-recent-chooser-key"

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

/* Free the return value with g_free(). */
static gchar *
get_menu_item_long_description (GtefMenuShell *gtef_menu_shell,
				GtkMenuItem   *menu_item)
{
	const gchar *long_description;
	gpointer data;
	gboolean is_for_recent_chooser;

	long_description = gtef_menu_item_get_long_description (menu_item);
	if (long_description != NULL)
	{
		return g_strdup (long_description);
	}

	data = g_object_get_data (G_OBJECT (gtef_menu_shell), MENU_SHELL_FOR_RECENT_CHOOSER_KEY);
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

		gtk_menu_shell = gtef_menu_shell_get_menu_shell (gtef_menu_shell);
		recent_chooser_menu = GTK_RECENT_CHOOSER_MENU (gtk_menu_shell);
		uri = gtef_utils_recent_chooser_menu_get_item_uri (recent_chooser_menu, menu_item);

		if (uri == NULL)
		{
			return NULL;
		}

		file = g_file_new_for_uri (uri);
		g_free (uri);

		parse_name = g_file_get_parse_name (file);
		g_object_unref (file);

		nicer_filename = _gtef_utils_replace_home_dir_with_tilde (parse_name);
		g_free (parse_name);

		/* Translators: %s is a filename. */
		ret = g_strdup_printf (_("Open “%s”"), nicer_filename);
		g_free (nicer_filename);

		return ret;
	}

	return NULL;
}

static void
menu_item_selected_cb (GtefMenuShell *gtef_menu_shell,
		       GtkMenuItem   *menu_item,
		       gpointer       user_data)
{
	GtefApplicationWindow *gtef_window = GTEF_APPLICATION_WINDOW (user_data);
	gchar *long_description;
	guint context_id;

	if (gtef_window->priv->statusbar == NULL)
	{
		return;
	}

	long_description = get_menu_item_long_description (gtef_menu_shell, menu_item);
	if (long_description == NULL)
	{
		return;
	}

	get_statusbar_context_id_for_menu_shell (gtef_window,
						 gtef_menu_shell,
						 TRUE,
						 &context_id);

	gtk_statusbar_push (gtef_window->priv->statusbar,
			    context_id,
			    long_description);

	g_free (long_description);
}

static void
menu_item_deselected_cb (GtefMenuShell *gtef_menu_shell,
			 GtkMenuItem   *menu_item,
			 gpointer       user_data)
{
	GtefApplicationWindow *gtef_window = GTEF_APPLICATION_WINDOW (user_data);
	const gchar *long_description;
	gpointer data;
	gboolean is_for_recent_chooser;
	guint context_id;

	if (gtef_window->priv->statusbar == NULL)
	{
		return;
	}

	long_description = gtef_menu_item_get_long_description (menu_item);

	data = g_object_get_data (G_OBJECT (gtef_menu_shell), MENU_SHELL_FOR_RECENT_CHOOSER_KEY);
	is_for_recent_chooser = data != NULL ? GPOINTER_TO_INT (data) : FALSE;

	if (long_description == NULL && !is_for_recent_chooser)
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
 * the long description of #GtkMenuItem's to the
 * #GtefApplicationWindow:statusbar.
 *
 * The long description is retrieved with gtef_menu_item_get_long_description().
 * So gtef_menu_item_set_long_description() must have been called, which is the
 * case if the #GtkMenuItem has been created with the functions available in
 * #GtefActionInfoStore.
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

/**
 * gtef_application_window_connect_recent_chooser_menu_to_statusbar:
 * @gtef_window: a #GtefApplicationWindow.
 * @menu: a #GtkRecentChooserMenu.
 *
 * An alternative to gtk_recent_chooser_set_show_tips(). Shows the full path in
 * the #GtefApplicationWindow:statusbar when a #GtkMenuItem of @menu is
 * selected.
 *
 * The full path is retrieved with
 * gtef_utils_recent_chooser_menu_get_item_uri().
 *
 * Since: 2.0
 */
void
gtef_application_window_connect_recent_chooser_menu_to_statusbar (GtefApplicationWindow *gtef_window,
								  GtkRecentChooserMenu  *menu)
{
	GtefMenuShell *gtef_menu_shell;

	g_return_if_fail (GTEF_IS_APPLICATION_WINDOW (gtef_window));
	g_return_if_fail (GTK_IS_RECENT_CHOOSER_MENU (menu));

	gtef_menu_shell = gtef_menu_shell_get_from_gtk_menu_shell (GTK_MENU_SHELL (menu));

	g_object_set_data (G_OBJECT (gtef_menu_shell),
			   MENU_SHELL_FOR_RECENT_CHOOSER_KEY,
			   GINT_TO_POINTER (TRUE));

	gtef_application_window_connect_menu_to_statusbar (gtef_window, gtef_menu_shell);
}

/* ex:set ts=8 noet: */
