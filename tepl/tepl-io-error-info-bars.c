/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2005 - Paolo Maggi
 * Copyright 2020 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * Tepl is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * Tepl is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "tepl-io-error-info-bars.h"
#include <glib/gi18n-lib.h>

/**
 * SECTION:io-error-info-bars
 * @Short_description: Verbose error reporting for file I/O operations
 * @Title: IO error info bars
 *
 * Verbose error reporting for file I/O operations.
 */

/**
 * tepl_io_error_info_bar_file_already_open:
 * @location: the #GFile already open in another window.
 *
 * Creates a warning about @location being already open in another window,
 * offering two possible actions:
 * - Edit anyway: %GTK_RESPONSE_YES.
 * - Don't edit: %GTK_RESPONSE_CANCEL.
 *
 * Returns: (transfer floating): the newly created #TeplInfoBar.
 * Since: 4.6
 */
TeplInfoBar *
tepl_io_error_info_bar_file_already_open (GFile *location)
{
	TeplInfoBar *info_bar;
	gchar *uri;
	gchar *primary_msg;

	g_return_val_if_fail (G_IS_FILE (location), NULL);

	info_bar = tepl_info_bar_new ();

	gtk_info_bar_add_button (GTK_INFO_BAR (info_bar),
				 _("_Edit Anyway"),
				 GTK_RESPONSE_YES);

	gtk_info_bar_add_button (GTK_INFO_BAR (info_bar),
				 _("_Don’t Edit"),
				 GTK_RESPONSE_CANCEL);

	gtk_info_bar_set_message_type (GTK_INFO_BAR (info_bar), GTK_MESSAGE_WARNING);

	uri = g_file_get_parse_name (location);
	primary_msg = g_strdup_printf (_("This file “%s” is already open in another window."), uri);
	tepl_info_bar_add_primary_message (info_bar, primary_msg);
	g_free (uri);
	g_free (primary_msg);

	tepl_info_bar_add_secondary_message (info_bar, _("Do you want to edit it anyway?"));

	return info_bar;
}

/**
 * tepl_io_error_info_bar_cant_create_backup:
 * @location: the #GFile for which the backup failed to be created.
 * @error: must be a %G_IO_ERROR_CANT_CREATE_BACKUP.
 *
 * When a %G_IO_ERROR_CANT_CREATE_BACKUP error occurs while saving @location,
 * offer two possible actions:
 * - Save anyway: %GTK_RESPONSE_YES.
 * - Don't save: %GTK_RESPONSE_CANCEL.
 *
 * Returns: (transfer floating): the newly created #TeplInfoBar.
 * Since: 4.6
 */
/* TODO add another possible action: save as? */
TeplInfoBar *
tepl_io_error_info_bar_cant_create_backup (GFile        *location,
					   const GError *error)
{
	TeplInfoBar *info_bar;
	gchar *uri;
	gchar *primary_msg;
	const gchar *secondary_msg;

	g_return_val_if_fail (G_IS_FILE (location), NULL);
	g_return_val_if_fail (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANT_CREATE_BACKUP), NULL);

	info_bar = tepl_info_bar_new ();

	gtk_info_bar_add_button (GTK_INFO_BAR (info_bar),
				 _("S_ave Anyway"),
				 GTK_RESPONSE_YES);

	gtk_info_bar_add_button (GTK_INFO_BAR (info_bar),
				 _("_Don’t Save"),
				 GTK_RESPONSE_CANCEL);

	gtk_info_bar_set_message_type (GTK_INFO_BAR (info_bar), GTK_MESSAGE_WARNING);

	uri = g_file_get_parse_name (location);
	primary_msg = g_strdup_printf (_("Could not create a backup file while saving “%s”"), uri);
	tepl_info_bar_add_primary_message (info_bar, primary_msg);
	g_free (uri);
	g_free (primary_msg);

	secondary_msg = _("Could not back up the old copy of the file before saving the new one. "
			  "You can ignore this warning and save the file anyway, but if an error "
			  "occurs while saving, you could lose the old copy of the file. Save anyway?");
	tepl_info_bar_add_secondary_message (info_bar, secondary_msg);

	if (error->message != NULL)
	{
		gchar *error_msg;

		error_msg = g_strdup_printf (_("Error message: %s"), error->message);
		tepl_info_bar_add_secondary_message (info_bar, error_msg);
		g_free (error_msg);
	}

	return info_bar;
}

/**
 * tepl_io_error_info_bar_externally_modified:
 * @location: the #GFile for which there has been an external modification.
 * @document_modified: whether the document (e.g. the #GtkTextBuffer) has
 *   unsaved modifications.
 *
 * Creates a warning about @location having changed on disk. The possible
 * actions:
 * - Depending on @document_modified, "Reload" or "Drop changes and reload":
 *   %GTK_RESPONSE_OK.
 * - A close button as added with gtk_info_bar_set_show_close_button().
 *
 * Returns: (transfer floating): the newly created #TeplInfoBar.
 * Since: 4.6
 */
TeplInfoBar *
tepl_io_error_info_bar_externally_modified (GFile    *location,
					    gboolean  document_modified)
{
	TeplInfoBar *info_bar;
	gchar *uri;
	gchar *primary_msg;
	const gchar *button_text;

	g_return_val_if_fail (G_IS_FILE (location), NULL);

	info_bar = tepl_info_bar_new ();

	uri = g_file_get_parse_name (location);
	primary_msg = g_strdup_printf (_("The file “%s” changed on disk."), uri);
	tepl_info_bar_add_primary_message (info_bar, primary_msg);
	g_free (uri);
	g_free (primary_msg);

	button_text = document_modified ? _("Drop Changes and _Reload") : _("_Reload");
	gtk_info_bar_add_button (GTK_INFO_BAR (info_bar),
				 button_text,
				 GTK_RESPONSE_OK);

	gtk_info_bar_set_show_close_button (GTK_INFO_BAR (info_bar), TRUE);
	gtk_info_bar_set_message_type (GTK_INFO_BAR (info_bar), GTK_MESSAGE_WARNING);

	return info_bar;
}
