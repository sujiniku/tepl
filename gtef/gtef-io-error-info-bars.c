/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2005 - Paolo Maggi
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

#include "config.h"
#include "gtef-io-error-info-bars.h"
#include <glib/gi18n-lib.h>
#include "gtef-utils.h"

/* Verbose error reporting for file I/O operations (load, save, etc.). */

static gboolean
is_recoverable_error (const GError *error)
{
	gboolean is_recoverable = FALSE;

	if (error->domain == G_IO_ERROR)
	{
		switch (error->code)
		{
			case G_IO_ERROR_PERMISSION_DENIED:
			case G_IO_ERROR_NOT_FOUND:
			case G_IO_ERROR_HOST_NOT_FOUND:
			case G_IO_ERROR_TIMED_OUT:
			case G_IO_ERROR_NOT_MOUNTABLE_FILE:
			case G_IO_ERROR_NOT_MOUNTED:
			case G_IO_ERROR_BUSY:
				is_recoverable = TRUE;
				break;

			default:
				break;
		}
	}

	return is_recoverable;
}

static void
parse_error (const GError  *error,
	     GFile         *location,
	     const gchar   *uri_for_display,
	     gchar        **primary_text,
	     gchar        **secondary_text)
{
	g_assert (error != NULL);
	g_assert (primary_text != NULL);
	g_assert (secondary_text != NULL);

	*primary_text = NULL;
	*secondary_text = NULL;

	if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND) ||
	    g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_DIRECTORY))
	{
		*primary_text = g_strdup_printf (_("Could not find the file “%s”."),
						 uri_for_display);

		*secondary_text = g_strdup (_("Please check that you typed the "
					      "location correctly and try again."));
	}
	else if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED) &&
		 location != NULL)
	{
		gchar *uri_scheme;

		uri_scheme = g_file_get_uri_scheme (location);

		/* Translators: %s is a URI scheme (like for example
		 * http:, ftp:, etc.).
		 */
		*secondary_text = g_strdup_printf (_("Unable to handle “%s:” locations."),
						   uri_scheme);

		g_free (uri_scheme);
	}
	else if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_MOUNTABLE_FILE) ||
		 g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_MOUNTED))
	{
		*secondary_text = g_strdup (_("The location of the file cannot be accessed."));
	}
	else if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_IS_DIRECTORY))
	{
		*primary_text = g_strdup_printf (_("“%s” is a directory."), uri_for_display);

		*secondary_text = g_strdup (_("Please check that you typed the "
					      "location correctly and try again."));
	}
	else if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_INVALID_FILENAME))
	{
		*primary_text = g_strdup_printf (_("“%s” is not a valid location."), uri_for_display);

		*secondary_text = g_strdup (_("Please check that you typed the "
					      "location correctly and try again."));
	}
	else if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_HOST_NOT_FOUND))
	{
		/* This case can be hit for user-typed strings like "foo" due to
		 * the code that guesses web addresses when there's no initial "/".
		 * But this case is also hit for legitimate web addresses when
		 * the proxy is set up wrong.
		 */
		gchar *uri = NULL;
		gchar *host = NULL;

		if (location != NULL)
		{
			uri = g_file_get_uri (location);
		}

		if (uri != NULL)
		{
			_gtef_utils_decode_uri (uri, NULL, NULL, &host, NULL, NULL);
		}

		if (host != NULL)
		{
			gchar *host_utf8;

			host_utf8 = _gtef_utils_make_valid_utf8 (host);

			/* Translators: %s is a hostname. */
			*secondary_text = g_strdup_printf (_("Host “%s” could not be found. Please check that "
							     "your proxy settings are correct and try again."),
							   host_utf8);

			g_free (host_utf8);
		}
		else
		{
			/* Use the same string as INVALID_HOST. */
			*secondary_text = g_strdup_printf (_("Hostname was invalid. Please check that you "
							     "typed the location correctly and try again."));
		}

		g_free (uri);
		g_free (host);
	}
	else if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_REGULAR_FILE))
	{
		*secondary_text = g_strdup_printf (_("“%s” is not a regular file."),
						   uri_for_display);
	}
	else if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_TIMED_OUT))
	{
		*secondary_text = g_strdup (_("Connection timed out. Please try again."));
	}
	else
	{
		*secondary_text = g_strdup_printf (_("Unexpected error: %s"), error->message);
	}
}

static GtkInfoBar *
create_info_bar (void)
{
	GtkInfoBar *info_bar;
	GtkWidget *action_area;

	info_bar = GTK_INFO_BAR (gtk_info_bar_new ());

	/* Change the buttons orientation to be vertical.
	 * With a small window, if 3 or more buttons are shown horizontally,
	 * there is a ridiculous amount of space for the text. And it can get
	 * worse since the button labels are translatable, in other languages it
	 * can take even more place. If the buttons are packed vertically, there
	 * is no problem.
	 */
	action_area = gtk_info_bar_get_action_area (info_bar);
	if (GTK_IS_ORIENTABLE (action_area))
	{
		gtk_orientable_set_orientation (GTK_ORIENTABLE (action_area),
						GTK_ORIENTATION_VERTICAL);
	}
	else
	{
		g_warning ("Failed to set vertical orientation to the GtkInfoBar action area.");
	}

	return info_bar;
}

static void
set_info_bar_content (GtkInfoBar *info_bar,
		      GtkWidget  *content)
{
	GtkWidget *content_area;

	content_area = gtk_info_bar_get_content_area (info_bar);
	gtk_container_add (GTK_CONTAINER (content_area), content);
}

static void
set_info_bar_text (GtkInfoBar  *info_bar,
		   const gchar *primary_text,
		   const gchar *secondary_text)
{
	GtkWidget *vgrid;
	gchar *primary_text_escaped;
	gchar *primary_markup;
	GtkLabel *primary_label;

	g_assert (primary_text != NULL);

	vgrid = gtk_grid_new ();
	gtk_orientable_set_orientation (GTK_ORIENTABLE (vgrid), GTK_ORIENTATION_VERTICAL);

	primary_text_escaped = g_markup_escape_text (primary_text, -1);
	primary_markup = g_strdup_printf ("<b>%s</b>", primary_text_escaped);
	primary_label = _gtef_utils_create_label_for_info_bar ();
	gtk_label_set_markup (primary_label, primary_markup);
	gtk_container_add (GTK_CONTAINER (vgrid),
			   GTK_WIDGET (primary_label));

	if (secondary_text != NULL)
	{
		gchar *secondary_text_escaped;
		gchar *secondary_markup;
		GtkLabel *secondary_label;

		secondary_text_escaped = g_markup_escape_text (secondary_text, -1);
		secondary_markup = g_strdup_printf ("<small>%s</small>", secondary_text_escaped);
		secondary_label = _gtef_utils_create_label_for_info_bar ();
		gtk_label_set_markup (secondary_label, secondary_markup);
		gtk_container_add (GTK_CONTAINER (vgrid),
				   GTK_WIDGET (secondary_label));

		g_free (secondary_text_escaped);
		g_free (secondary_markup);
	}

	gtk_widget_show_all (vgrid);
	set_info_bar_content (info_bar, vgrid);

	g_free (primary_text_escaped);
	g_free (primary_markup);
}

static GtkInfoBar *
create_io_loading_error_info_bar (const gchar *primary_text,
				  const gchar *secondary_text,
				  gboolean     recoverable_error)
{
	GtkInfoBar *info_bar;

	info_bar = create_info_bar ();
	gtk_info_bar_set_message_type (info_bar, GTK_MESSAGE_ERROR);
	gtk_info_bar_add_button (info_bar, _("_Cancel"), GTK_RESPONSE_CANCEL);

	if (recoverable_error)
	{
		gtk_info_bar_add_button (info_bar, _("_Retry"), GTK_RESPONSE_OK);
	}

	set_info_bar_text (info_bar, primary_text, secondary_text);

	return info_bar;
}

static GtkInfoBar *
create_conversion_error_info_bar (const gchar *primary_text,
				  const gchar *secondary_text,
				  gboolean     edit_anyway)
{
	GtkInfoBar *info_bar;

	info_bar = create_info_bar ();

	gtk_info_bar_add_button (info_bar, _("_Retry"), GTK_RESPONSE_OK);

	if (edit_anyway)
	{
		gtk_info_bar_add_button (info_bar, _("Edit Any_way"), GTK_RESPONSE_YES);
		gtk_info_bar_set_message_type (info_bar, GTK_MESSAGE_WARNING);
	}
	else
	{
		gtk_info_bar_set_message_type (info_bar, GTK_MESSAGE_ERROR);
	}

	gtk_info_bar_add_button (info_bar, _("_Cancel"), GTK_RESPONSE_CANCEL);

	set_info_bar_text (info_bar, primary_text, secondary_text);

	/* TODO add combobox to choose encoding. */

	return info_bar;
}

/*
 * _gtef_io_loading_error_info_bar_new:
 * @loader: a #GtkSourceFileLoader.
 * @error: the #GError received with @loader.
 *
 * Returns: the new #GtkInfoBar.
 */
GtkInfoBar *
_gtef_io_loading_error_info_bar_new (GtkSourceFileLoader *loader,
				     const GError        *error)
{
	GFile *location;
	const GtkSourceEncoding *encoding;
	gchar *uri_for_display;
	gchar *primary_text = NULL;
	gchar *secondary_text = NULL;
	gboolean edit_anyway = FALSE;
	gboolean convert_error = FALSE;
	GtkInfoBar *info_bar;

	g_return_val_if_fail (error != NULL, NULL);
	g_return_val_if_fail (error->domain == GTK_SOURCE_FILE_LOADER_ERROR ||
			      error->domain == G_IO_ERROR ||
			      error->domain == G_CONVERT_ERROR, NULL);

	location = gtk_source_file_loader_get_location (loader);
	encoding = gtk_source_file_loader_get_encoding (loader);

	if (location != NULL)
	{
		uri_for_display = g_file_get_parse_name (location);
	}
	else
	{
		/* FIXME ugly. "stdin" should not be hardcoded here. It should
		 * be set to @loader at the place where we know that we are
		 * loading from stdin.
		 */
		uri_for_display = g_strdup ("stdin");
	}

	if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_TOO_MANY_LINKS))
	{
		secondary_text = g_strdup (_("The number of followed links is limited and the "
					     "actual file could not be found within this limit."));
	}
	else if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_PERMISSION_DENIED))
	{
		secondary_text = g_strdup (_("You do not have the permissions necessary to open the file."));
	}
	/* FIXME can the G_IO_ERROR_INVALID_DATA error happen with
	 * GtkSourceFileLoader?
	 */
	else if ((g_error_matches (error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA) && encoding == NULL) ||
		 g_error_matches (error,
				  GTK_SOURCE_FILE_LOADER_ERROR,
				  GTK_SOURCE_FILE_LOADER_ERROR_ENCODING_AUTO_DETECTION_FAILED))
	{
		secondary_text = g_strdup (_("Unable to detect the character encoding.\n"
					     "Please check that you are not trying to open a binary file.\n"
					     "Select a character encoding from the menu and try again."));
		convert_error = TRUE;
	}
	else if (g_error_matches (error,
				  GTK_SOURCE_FILE_LOADER_ERROR,
				  GTK_SOURCE_FILE_LOADER_ERROR_CONVERSION_FALLBACK))
	{
		primary_text = g_strdup_printf (_("There was a problem opening the file “%s”."), uri_for_display);

		secondary_text = g_strdup (_("The file you opened has some invalid characters. "
					     "If you continue editing this file you could corrupt it.\n"
					     "You can also choose another character encoding and try again."));
		edit_anyway = TRUE;
		convert_error = TRUE;
	}
	/* FIXME can the G_IO_ERROR_INVALID_DATA error happen with
	 * GtkSourceFileLoader?
	 */
	else if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA) &&
		 encoding != NULL)
	{
		gchar *encoding_name;

		encoding_name = gtk_source_encoding_to_string (encoding);

		primary_text = g_strdup_printf (_("Could not open the file “%s” using the “%s” character encoding."),
						uri_for_display,
						encoding_name);

		secondary_text = g_strdup (_("Please check that you are not trying to open a binary file.\n"
					     "Select a different character encoding from the menu and try again."));
		convert_error = TRUE;

		g_free (encoding_name);
	}
	else
	{
		parse_error (error, location, uri_for_display, &primary_text, &secondary_text);
	}

	if (primary_text == NULL)
	{
		primary_text = g_strdup_printf (_("Could not open the file “%s”."), uri_for_display);
	}

	if (convert_error)
	{
		info_bar = create_conversion_error_info_bar (primary_text,
							     secondary_text,
							     edit_anyway);
	}
	else
	{
		info_bar = create_io_loading_error_info_bar (primary_text,
							     secondary_text,
							     is_recoverable_error (error));
	}

	g_free (uri_for_display);
	g_free (primary_text);
	g_free (secondary_text);

	return info_bar;
}
