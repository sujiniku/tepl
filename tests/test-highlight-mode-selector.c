/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <tepl/tepl.h>
#include <stdlib.h>

static void
language_selected_cb (TeplHighlightModeSelector *selector,
		      GtkSourceLanguage         *language,
		      gpointer                   user_data)
{
	g_message ("Language selected: %s", gtk_source_language_get_id (language));
}

int
main (int    argc,
      char **argv)
{
	GtkWidget *window;
	TeplHighlightModeSelector *selector;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (window), 500, 500);
	g_signal_connect (window, "destroy", gtk_main_quit, NULL);

	selector = tepl_highlight_mode_selector_new ();
	gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (selector));

	g_signal_connect (selector,
			  "language-selected",
			  G_CALLBACK (language_selected_cb),
			  NULL);

	gtk_widget_show_all (window);

	gtk_main ();
	return EXIT_SUCCESS;
}
