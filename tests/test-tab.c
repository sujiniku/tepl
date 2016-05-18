/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include <gtef/gtef.h>
#include <stdlib.h>

static void
add_info_bars (GtefTab *tab)
{
	GtkInfoBar *info_bar1;
	GtkInfoBar *info_bar2;
	GtkWidget *label;
	GtkWidget *content_area;

	info_bar1 = GTK_INFO_BAR (gtk_info_bar_new ());
	info_bar2 = GTK_INFO_BAR (gtk_info_bar_new ());

	gtk_info_bar_set_show_close_button (info_bar1, TRUE);
	gtk_info_bar_set_show_close_button (info_bar2, TRUE);

	label = gtk_label_new ("First info bar.");
	content_area = gtk_info_bar_get_content_area (info_bar1);
	gtk_container_add (GTK_CONTAINER (content_area), label);

	label = gtk_label_new ("Second info bar. This is a revolution! Two info bars, wow!");
	content_area = gtk_info_bar_get_content_area (info_bar2);
	gtk_container_add (GTK_CONTAINER (content_area), label);

	gtef_tab_add_info_bar (tab, info_bar1);
	gtef_tab_add_info_bar (tab, info_bar2);

	gtk_widget_show_all (GTK_WIDGET (info_bar1));
	gtk_widget_show_all (GTK_WIDGET (info_bar2));
}

static GtefTab *
create_tab (void)
{
	GtkWidget *view;
	GtkWidget *scrolled_window;
	GtefTab *tab;

	view = gtef_view_new ();

	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (scrolled_window), view);
	g_object_set (scrolled_window,
		      "expand", TRUE,
		      NULL);

	tab = gtef_tab_new (scrolled_window);
	add_info_bars (tab);

	gtk_widget_show_all (GTK_WIDGET (tab));

	return tab;
}

gint
main (gint    argc,
      gchar **argv)
{
	GtkWidget *window;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);

	g_signal_connect (window,
			  "destroy",
			  G_CALLBACK (gtk_main_quit),
			  NULL);

	gtk_container_add (GTK_CONTAINER (window),
			   GTK_WIDGET (create_tab ()));

	gtk_widget_show (window);

	gtk_main ();

	return EXIT_SUCCESS;
}
