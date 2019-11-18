/*
 * This file is part of Tepl, a text editor library.
 *
 * From gedit-utils.c:
 * Copyright 1998, 1999 - Alex Roberts, Evan Lawrence
 * Copyright 2000, 2002 - Chema Celorio, Paolo Maggi
 * Copyright 2003-2005 - Paolo Maggi
 *
 * Copyright 2016, 2017 - Sébastien Wilmet <swilmet@gnome.org>
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

#include "tepl-utils.h"
#include <string.h>
#include "tepl-application-window.h"

/**
 * SECTION:utils
 * @Short_description: Utility functions
 * @Title: TeplUtils
 *
 * Utility functions.
 */

static gchar *
str_truncate (const gchar *string,
	      guint        truncate_length,
	      gboolean     middle)
{
	GString *truncated;
	guint length;
	guint n_chars;
	guint num_left_chars;
	guint right_offset;
	guint delimiter_length;
	const gchar *delimiter = "\342\200\246"; /* The character: … */

	g_return_val_if_fail (string != NULL, NULL);

	length = strlen (string);

	g_return_val_if_fail (g_utf8_validate (string, length, NULL), NULL);

	/* It doesnt make sense to truncate strings to less than
	 * the size of the delimiter plus 2 characters (one on each
	 * side)
	 */
	delimiter_length = g_utf8_strlen (delimiter, -1);
	if (truncate_length < (delimiter_length + 2))
	{
		return g_strdup (string);
	}

	n_chars = g_utf8_strlen (string, length);

	/* Make sure the string is not already small enough. */
	if (n_chars <= truncate_length)
	{
		return g_strdup (string);
	}

	/* Find the 'middle' where the truncation will occur. */
	if (middle)
	{
		num_left_chars = (truncate_length - delimiter_length) / 2;
		right_offset = n_chars - truncate_length + num_left_chars + delimiter_length;

		truncated = g_string_new_len (string,
					      g_utf8_offset_to_pointer (string, num_left_chars) - string);
		g_string_append (truncated, delimiter);
		g_string_append (truncated, g_utf8_offset_to_pointer (string, right_offset));
	}
	else
	{
		num_left_chars = truncate_length - delimiter_length;
		truncated = g_string_new_len (string,
					      g_utf8_offset_to_pointer (string, num_left_chars) - string);
		g_string_append (truncated, delimiter);
	}

	return g_string_free (truncated, FALSE);
}

/**
 * tepl_utils_str_middle_truncate:
 * @str: a UTF-8 string.
 * @truncate_length: truncate the string at that length, in UTF-8 characters
 *   (not bytes).
 *
 * If @str is longer than @truncate_length, then this function returns @str
 * truncated in the middle with a “…” character. Otherwise it just returns a
 * copy of @str.
 *
 * Returns: the truncated string. Free with g_free().
 * Since: 4.4
 */
gchar *
tepl_utils_str_middle_truncate (const gchar *str,
				guint        truncate_length)
{
	return str_truncate (str, truncate_length, TRUE);
}

/**
 * tepl_utils_str_end_truncate:
 * @str: a UTF-8 string.
 * @truncate_length: truncate the string at that length, in UTF-8 characters
 *   (not bytes).
 *
 * Like tepl_utils_str_middle_truncate() but the “…” character is at the end.
 *
 * Returns: the truncated string. Free with g_free().
 * Since: 4.4
 */
gchar *
tepl_utils_str_end_truncate (const gchar *str,
			     guint        truncate_length)
{
	return str_truncate (str, truncate_length, FALSE);
}

static gint
get_extension_position (const gchar *filename)
{
	const gchar *pos;
	gint length;

	if (filename == NULL)
	{
		return 0;
	}

	length = strlen (filename);
	pos = filename + length;
	g_assert (pos[0] == '\0');

	while (TRUE)
	{
		pos = g_utf8_find_prev_char (filename, pos);

		if (pos == NULL || pos[0] == '/')
		{
			break;
		}

		if (pos[0] == '.')
		{
			return pos - filename;
		}
	}

	return length;
}

/**
 * tepl_utils_get_file_extension:
 * @filename: a filename.
 *
 * Examples:
 * - "file.pdf" returns ".pdf".
 * - "file.PDF" returns ".pdf".
 * - "file.tar.gz" returns ".gz".
 * - "path/to/file.pdf" returns ".pdf".
 * - "file" (without an extension) returns "" (the empty string).
 *
 * Returns: the @filename's extension with the dot, in lowercase. Free with
 * g_free().
 * Since: 4.4
 */
gchar *
tepl_utils_get_file_extension (const gchar *filename)
{
	gint pos = get_extension_position (filename);

	return g_ascii_strdown (filename + pos, -1);
}

/**
 * tepl_utils_get_file_shortname:
 * @filename: a filename.
 *
 * Returns @filename without its extension. With the “extension” having the same
 * definition as in tepl_utils_get_file_extension(); in other words it returns
 * the other part of @filename.
 *
 * Returns: the @filename without its extension. Free with g_free().
 * Since: 4.4
 */
gchar *
tepl_utils_get_file_shortname (const gchar *filename)
{
	return g_strndup (filename, get_extension_position (filename));
}

/*
 * _tepl_utils_replace_home_dir_with_tilde:
 * @filename: the filename.
 *
 * Replaces the home directory with a tilde, if the home directory is present in
 * the @filename.
 *
 * Returns: the new filename. Free with g_free().
 */
/* This function comes from gedit. */
gchar *
_tepl_utils_replace_home_dir_with_tilde (const gchar *filename)
{
	gchar *tmp;
	gchar *home;

	g_return_val_if_fail (filename != NULL, NULL);

	/* Note that g_get_home_dir returns a const string */
	tmp = (gchar *) g_get_home_dir ();

	if (tmp == NULL)
	{
		return g_strdup (filename);
	}

	home = g_filename_to_utf8 (tmp, -1, NULL, NULL, NULL);
	if (home == NULL)
	{
		return g_strdup (filename);
	}

	if (g_str_equal (filename, home))
	{
		g_free (home);
		return g_strdup ("~");
	}

	tmp = home;
	home = g_strdup_printf ("%s/", tmp);
	g_free (tmp);

	if (g_str_has_prefix (filename, home))
	{
		gchar *res = g_strdup_printf ("~/%s", filename + strlen (home));
		g_free (home);
		return res;
	}

	g_free (home);
	return g_strdup (filename);
}

static void
null_ptr (gchar **ptr)
{
	if (ptr != NULL)
	{
		*ptr = NULL;
	}
}

/*
 * _tepl_utils_decode_uri:
 * @uri: the uri to decode
 * @scheme: (out) (optional): return value pointer for the uri's
 *     scheme (e.g. http, sftp, ...), or %NULL
 * @user: (out) (optional): return value pointer for the uri user info, or %NULL
 * @port: (out) (optional): return value pointer for the uri port, or %NULL
 * @host: (out) (optional): return value pointer for the uri host, or %NULL
 * @path: (out) (optional): return value pointer for the uri path, or %NULL
 *
 * Parse and break an uri apart in its individual components like the uri
 * scheme, user info, port, host and path. The return value pointer can be
 * %NULL to ignore certain parts of the uri. If the function returns %TRUE, then
 * all return value pointers should be freed using g_free().
 *
 * Returns: %TRUE if the uri could be properly decoded, %FALSE otherwise.
 */
gboolean
_tepl_utils_decode_uri (const gchar  *uri,
			gchar       **scheme,
			gchar       **user,
			gchar       **host,
			gchar       **port,
			gchar       **path)
{
	/* Largely copied from glib/gio/gdummyfile.c: _g_decode_uri().
	 * This functionality is currently not in GLib/GIO, so for now we
	 * implement it ourselves. See:
	 * https://bugzilla.gnome.org/show_bug.cgi?id=555490
	 */

	const char *p, *in, *hier_part_start, *hier_part_end;
	char *out;
	char c;

	/* From RFC 3986 Decodes:
	 * URI = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
	 */

	p = uri;

	null_ptr (scheme);
	null_ptr (user);
	null_ptr (port);
	null_ptr (host);
	null_ptr (path);

	/* Decode scheme:
	 * scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
	 */

	if (!g_ascii_isalpha (*p))
		return FALSE;

	while (1)
	{
		c = *p++;

		if (c == ':')
			break;

		if (!(g_ascii_isalnum(c) ||
		      c == '+' ||
		      c == '-' ||
		      c == '.'))
		{
			return FALSE;
		}
	}

	if (scheme)
	{
		*scheme = g_malloc (p - uri);
		out = *scheme;

		for (in = uri; in < p - 1; in++)
		{
			*out++ = g_ascii_tolower (*in);
		}

		*out = '\0';
	}

	hier_part_start = p;
	hier_part_end = p + strlen (p);

	if (hier_part_start[0] == '/' && hier_part_start[1] == '/')
	{
		const char *authority_start, *authority_end;
		const char *userinfo_start, *userinfo_end;
		const char *host_start, *host_end;
		const char *port_start;

		authority_start = hier_part_start + 2;
		/* authority is always followed by / or nothing */
		authority_end = memchr (authority_start, '/', hier_part_end - authority_start);

		if (authority_end == NULL)
			authority_end = hier_part_end;

		/* 3.2:
		 * authority = [ userinfo "@" ] host [ ":" port ]
		 */

		userinfo_end = memchr (authority_start, '@', authority_end - authority_start);

		if (userinfo_end)
		{
			userinfo_start = authority_start;

			if (user)
				*user = g_uri_unescape_segment (userinfo_start, userinfo_end, NULL);

			if (user && *user == NULL)
			{
				if (scheme)
					g_free (*scheme);

				return FALSE;
			}

			host_start = userinfo_end + 1;
		}
		else
		{
			host_start = authority_start;
		}

		port_start = memchr (host_start, ':', authority_end - host_start);

		if (port_start)
		{
			host_end = port_start++;

			if (port)
				*port = g_strndup (port_start, authority_end - port_start);
		}
		else
		{
			host_end = authority_end;
		}

		if (host)
			*host = g_strndup (host_start, host_end - host_start);

		hier_part_start = authority_end;
	}

	if (path)
		*path = g_uri_unescape_segment (hier_part_start, hier_part_end, "/");

	return TRUE;
}

/**
 * _tepl_utils_get_fallback_basename_for_display:
 * @location: a #GFile.
 *
 * If querying the %G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME fails, this function
 * can be used as a fallback.
 *
 * Returns: (transfer full): the @location's basename suitable for display.
 */
gchar *
_tepl_utils_get_fallback_basename_for_display (GFile *location)
{
	gchar *basename;
	gchar *parse_name;

	g_return_val_if_fail (G_IS_FILE (location), NULL);

	if (g_file_has_uri_scheme (location, "file"))
	{
		gchar *local_path;

		local_path = g_file_get_path (location);
		basename = g_filename_display_basename (local_path);
		g_free (local_path);

		return basename;
	}

	if (!g_file_has_parent (location, NULL))
	{
		return g_file_get_parse_name (location);
	}

	parse_name = g_file_get_parse_name (location);
	basename = g_filename_display_basename (parse_name);
	g_free (parse_name);

	/* FIXME: maybe needed:
	 * basename_unescaped = g_uri_unescape_string (basename, NULL);
	 */

	return basename;
}

GtkWidget *
_tepl_utils_create_close_button (void)
{
	GtkWidget *close_button;
	GtkStyleContext *style_context;

	close_button = gtk_button_new_from_icon_name ("window-close-symbolic",
						      GTK_ICON_SIZE_BUTTON);
	gtk_button_set_relief (GTK_BUTTON (close_button), GTK_RELIEF_NONE);
	gtk_widget_set_focus_on_click (close_button, FALSE);

	style_context = gtk_widget_get_style_context (close_button);
	gtk_style_context_add_class (style_context, GTK_STYLE_CLASS_FLAT);

	return close_button;
}

/* For a secondary window (e.g. a GtkDialog):
 * - Set transient parent.
 * - Add it to the GtkWindowGroup.
 * Just by giving a widget inside the main window.
 */
void
_tepl_utils_associate_secondary_window (GtkWindow *secondary_window,
					GtkWidget *main_window_widget)
{
	GtkWidget *toplevel;
	GtkWindow *main_window = NULL;

	g_return_if_fail (GTK_IS_WINDOW (secondary_window));
	g_return_if_fail (GTK_IS_WIDGET (main_window_widget));

	/* gtk_widget_get_toplevel() is a bit evil, normally it's a bad practice
	 * when an object is aware of who contains it, i.e. it's fine that a
	 * container knows what it contains (of course) but the reverse is not
	 * true.
	 *
	 * But here it's just to setup correctly e.g. a GtkDialog, it's
	 * something a bit specific to GTK. As long as this bad practice is
	 * applied only in this case (setting the transient parent and adding
	 * the secondary window to a GtkWindowGroup), it should be fine. It
	 * would be more problematic to call other TeplApplicationWindow
	 * functions.
	 */
	toplevel = gtk_widget_get_toplevel (main_window_widget);
	if (gtk_widget_is_toplevel (toplevel))
	{
		main_window = GTK_WINDOW (toplevel);
	}

	if (main_window != NULL)
	{
		gtk_window_set_transient_for (secondary_window, main_window);
	}

	if (GTK_IS_APPLICATION_WINDOW (main_window) &&
	    tepl_application_window_is_main_window (GTK_APPLICATION_WINDOW (main_window)))
	{
		TeplApplicationWindow *tepl_window;
		GtkWindowGroup *window_group;

		tepl_window = tepl_application_window_get_from_gtk_application_window (GTK_APPLICATION_WINDOW (main_window));

		window_group = tepl_application_window_get_window_group (tepl_window);
		gtk_window_group_add_window (window_group, secondary_window);
	}
}
