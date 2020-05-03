/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2020 - Sébastien Wilmet <swilmet@gnome.org>
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
#include "tepl-window-actions-file.h"
#include <amtk/amtk.h>
#include <glib/gi18n-lib.h>
#include "tepl-abstract-factory.h"
#include "tepl-buffer.h"
#include "tepl-file.h"
#include "tepl-tab.h"
#include "tepl-tab-group.h"

/* TeplApplicationWindow GActions for the File menu. */

static void
new_file_activate_cb (GSimpleAction *action,
		      GVariant      *parameter,
		      gpointer       user_data)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (user_data);
	TeplAbstractFactory *factory;
	TeplTab *new_tab;

	factory = tepl_abstract_factory_get_singleton ();
	new_tab = tepl_abstract_factory_create_tab (factory);
	gtk_widget_show (GTK_WIDGET (new_tab));

	tepl_tab_group_append_tab (TEPL_TAB_GROUP (tepl_window), new_tab, TRUE);
}

static void
open_file_chooser_response_cb (GtkFileChooserDialog  *file_chooser_dialog,
			       gint                   response_id,
			       TeplApplicationWindow *tepl_window)
{
	if (response_id == GTK_RESPONSE_ACCEPT)
	{
		GFile *location;
		GtkApplicationWindow *gtk_window;

		location = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (file_chooser_dialog));
		tepl_application_window_open_file (tepl_window, location, TRUE);
		g_object_unref (location);

		/* Present the window because it is not necessarily the most
		 * recently focused window.
		 */
		gtk_window = tepl_application_window_get_application_window (tepl_window);
		gtk_window_present (GTK_WINDOW (gtk_window));
	}

	gtk_widget_destroy (GTK_WIDGET (file_chooser_dialog));
}

static void
open_activate_cb (GSimpleAction *open_action,
		  GVariant      *parameter,
		  gpointer       user_data)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (user_data);
	GtkApplicationWindow *gtk_window;
	GtkWidget *file_chooser_dialog;
	GtkWindowGroup *window_group;

	gtk_window = tepl_application_window_get_application_window (tepl_window);

	/* Create a GtkFileChooserDialog, not a GtkFileChooserNative, because
	 * with GtkFileChooserNative the GFile that we obtain (in flatpak)
	 * doesn't have the real path to the file, so it would ruin some
	 * features for text editors:
	 * - showing the directory in parentheses in the window title, or in the
	 *   tab tooltip;
	 * - opening a recent file.
	 * Basically everywhere where the directory is shown.
	 */
	file_chooser_dialog = gtk_file_chooser_dialog_new (_("Open File"),
							   GTK_WINDOW (gtk_window),
							   GTK_FILE_CHOOSER_ACTION_OPEN,
							   _("_Cancel"), GTK_RESPONSE_CANCEL,
							   _("_Open"), GTK_RESPONSE_ACCEPT,
							   NULL);

	gtk_dialog_set_default_response (GTK_DIALOG (file_chooser_dialog), GTK_RESPONSE_ACCEPT);
	gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (file_chooser_dialog), FALSE);

	/* Do not set it modal, it's not absolutely required. But in that case
	 * it's better to destroy the dialog when the main window is closed.
	 */
	gtk_window_set_destroy_with_parent (GTK_WINDOW (file_chooser_dialog), TRUE);

	window_group = tepl_application_window_get_window_group (tepl_window);
	gtk_window_group_add_window (window_group, GTK_WINDOW (file_chooser_dialog));

	g_signal_connect_object (file_chooser_dialog,
				 "response",
				 G_CALLBACK (open_file_chooser_response_cb),
				 tepl_window,
				 0);

	gtk_widget_show (file_chooser_dialog);
}

static void
save_activate_cb (GSimpleAction *save_action,
		  GVariant      *parameter,
		  gpointer       user_data)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (user_data);
	TeplTab *tab;
	TeplBuffer *buffer;
	TeplFile *file;
	GFile *location;

	tab = tepl_tab_group_get_active_tab (TEPL_TAB_GROUP (tepl_window));
	g_return_if_fail (tab != NULL);

	buffer = tepl_tab_get_buffer (tab);
	file = tepl_buffer_get_file (buffer);
	location = tepl_file_get_location (file);

	if (location != NULL)
	{
		tepl_tab_save_async_simple (tab);
	}
	else
	{
		GtkApplicationWindow *gtk_window;

		gtk_window = tepl_application_window_get_application_window (tepl_window);
		g_action_group_activate_action (G_ACTION_GROUP (gtk_window), "tepl-save-as", NULL);
	}
}

static void
save_as_activate_cb (GSimpleAction *save_as_action,
		     GVariant      *parameter,
		     gpointer       user_data)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (user_data);
	TeplTab *tab;

	tab = tepl_tab_group_get_active_tab (TEPL_TAB_GROUP (tepl_window));
	g_return_if_fail (tab != NULL);

	tepl_tab_save_as_async_simple (tab);
}

static void
update_actions_sensitivity (TeplApplicationWindow *tepl_window)
{
	TeplBuffer *buffer;
	GActionMap *action_map;
	GAction *action;

	buffer = tepl_tab_group_get_active_buffer (TEPL_TAB_GROUP (tepl_window));

	action_map = G_ACTION_MAP (tepl_application_window_get_application_window (tepl_window));

	action = g_action_map_lookup_action (action_map, "tepl-save");
	g_simple_action_set_enabled (G_SIMPLE_ACTION (action),
				     buffer != NULL);

	action = g_action_map_lookup_action (action_map, "tepl-save-as");
	g_simple_action_set_enabled (G_SIMPLE_ACTION (action),
				     buffer != NULL);
}

static void
active_buffer_notify_cb (TeplApplicationWindow *tepl_window,
			 GParamSpec            *pspec,
			 gpointer               user_data)
{
	update_actions_sensitivity (tepl_window);
}

void
_tepl_window_actions_file_add_actions (TeplApplicationWindow *tepl_window)
{
	GtkApplicationWindow *gtk_window;

	const GActionEntry entries[] = {
		{ "tepl-new-file", new_file_activate_cb },
		{ "tepl-open", open_activate_cb },
		{ "tepl-save", save_activate_cb },
		{ "tepl-save-as", save_as_activate_cb },
	};

	g_return_if_fail (TEPL_IS_APPLICATION_WINDOW (tepl_window));

	gtk_window = tepl_application_window_get_application_window (tepl_window);
	amtk_action_map_add_action_entries_check_dups (G_ACTION_MAP (gtk_window),
						       entries,
						       G_N_ELEMENTS (entries),
						       tepl_window);

	update_actions_sensitivity (tepl_window);

	g_signal_connect (tepl_window,
			  "notify::active-buffer",
			  G_CALLBACK (active_buffer_notify_cb),
			  NULL);
}
