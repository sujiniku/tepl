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

#include <tepl/tepl.h>

static void
add_action_info_entries (TeplApplication *tepl_app)
{
	AmtkActionInfoStore *store;

	const AmtkActionInfoEntry entries[] =
	{
		/* action, icon, label, accel, tooltip */

		{ "app.quit", "application-exit", "_Quit", "<Control>q",
		  "Quit the application" },

		{ "app.about", "help-about", "_About", NULL,
		  "About this application" },

		{ "win.show-side-panel", NULL, "_Side Panel", "F12",
		  "Show or hide the side panel" },
	};

	store = tepl_application_get_app_action_info_store (tepl_app);

	amtk_action_info_store_add_entries (store,
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
add_app_action_entries (GApplication *app)
{
	const GActionEntry entries[] =
	{
		{ "quit", quit_activate_cb },
		{ "about", about_activate_cb },
	};

	amtk_action_map_add_action_entries_check_dups (G_ACTION_MAP (app),
						       entries,
						       G_N_ELEMENTS (entries),
						       app);
}

static void
startup_cb (GApplication *g_app,
	    gpointer      user_data)
{
	TeplApplication *tepl_app;

	tepl_app = tepl_application_get_from_gtk_application (GTK_APPLICATION (g_app));

	add_action_info_entries (tepl_app);
	add_app_action_entries (g_app);
}

static GtkWidget *
create_file_submenu (void)
{
	GtkMenuShell *file_submenu;
	AmtkFactory *factory;

	file_submenu = GTK_MENU_SHELL (gtk_menu_new ());

	factory = amtk_factory_new_with_default_application ();
	gtk_menu_shell_append (file_submenu, amtk_factory_create_menu_item (factory, "app.quit"));
	g_object_unref (factory);

	return GTK_WIDGET (file_submenu);
}

static GtkWidget *
create_view_submenu (void)
{
	GtkMenuShell *view_submenu;
	AmtkFactory *factory;

	view_submenu = GTK_MENU_SHELL (gtk_menu_new ());

	factory = amtk_factory_new_with_default_application ();
	gtk_menu_shell_append (view_submenu, amtk_factory_create_check_menu_item (factory, "win.show-side-panel"));
	g_object_unref (factory);

	return GTK_WIDGET (view_submenu);
}

static GtkWidget *
create_help_submenu (void)
{
	GtkMenuShell *help_submenu;
	AmtkFactory *factory;

	help_submenu = GTK_MENU_SHELL (gtk_menu_new ());

	factory = amtk_factory_new_with_default_application ();
	gtk_menu_shell_append (help_submenu, amtk_factory_create_menu_item (factory, "app.about"));
	g_object_unref (factory);

	return GTK_WIDGET (help_submenu);
}

static GtkMenuBar *
create_menu_bar (void)
{
	GtkWidget *file_menu_item;
	GtkWidget *view_menu_item;
	GtkWidget *help_menu_item;
	GtkMenuBar *menu_bar;
	TeplApplication *app;
	AmtkActionInfoStore *store;

	file_menu_item = gtk_menu_item_new_with_mnemonic ("_File");
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (file_menu_item),
				   create_file_submenu ());

	view_menu_item = gtk_menu_item_new_with_mnemonic ("_View");
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (view_menu_item),
				   create_view_submenu ());

	help_menu_item = gtk_menu_item_new_with_mnemonic ("_Help");
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (help_menu_item),
				   create_help_submenu ());

	menu_bar = GTK_MENU_BAR (gtk_menu_bar_new ());
	gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), file_menu_item);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), view_menu_item);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), help_menu_item);

	app = tepl_application_get_default ();
	store = tepl_application_get_app_action_info_store (app);
	amtk_action_info_store_check_all_used (store);

	return menu_bar;
}

static void
add_win_actions (GtkApplicationWindow *window,
		 GtkWidget            *side_panel)
{
	GPropertyAction *side_panel_action;

	side_panel_action = g_property_action_new ("show-side-panel", side_panel, "visible");
	g_action_map_add_action (G_ACTION_MAP (window), G_ACTION (side_panel_action));
	g_object_unref (side_panel_action);
}

static void
activate_cb (GApplication *g_app,
	     gpointer      user_data)
{
	GtkWidget *window;
	GtkWidget *vgrid;
	GtkWidget *hgrid;
	GtkWidget *side_panel;

	window = gtk_application_window_new (GTK_APPLICATION (g_app));
	gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);

	vgrid = gtk_grid_new ();
	gtk_orientable_set_orientation (GTK_ORIENTABLE (vgrid), GTK_ORIENTATION_VERTICAL);

	gtk_container_add (GTK_CONTAINER (vgrid), GTK_WIDGET (create_menu_bar ()));

	hgrid = gtk_grid_new ();
	side_panel = gtk_label_new ("Side panel");
	gtk_container_add (GTK_CONTAINER (hgrid), side_panel);
	gtk_container_add (GTK_CONTAINER (hgrid), gtk_label_new ("Text view"));
	gtk_container_add (GTK_CONTAINER (vgrid), hgrid);

	add_win_actions (GTK_APPLICATION_WINDOW (window), side_panel);

	gtk_container_add (GTK_CONTAINER (window), vgrid);
	gtk_widget_show_all (window);
}

int
main (int    argc,
      char **argv)
{
	GtkApplication *app;
	int status;

	app = gtk_application_new ("org.gnome.tepl.test-menu", G_APPLICATION_FLAGS_NONE);

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
