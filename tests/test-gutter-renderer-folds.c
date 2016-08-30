/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2016 - David Rabel <david.rabel@noresoft.com>
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

#include "gtef-gutter-renderer-folds-sub.h"

static void
add_folding_gutter (GtkSourceView *view)
{
	GtkSourceGutter *gutter;
	GtkSourceGutterRenderer *renderer;
	GtkTextBuffer *text_buffer;

	text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
	gtk_text_buffer_insert_at_cursor (text_buffer, "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19", -1);

	gutter = gtk_source_view_get_gutter (view, GTK_TEXT_WINDOW_LEFT);
	renderer = gtef_gutter_renderer_folds_sub_new ();

	gtk_source_gutter_insert (gutter, renderer, 0);
}

int
main (int argc, char *argv[])
{
	GtkWidget *window;
	GtkWidget *scrolled_window;
	GtkWidget *gtef_view;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (window), 500, 500);
	g_signal_connect (window, "destroy", gtk_main_quit, NULL);

	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (window), scrolled_window);

	gtef_view = gtef_view_new ();
	gtk_container_add (GTK_CONTAINER (scrolled_window), gtef_view);

	add_folding_gutter (GTK_SOURCE_VIEW (gtef_view));

	gtk_widget_show_all (window);

	gtk_main ();
	return 0;
}
