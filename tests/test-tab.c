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
#include "gtef/gtef-progress-info-bar.h"
#include <stdlib.h>

static void
info_bar_response_cb (GtkInfoBar *info_bar,
		      gint        response_id,
		      gpointer    user_data)
{
	gtk_widget_destroy (GTK_WIDGET (info_bar));
}

static void
add_basic_info_bar_button_clicked_cb (GtkButton *button,
				      GtefTab   *tab)
{
	GtkInfoBar *info_bar;
	GtkWidget *label;
	GtkWidget *content_area;

	info_bar = GTK_INFO_BAR (gtk_info_bar_new ());
	gtk_info_bar_set_show_close_button (info_bar, TRUE);

	label = gtk_label_new ("Basic info bar.");
	content_area = gtk_info_bar_get_content_area (info_bar);
	gtk_container_add (GTK_CONTAINER (content_area), label);

	g_signal_connect (info_bar,
			  "response",
			  G_CALLBACK (info_bar_response_cb),
			  NULL);

	gtef_tab_add_info_bar (tab, info_bar);
	gtk_widget_show_all (GTK_WIDGET (info_bar));
}

static void
add_progress_info_bar_button_clicked_cb (GtkButton *button,
					 GtefTab   *tab)
{
	GtefProgressInfoBar *info_bar;

	info_bar = _gtef_progress_info_bar_new ("File loading...", TRUE);
	_gtef_progress_info_bar_set_fraction (info_bar, 0.3);

	g_signal_connect (info_bar,
			  "response",
			  G_CALLBACK (info_bar_response_cb),
			  NULL);

	gtef_tab_add_info_bar (tab, GTK_INFO_BAR (info_bar));
	gtk_widget_show_all (GTK_WIDGET (info_bar));
}

static GtkWidget *
create_side_panel (GtefTab *tab)
{
	GtkGrid *vgrid;
	GtkWidget *add_basic_info_bar_button;
	GtkWidget *add_progress_info_bar_button;

	vgrid = GTK_GRID (gtk_grid_new ());
	gtk_orientable_set_orientation (GTK_ORIENTABLE (vgrid), GTK_ORIENTATION_VERTICAL);
	gtk_grid_set_row_spacing (vgrid, 6);

	add_basic_info_bar_button = gtk_button_new_with_label ("Add basic info bar");
	gtk_container_add (GTK_CONTAINER (vgrid), add_basic_info_bar_button);

	g_signal_connect_object (add_basic_info_bar_button,
				 "clicked",
				 G_CALLBACK (add_basic_info_bar_button_clicked_cb),
				 tab,
				 0);

	add_progress_info_bar_button = gtk_button_new_with_label ("Add progress info bar");
	gtk_container_add (GTK_CONTAINER (vgrid), add_progress_info_bar_button);

	g_signal_connect_object (add_progress_info_bar_button,
				 "clicked",
				 G_CALLBACK (add_progress_info_bar_button_clicked_cb),
				 tab,
				 0);

	return GTK_WIDGET (vgrid);
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
	gtk_widget_show_all (GTK_WIDGET (tab));

	return tab;
}

static GtkWidget *
create_window_content (void)
{
	GtkGrid *hgrid;
	GtefTab *tab;
	GtkWidget *side_panel;

	hgrid = GTK_GRID (gtk_grid_new ());
	gtk_orientable_set_orientation (GTK_ORIENTABLE (hgrid), GTK_ORIENTATION_HORIZONTAL);
	gtk_grid_set_column_spacing (hgrid, 6);
	g_object_set (hgrid,
		      "margin", 6,
		      NULL);

	tab = create_tab ();
	side_panel = create_side_panel (tab);

	gtk_container_add (GTK_CONTAINER (hgrid), side_panel);
	gtk_container_add (GTK_CONTAINER (hgrid), GTK_WIDGET (tab));

	gtk_widget_show_all (GTK_WIDGET (hgrid));

	return GTK_WIDGET (hgrid);
}

gint
main (gint    argc,
      gchar **argv)
{
	GtkWidget *window;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);
	gtk_container_add (GTK_CONTAINER (window), create_window_content ());
	gtk_widget_show (window);

	g_signal_connect (window,
			  "destroy",
			  G_CALLBACK (gtk_main_quit),
			  NULL);

	gtk_main ();

	return EXIT_SUCCESS;
}
