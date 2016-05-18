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
