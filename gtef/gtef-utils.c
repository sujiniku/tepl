/*
 * This file is part of Gtef, a text editor library.
 *
 * From gedit-utils.c:
 * Copyright 1998, 1999 - Alex Roberts, Evan Lawrence
 * Copyright 2000, 2002 - Chema Celorio, Paolo Maggi
 * Copyright 2003-2005 - Paolo Maggi
 *
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * Gtef is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * Gtef is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "gtef-utils.h"
#include <string.h>
#include <glib/gi18n-lib.h>

/**
 * SECTION:utils
 * @title: GtefUtils
 * @short_description: Utilities functions
 *
 * Utilities functions.
 */

/*
 * _gtef_utils_replace_home_dir_with_tilde:
 * @filename: the filename.
 *
 * Replaces the home directory with a tilde, if the home directory is present in
 * the @filename.
 *
 * Returns: the new filename. Free with g_free().
 */
/* This function comes from gedit. */
gchar *
_gtef_utils_replace_home_dir_with_tilde (const gchar *filename)
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
 * _gtef_utils_decode_uri:
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
_gtef_utils_decode_uri (const gchar  *uri,
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
 * _gtef_utils_get_fallback_basename_for_display:
 * @location: a #GFile.
 *
 * If querying the %G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME fails, this function
 * can be used as a fallback.
 *
 * Returns: (transfer full): the @location's basename suitable for display.
 */
gchar *
_gtef_utils_get_fallback_basename_for_display (GFile *location)
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

gchar *
_gtef_utils_make_valid_utf8 (const gchar *_str)
{
	const gchar *remainder;
	gint remaining_bytes;
	gint valid_bytes;
	GString *string = NULL;

	g_return_val_if_fail (_str != NULL, NULL);

	remainder = _str;
	remaining_bytes = strlen (_str);

	while (remaining_bytes != 0)
	{
		const gchar *invalid;

		if (g_utf8_validate (remainder, remaining_bytes, &invalid))
		{
			break;
		}

		valid_bytes = invalid - remainder;

		if (string == NULL)
		{
			string = g_string_sized_new (remaining_bytes);
		}

		g_string_append_len (string, remainder, valid_bytes);

		/* Append U+FFFD REPLACEMENT CHARACTER. */
		g_string_append (string, "\357\277\275");

		remaining_bytes -= valid_bytes + 1;
		remainder = invalid + 1;
	}

	if (string == NULL)
	{
		return g_strdup (_str);
	}

	g_string_append (string, remainder);

	g_assert (g_utf8_validate (string->str, -1, NULL));

	return g_string_free (string, FALSE);
}

/**
 * gtef_utils_menu_item_set_icon_name:
 * @item: a #GtkMenuItem.
 * @icon_name: an icon name.
 *
 * Sets an icon to a #GtkMenuItem.
 *
 * Since: 1.2
 */
/* Based on gtk_model_menu_item_set_icon() from gtkmodelmenuitem.c (private
 * GTK+ class).
 * Copyright 2011, 2013 Canonical Limited
 */
void
gtef_utils_menu_item_set_icon_name (GtkMenuItem *item,
				    const gchar *icon_name)
{
	GtkWidget *child;

	g_return_if_fail (GTK_IS_MENU_ITEM (item));

	child = gtk_bin_get_child (GTK_BIN (item));

	/* There are only three possibilities here:
	 *
	 *   - no child
	 *   - accel label child
	 *   - already a box
	 *
	 * Handle the no-child case by having GtkMenuItem create the accel
	 * label, then we will only have two possible cases.
	 */
	if (child == NULL)
	{
		gtk_menu_item_get_label (item);
		child = gtk_bin_get_child (GTK_BIN (item));
		g_return_if_fail (GTK_IS_LABEL (child));
	}

	/* If it is a box, make sure there are no images inside of it already. */
	if (GTK_IS_BOX (child))
	{
		GList *children;

		children = gtk_container_get_children (GTK_CONTAINER (child));
		while (children != NULL)
		{
			if (GTK_IS_IMAGE (children->data))
			{
				gtk_widget_destroy (children->data);
			}

			children = g_list_delete_link (children, children);
		}
	}
	else
	{
		GtkWidget *box;

		if (icon_name == NULL)
		{
			return;
		}

		box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);

		/* Reparent the child without destroying it */
		g_object_ref (child);
		gtk_container_remove (GTK_CONTAINER (item), child);
		gtk_box_pack_end (GTK_BOX (box), child, TRUE, TRUE, 0);
		g_object_unref (child);

		gtk_container_add (GTK_CONTAINER (item), box);
		gtk_widget_show (box);

		/* Now we have a box */
		child = box;
	}

	g_assert (GTK_IS_BOX (child));

	/* child is now a box containing a label and no image. Add the icon,
	 * if appropriate.
	 */
	if (icon_name != NULL)
	{
		GtkWidget *image;

		image = gtk_image_new_from_icon_name (icon_name, GTK_ICON_SIZE_MENU);
		gtk_image_set_pixel_size (GTK_IMAGE (image), 16);
		gtk_box_pack_start (GTK_BOX (child), image, FALSE, FALSE, 0);
		gtk_widget_show (image);
	}
}
