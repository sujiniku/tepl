/* SPDX-FileCopyrightText: 2016 - David Rabel <david.rabel@noresoft.com>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <tepl/tepl.h>

static GtkWidget *
create_view (void)
{
	GtkWidget *view;
	GtkTextBuffer *buffer;
	TeplFoldRegion *fold_region;
	GtkTextIter start_iter;
	GtkTextIter end_iter;

	view = tepl_view_new ();

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
	gtk_text_buffer_insert_at_cursor (buffer, "Line0\nLine1\nLine2\nLine3\nLine4\nLine5", -1);

	gtk_text_buffer_get_iter_at_line (buffer, &start_iter, 1);
	gtk_text_buffer_get_iter_at_line (buffer, &end_iter, 3);

	/* FIXME: fold_region must be unreffed on exit. */
	fold_region = tepl_fold_region_new (buffer,
	                                    &start_iter,
	                                    &end_iter);

	tepl_fold_region_set_folded (fold_region, TRUE);

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
