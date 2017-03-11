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

#include <gtef/gtef.h>

static void
add_action_info_entries (GtefApplication *gtef_app)
{
	GtefActionInfoStore *store;

	const GtefActionInfoEntry entries[] =
	{
		/* action, icon, label, accel, tooltip */

		{ "app.quit", "application-exit", "_Quit", "<Control>q",
		  "Quit the application" },

		{ "app.about", "help-about", "_About", NULL,
		  "About this application" },
	};

	store = gtef_application_get_app_action_info_store (gtef_app);

	gtef_action_info_store_add_entries (store,
					    entries,
					    G_N_ELEMENTS (entries),
					    NULL);
}

static void
quit_activate_cb (GSimpleAction *quit_action,
		  GVariant      *parameter,
		  gpointer       user_data)
{
	g_application_quit (G_APPLICATION (user_data));
}

static void
about_activate_cb (GSimpleAction *about_action,
		   GVariant      *parameter,
		   gpointer       user_data)
{
	g_print ("About\n");
}

static void
add_action_entries (GApplication *app)
{
	const GActionEntry entries[] =
	{
		{ "quit", quit_activate_cb },
		{ "about", about_activate_cb },
	};

	gtef_action_map_add_action_entries_check_dups (G_ACTION_MAP (app),
						       entries,
						       G_N_ELEMENTS (entries),
						       app);
}

static void
startup_cb (GApplication *g_app,
	    gpointer      user_data)
{
	GtefApplication *gtef_app;

	gtef_app = gtef_application_get_from_gtk_application (GTK_APPLICATION (g_app));

	add_action_info_entries (gtef_app);
	add_action_entries (g_app);
}

static GtefActionInfoStore *
get_action_info_store (void)
{
	GtefApplication *app;

	app = gtef_application_get_default ();

	return gtef_application_get_app_action_info_store (app);
}

static GtkWidget *
create_file_submenu (void)
{
	GtefActionInfoStore *store;
	GtkMenuShell *file_submenu;

	store = get_action_info_store ();
	file_submenu = GTK_MENU_SHELL (gtk_menu_new ());

	gtk_menu_shell_append (file_submenu, gtef_action_info_store_create_menu_item (store, "app.quit"));

	return GTK_WIDGET (file_submenu);
}

static GtkWidget *
create_help_submenu (void)
{
	GtefActionInfoStore *store;
	GtkMenuShell *help_submenu;

	store = get_action_info_store ();
	help_submenu = GTK_MENU_SHELL (gtk_menu_new ());

	gtk_menu_shell_append (help_submenu, gtef_action_info_store_create_menu_item (store, "app.about"));

	return GTK_WIDGET (help_submenu);
}

static GtkMenuBar *
create_menu_bar (void)
{
	GtkWidget *file_menu_item;
	GtkWidget *help_menu_item;
	GtkMenuBar *menu_bar;
	GtefActionInfoStore *store;

	file_menu_item = gtk_menu_item_new_with_mnemonic ("_File");
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (file_menu_item),
				   create_file_submenu ());

	help_menu_item = gtk_menu_item_new_with_mnemonic ("_Help");
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (help_menu_item),
				   create_help_submenu ());

	menu_bar = GTK_MENU_BAR (gtk_menu_bar_new ());
	gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), file_menu_item);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), help_menu_item);

	store = get_action_info_store ();
	gtef_action_info_store_check_all_used (store);

	return menu_bar;
}

static GtkWidget *
create_window_content (void)
{
	GtkWidget *vgrid;

	vgrid = gtk_grid_new ();
	gtk_orientable_set_orientation (GTK_ORIENTABLE (vgrid), GTK_ORIENTATION_VERTICAL);

	gtk_container_add (GTK_CONTAINER (vgrid), GTK_WIDGET (create_menu_bar ()));

	gtk_widget_show_all (vgrid);
	return vgrid;
}

static void
activate_cb (GApplication *g_app,
	     gpointer      user_data)
{
	GtkWidget *window;

	window = gtk_application_window_new (GTK_APPLICATION (g_app));
	gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);
	gtk_container_add (GTK_CONTAINER (window), create_window_content ());
	gtk_widget_show (window);
}

gint
main (gint    argc,
      gchar **argv)
{
	GtkApplication *app;
	gint status;

	app = gtk_application_new ("org.gnome.gtef.test-menu", G_APPLICATION_FLAGS_NONE);

	g_signal_connect (app,
			  "startup",
			  G_CALLBACK (startup_cb),
			  NULL);

	g_signal_connect (app,
			  "activate",
			  G_CALLBACK (activate_cb),
			  NULL);

	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);
	return status;
}
