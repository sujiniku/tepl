/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <tepl/tepl.h>
#include <stdlib.h>

static void
select_random_language (TeplLanguageChooserDialog *dialog)
{
	GtkSourceLanguageManager *manager;
	GtkSourceLanguage *language;

	manager = gtk_source_language_manager_get_default ();
	// xml has been picked at random.
	language = gtk_source_language_manager_get_language (manager, "xml");

	tepl_language_chooser_select_language (TEPL_LANGUAGE_CHOOSER (dialog), language);
}

static void
language_activated_cb (TeplLanguageChooserDialog *dialog,
		       GtkSourceLanguage         *language,
		       gpointer                   user_data)
{
	if (language != NULL)
	{
		g_message ("Language activated: %s", gtk_source_language_get_id (language));
	}
	else
	{
		g_message ("Plain Text activated.");
	}

	gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
dialog_response_after_cb (GtkDialog *dialog,
			  gint       response_id,
			  gpointer   user_data)
{
	gtk_widget_destroy (GTK_WIDGET (dialog));
}

int
main (int    argc,
      char **argv)
{
	TeplLanguageChooserDialog *dialog;

	gtk_init (&argc, &argv);

	dialog = tepl_language_chooser_dialog_new (NULL);
	select_random_language (dialog);

	g_signal_connect (dialog,
			  "language-activated",
			  G_CALLBACK (language_activated_cb),
			  NULL);

	g_signal_connect_after (dialog,
				"response",
				G_CALLBACK (dialog_response_after_cb),
				NULL);

	g_signal_connect (dialog,
			  "destroy",
			  gtk_main_quit,
			  NULL);

	gtk_widget_show (GTK_WIDGET (dialog));
	gtk_main ();
	return EXIT_SUCCESS;
}
