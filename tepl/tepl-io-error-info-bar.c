/* SPDX-FileCopyrightText: 2005 - Paolo Maggi
 * SPDX-FileCopyrightText: 2016 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "config.h"
#include "tepl-io-error-info-bar.h"
#include <glib/gi18n-lib.h>
#include "tepl-utils.h"

struct _TeplIoErrorInfoBarPrivate
{
	gint something;
};

/* Verbose error reporting for file I/O operations (load, save, etc.). */

G_DEFINE_TYPE_WITH_PRIVATE (TeplIoErrorInfoBar, _tepl_io_error_info_bar, TEPL_TYPE_INFO_BAR)

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
			tepl_utils_decode_uri (uri, NULL, NULL, &host, NULL, NULL);
		}

		if (host != NULL)
		{
			gchar *host_utf8;

			host_utf8 = g_utf8_make_valid (host, -1);

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

static void
set_io_loading_error (TeplIoErrorInfoBar *info_bar,
		      gboolean            recoverable_error)
{
	gtk_info_bar_set_message_type (GTK_INFO_BAR (info_bar), GTK_MESSAGE_ERROR);

	gtk_info_bar_add_button (GTK_INFO_BAR (info_bar),
				 _("_Cancel"),
				 GTK_RESPONSE_CANCEL);

	if (recoverable_error)
	{
		gtk_info_bar_add_button (GTK_INFO_BAR (info_bar),
					 _("_Retry"),
					 GTK_RESPONSE_OK);
	}
}

static void
set_conversion_error (TeplIoErrorInfoBar *info_bar,
		      gboolean            edit_anyway)
{
	gtk_info_bar_add_button (GTK_INFO_BAR (info_bar),
				 _("_Retry"),
				 GTK_RESPONSE_OK);

	if (edit_anyway)
	{
		gtk_info_bar_add_button (GTK_INFO_BAR (info_bar),
					 _("Edit Any_way"),
					 GTK_RESPONSE_YES);

		gtk_info_bar_set_message_type (GTK_INFO_BAR (info_bar), GTK_MESSAGE_WARNING);
	}
	else
	{
		gtk_info_bar_set_message_type (GTK_INFO_BAR (info_bar), GTK_MESSAGE_ERROR);
	}

	gtk_info_bar_add_button (GTK_INFO_BAR (info_bar),
				 _("_Cancel"),
				 GTK_RESPONSE_CANCEL);
}

static void
_tepl_io_error_info_bar_class_init (TeplIoErrorInfoBarClass *klass)
{
}

static void
_tepl_io_error_info_bar_init (TeplIoErrorInfoBar *info_bar)
{
	info_bar->priv = _tepl_io_error_info_bar_get_instance_private (info_bar);
}

TeplIoErrorInfoBar *
_tepl_io_error_info_bar_new (void)
{
	return g_object_new (TEPL_TYPE_IO_ERROR_INFO_BAR, NULL);
}

void
_tepl_io_error_info_bar_set_loading_error (TeplIoErrorInfoBar  *info_bar,
					   GtkSourceFileLoader *loader,
					   const GError        *error)
{
	GFile *location;
	const GtkSourceEncoding *encoding;
	gchar *uri_for_display;
	gchar *primary_text = NULL;
	gchar *secondary_text = NULL;
	gboolean edit_anyway = FALSE;
	gboolean convert_error = FALSE;

	g_return_if_fail (TEPL_IS_IO_ERROR_INFO_BAR (info_bar));
	g_return_if_fail (GTK_SOURCE_IS_FILE_LOADER (loader));
	g_return_if_fail (error != NULL);
	g_return_if_fail (error->domain == GTK_SOURCE_FILE_LOADER_ERROR ||
			  error->domain == G_IO_ERROR ||
			  error->domain == G_CONVERT_ERROR);

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
		set_conversion_error (info_bar, edit_anyway);
	}
	else
	{
		set_io_loading_error (info_bar, is_recoverable_error (error));
	}

	tepl_info_bar_add_primary_message (TEPL_INFO_BAR (info_bar),
					   primary_text);

	if (secondary_text != NULL)
	{
		tepl_info_bar_add_secondary_message (TEPL_INFO_BAR (info_bar),
						     secondary_text);
	}

	g_free (uri_for_display);
	g_free (primary_text);
	g_free (secondary_text);
}
