/* SPDX-FileCopyrightText: 2016 - David Rabel <david.rabel@noresoft.com>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "tepl-gutter-renderer-folds-sub.h"

static GtkWidget *
create_view (void)
{
	GtkWidget *view;
	GtkTextBuffer *buffer;
	GtkSourceGutter *gutter;
	GtkSourceGutterRenderer *renderer;

	view = tepl_view_new ();

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
	gtk_text_buffer_insert_at_cursor (buffer,
					  "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n"
					  "11\n12\n13\n14\n15\n16\n17\n18\n19",
					  -1);

	gutter = gtk_source_view_get_gutter (GTK_SOURCE_VIEW (view), GTK_TEXT_WINDOW_LEFT);
	renderer = tepl_gutter_renderer_folds_sub_new ();
	gtk_source_gutter_insert (gutter, renderer, 0);

	return view;
}

int
main (int argc, char *argv[])
{
	GtkWidget *window;
	GtkWidget *scrolled_window;

	tepl_init ();
	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (window), 500, 500);
	g_signal_connect (window, "destroy", gtk_main_quit, NULL);

	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (scrolled_window), create_view ());
	gtk_container_add (GTK_CONTAINER (window), scrolled_window);

	gtk_widget_show_all (window);

	gtk_main ();
	tepl_finalize ();
	return 0;
}
