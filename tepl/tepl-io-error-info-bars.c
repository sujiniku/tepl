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
 * tepl_io_error_info_bar_file_already_open_warning_new:
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
tepl_io_error_info_bar_file_already_open_warning_new (GFile *location)
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
