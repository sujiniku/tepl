/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <tepl/tepl.h>
#include <stdlib.h>

static void
language_activated_cb (TeplLanguageChooser *chooser,
		       GtkSourceLanguage   *language,
		       gpointer             user_data)
{
	if (language != NULL)
	{
		g_message ("Language activated: %s", gtk_source_language_get_id (language));
	}
	else
	{
		g_message ("Plain Text activated.");
	}
}

static void
select_random_language (TeplLanguageChooserWidget *chooser_widget)
{
	GtkSourceLanguageManager *manager;
	GtkSourceLanguage *language;

	manager = gtk_source_language_manager_get_default ();
	// html has been picked at random.
	language = gtk_source_language_manager_get_language (manager, "html");

	tepl_language_chooser_select_language (TEPL_LANGUAGE_CHOOSER (chooser_widget), language);
}

int
main (int    argc,
      char **argv)
{
	GtkWidget *window;
	TeplLanguageChooserWidget *chooser_widget;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	g_signal_connect (window, "destroy", gtk_main_quit, NULL);

	chooser_widget = tepl_language_chooser_widget_new ();
	select_random_language (chooser_widget);
	gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (chooser_widget));

	g_signal_connect (chooser_widget,
			  "language-activated",
			  G_CALLBACK (language_activated_cb),
			  NULL);

	gtk_widget_show_all (window);

	gtk_main ();
	return EXIT_SUCCESS;
}
