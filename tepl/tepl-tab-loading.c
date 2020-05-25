/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "config.h"
#include "tepl-tab-loading.h"
#include <glib/gi18n-lib.h>
#include "tepl-file-loader.h"
#include "tepl-info-bar.h"

static void
load_file_cb (GObject      *source_object,
	      GAsyncResult *result,
	      gpointer      user_data)
{
	TeplFileLoader *loader = TEPL_FILE_LOADER (source_object);
	TeplTab *tab = TEPL_TAB (user_data);
	GError *error = NULL;

	if (tepl_file_loader_load_finish (loader, result, &error))
	{
		TeplBuffer *buffer;
		TeplFile *file;

		buffer = tepl_tab_get_buffer (tab);
		file = tepl_buffer_get_file (buffer);
		tepl_file_add_uri_to_recent_manager (file);
	}

	if (error != NULL &&
	    !g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
	{
		TeplInfoBar *info_bar;

		info_bar = tepl_info_bar_new_simple (GTK_MESSAGE_ERROR,
						     _("Error when loading the file."),
						     error->message);

		tepl_tab_add_info_bar (tab, GTK_INFO_BAR (info_bar));
		gtk_widget_show (GTK_WIDGET (info_bar));
	}

	g_clear_error (&error);
	g_object_unref (loader);
	g_object_unref (tab);
}

/**
 * tepl_tab_load_file:
 * @tab: a #TeplTab.
 * @location: a #GFile.
 *
 * Unconditionally loads a file in @tab, regardless if there are unsaved changes
 * in the #GtkTextBuffer. The previous buffer content is lost.
 *
 * This function is asynchronous, there is no way to know when the file loading
 * is finished.
 *
 * Since: 4.0
 */
void
tepl_tab_load_file (TeplTab *tab,
		    GFile   *location)
{
	TeplBuffer *buffer;
	TeplFile *file;
	TeplFileLoader *loader;
	GCancellable *cancellable;

	g_return_if_fail (TEPL_IS_TAB (tab));
	g_return_if_fail (G_IS_FILE (location));

	buffer = tepl_tab_get_buffer (tab);
	file = tepl_buffer_get_file (buffer);

	tepl_file_set_location (file, location);
	loader = tepl_file_loader_new (buffer, file);

	cancellable = g_cancellable_new ();

	/* If there is a request to destroy the tab, it's pointless to continue
	 * loading the file. So, cancel the operation when the tab is destroyed,
	 * to free up resources for other operations.
	 */
	g_signal_connect_object (tab,
				 "destroy",
				 G_CALLBACK (g_cancellable_cancel),
				 cancellable,
				 G_CONNECT_SWAPPED);

	tepl_file_loader_load_async (loader,
				     G_PRIORITY_DEFAULT,
				     cancellable,
				     load_file_cb,
				     g_object_ref (tab));

	g_object_unref (cancellable);
}
