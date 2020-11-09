/* From gedit-utils.c:
 * SPDX-FileCopyrightText: 1998, 1999 - Alex Roberts, Evan Lawrence
 * SPDX-FileCopyrightText: 2000, 2002 - Chema Celorio, Paolo Maggi
 * SPDX-FileCopyrightText: 2003-2005 - Paolo Maggi
 *
 * SPDX-FileCopyrightText: 2016-2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "tepl-utils.h"
#include <string.h>
#include "tepl-application-window.h"
#include "tepl-icu.h"

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

/**
 * tepl_utils_str_replace:
 * @string: a string
 * @search: the search string
 * @replacement: the replacement string
 *
 * Replaces all occurences of @search by @replacement.
 *
 * The function does only one pass, for example:
 * |[
 * tepl_utils_str_replace ("aaaa", "aa", "a");
 * ]|
 * returns "aa", not "a".
 *
 * Returns: A newly allocated string with the replacements. Free with g_free().
 * Since: 4.4
 */
gchar *
tepl_utils_str_replace (const gchar *string,
			const gchar *search,
			const gchar *replacement)
{
	gchar **chunks;
	gchar *ret;

	g_return_val_if_fail (string != NULL, NULL);
	g_return_val_if_fail (search != NULL, NULL);
	g_return_val_if_fail (replacement != NULL, NULL);

	chunks = g_strsplit (string, search, -1);
	if (chunks != NULL && chunks[0] != NULL)
	{
		ret = g_strjoinv (replacement, chunks);
	}
	else
	{
		ret = g_strdup (string);
	}

	g_strfreev (chunks);
	return ret;
}

/**
 * tepl_utils_markup_escape_text:
 * @src: a nul-terminated UTF-8 string.
 *
 * The same as g_markup_escape_text(), but with an implementation that fully
 * supports round-trip integrity. I.e. when #GMarkupParser or any other XML
 * parser will decode/unescape the string, the exact same string as @src will be
 * brought back. As long as @src is a valid UTF-8 string.
 *
 * The other difference with g_markup_escape_text() is that the @length
 * parameter is not present for tepl_utils_markup_escape_text().
 *
 * # g_markup_escape_text() doesn't fully support round-trip integrity
 *
 * In fact, g_markup_escape_text() doesn't escape the tabstop, newline and
 * carriage return characters. And the #GMarkupParser correctly processes
 * whitespace and line endings according to the [XML rules for normalization of
 * line endings and attribute values](https://www.w3.org/TR/xml/#AVNormalize).
 *
 * For example `"\t"` (a tab) after a round-trip through g_markup_escape_text()
 * and #GMarkupParser becomes a simple space.
 *
 * Returns: (transfer full) (nullable): a newly allocated string with the
 * escaped text, or %NULL if @src is not a valid UTF-8 string. Free with
 * g_free() when no longer needed.
 * Since: 5.0
 */
gchar *
tepl_utils_markup_escape_text (const gchar *src)
{
	UChar *src_uchars;
	UTransliterator *trans;
	UChar *dest_uchars = NULL;
	gchar *dest = NULL;

	src_uchars = _tepl_icu_strFromUTF8Simple (src);
	if (src_uchars == NULL)
	{
		return NULL;
	}

	trans = _tepl_icu_trans_open_xml_escape ();
	if (trans == NULL)
	{
		goto out;
	}

	dest_uchars = _tepl_icu_trans_transUCharsSimple (trans, src_uchars);
	if (dest_uchars == NULL)
	{
		goto out;
	}

	dest = _tepl_icu_strToUTF8Simple (dest_uchars);

out:
	g_free (src_uchars);
	g_free (dest_uchars);

	if (trans != NULL)
	{
		utrans_close (trans);
	}

	return dest;
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

		if (pos == NULL || pos[0] == G_DIR_SEPARATOR)
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

	return g_utf8_strdown (filename + pos, -1);
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

/**
 * tepl_utils_replace_home_dir_with_tilde:
 * @filename: the filename.
 *
 * Replaces the home directory with a tilde, if the home directory is present in
 * the @filename.
 *
 * Returns: the new filename. Free with g_free().
 * Since: 4.4
 */
/* This function comes from gedit. */
gchar *
tepl_utils_replace_home_dir_with_tilde (const gchar *filename)
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

/**
 * tepl_utils_decode_uri:
 * @uri: the uri to decode
 * @scheme: (out) (optional): return value pointer for the uri's
 *     scheme (e.g. http, sftp, ...), or %NULL
 * @user: (out) (optional): return value pointer for the uri user info, or %NULL
 * @host: (out) (optional): return value pointer for the uri host, or %NULL
 * @port: (out) (optional): return value pointer for the uri port, or %NULL
 * @path: (out) (optional): return value pointer for the uri path, or %NULL
 *
 * Parse and break an uri apart in its individual components like the uri
 * scheme, user info, host, port and path. The return value pointer can be
 * %NULL to ignore certain parts of the uri. If the function returns %TRUE, then
 * all return value pointers should be freed using g_free().
 *
 * Returns: %TRUE if the uri could be properly decoded, %FALSE otherwise.
 * Since: 5.0
 */
gboolean
tepl_utils_decode_uri (const gchar  *uri,
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

/**
 * tepl_utils_create_parent_directories:
 * @file: a file
 * @cancellable: (nullable): optional #GCancellable object, %NULL to ignore.
 * @error: (out) (optional): a location to a %NULL #GError, or %NULL.
 *
 * Synchronously creates parent directories of @file, so that @file can be
 * saved.
 *
 * Returns: whether the directories are correctly created. %FALSE is returned on
 * error.
 * Since: 5.0
 */
gboolean
tepl_utils_create_parent_directories (GFile         *file,
				      GCancellable  *cancellable,
				      GError       **error)
{
	GFile *parent;
	GError *my_error = NULL;

	g_return_val_if_fail (G_IS_FILE (file), FALSE);
	g_return_val_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	parent = g_file_get_parent (file);

	if (parent == NULL)
	{
		return TRUE;
	}

	g_file_make_directory_with_parents (parent, cancellable, &my_error);
	g_clear_object (&parent);

	if (g_error_matches (my_error, G_IO_ERROR, G_IO_ERROR_EXISTS))
	{
		g_error_free (my_error);
		return TRUE;
	}
	if (my_error != NULL)
	{
		g_propagate_error (error, my_error);
		return FALSE;
	}

	return TRUE;
}

/**
 * tepl_utils_file_query_exists_async:
 * @file: a #GFile.
 * @cancellable: a #GCancellable.
 * @callback: the callback to call when the operation is finished.
 * @user_data: the data to pass to the callback function.
 *
 * The asynchronous version of g_file_query_exists(). When the operation is
 * finished, @callback will be called. You can then call
 * tepl_utils_file_query_exists_finish() to get the result of the operation.
 *
 * Since: 5.0
 */
void
tepl_utils_file_query_exists_async (GFile               *file,
				    GCancellable        *cancellable,
				    GAsyncReadyCallback  callback,
				    gpointer             user_data)
{
	g_file_query_info_async (file,
				 G_FILE_ATTRIBUTE_STANDARD_TYPE,
				 G_FILE_QUERY_INFO_NONE,
				 G_PRIORITY_DEFAULT,
				 cancellable,
				 callback,
				 user_data);
}

/**
 * tepl_utils_file_query_exists_finish:
 * @file: a #GFile.
 * @result: a #GAsyncResult.
 *
 * Finishes the operation started with tepl_utils_file_query_exists_async().
 * There is no output #GError parameter, so you should check if the operation
 * has been cancelled (in which case %FALSE will be returned).
 *
 * Returns: %TRUE if the file exists and the operation hasn't been cancelled,
 * %FALSE otherwise.
 * Since: 5.0
 */
gboolean
tepl_utils_file_query_exists_finish (GFile        *file,
				     GAsyncResult *result)
{
	GFileInfo *info = g_file_query_info_finish (file, result, NULL);

	if (info != NULL)
	{
		g_object_unref (info);
		return TRUE;
	}

	return FALSE;
}

/**
 * tepl_utils_create_close_button:
 *
 * Returns: (transfer floating): a new close button (a #GtkButton).
 * Since: 5.0
 */
GtkWidget *
tepl_utils_create_close_button (void)
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

/**
 * tepl_utils_show_warning_dialog:
 * @parent: (nullable): the #GtkWindow issuing the warning.
 * @format: format string, as with printf().
 * @...: parameters to insert into the format string.
 *
 * Shows a #GtkDialog with the provided warning message.
 *
 * Since: 5.0
 */
void
tepl_utils_show_warning_dialog (GtkWindow   *parent,
				const gchar *format,
				...)
{
	va_list args;
	gchar *str;
	GtkWidget *dialog;
	GtkWindowGroup *window_group = NULL;

	g_return_if_fail (format != NULL);

	if (parent != NULL)
	{
		window_group = gtk_window_get_group (parent);
	}

	va_start (args, format);
	str = g_strdup_vprintf (format, args);
	va_end (args);

	dialog = gtk_message_dialog_new_with_markup (parent,
						     GTK_DIALOG_MODAL |
						     GTK_DIALOG_DESTROY_WITH_PARENT,
						     GTK_MESSAGE_ERROR,
						     GTK_BUTTONS_OK,
						     "%s", str);

	g_free (str);

	if (window_group != NULL)
	{
		gtk_window_group_add_window (window_group, GTK_WINDOW (dialog));
	}

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

	g_signal_connect (dialog,
			  "response",
			  G_CALLBACK (gtk_widget_destroy),
			  NULL);

	gtk_widget_show (dialog);
}

static void
list_box_clear_foreach_cb (GtkWidget *child,
			   gpointer   user_data)
{
	gtk_widget_destroy (child);
}

/**
 * tepl_utils_list_box_clear:
 * @list_box: a #GtkListBox.
 *
 * Removes all rows of @list_box, to obtain an empty #GtkListBox.
 *
 * Since: 6.0
 */
void
tepl_utils_list_box_clear (GtkListBox *list_box)
{
	g_return_if_fail (GTK_IS_LIST_BOX (list_box));

	gtk_container_foreach (GTK_CONTAINER (list_box),
			       list_box_clear_foreach_cb,
			       NULL);
}

/**
 * tepl_utils_list_box_setup_scrolling:
 * @list_box: a #GtkListBox.
 * @scrolled_window: a #GtkScrolledWindow.
 *
 * Setup vertical scrolling between @list_box and @scrolled_window, to be able
 * to use tepl_utils_list_box_scroll_to_row() afterwards.
 *
 * This function is intended to be called only once per #GtkListBox, when
 * initializing the @list_box and @scrolled_window widgets.
 *
 * Since: 6.0
 */
void
tepl_utils_list_box_setup_scrolling (GtkListBox        *list_box,
				     GtkScrolledWindow *scrolled_window)
{
	GtkAdjustment *vadjustment;

	g_return_if_fail (GTK_IS_LIST_BOX (list_box));
	g_return_if_fail (GTK_IS_SCROLLED_WINDOW (scrolled_window));

	vadjustment = gtk_scrolled_window_get_vadjustment (scrolled_window);
	gtk_container_set_focus_vadjustment (GTK_CONTAINER (list_box), vadjustment);
}

/**
 * tepl_utils_list_box_scroll_to_row:
 * @list_box: a #GtkListBox.
 * @row: a #GtkListBoxRow.
 *
 * Scrolls to a specific #GtkListBoxRow.
 *
 * Before using this function, tepl_utils_list_box_setup_scrolling() must have
 * been called.
 *
 * Since: 6.0
 */
void
tepl_utils_list_box_scroll_to_row (GtkListBox    *list_box,
				   GtkListBoxRow *row)
{
	g_return_if_fail (GTK_IS_LIST_BOX (list_box));
	g_return_if_fail (GTK_IS_LIST_BOX_ROW (row));

	gtk_container_set_focus_child (GTK_CONTAINER (list_box), GTK_WIDGET (row));
}

/**
 * tepl_utils_list_box_scroll_to_selected_row:
 * @list_box: a #GtkListBox.
 *
 * Calls tepl_utils_list_box_scroll_to_row() on the row returned by
 * gtk_list_box_get_selected_row(). This function assumes that there is either
 * zero or one selected row.
 *
 * Before using this function, tepl_utils_list_box_setup_scrolling() must have
 * been called.
 *
 * Since: 6.0
 */
void
tepl_utils_list_box_scroll_to_selected_row (GtkListBox *list_box)
{
	GtkListBoxRow *selected_row;

	g_return_if_fail (GTK_IS_LIST_BOX (list_box));

	selected_row = gtk_list_box_get_selected_row (list_box);
	if (selected_row != NULL)
	{
		tepl_utils_list_box_scroll_to_row (list_box, selected_row);
	}
}

/**
 * tepl_utils_list_box_get_row_at_index_with_filter:
 * @list_box: a #GtkListBox.
 * @index: the index of the row, starting at 0. The index is among the filtered
 *   rows only.
 * @filter_func: (scope call): non-%NULL callback function.
 * @user_data: user data passed to @filter_func.
 *
 * This function has the same semantics as gtk_list_box_get_row_at_index(), but
 * it takes into account only the rows for which @filter_func returns %TRUE.
 *
 * As an example, if @index is 0, it returns the first #GtkListBoxRow for which
 * @filter_func returns %TRUE.
 *
 * Returns: (transfer none) (nullable): the child #GtkListBoxRow or %NULL.
 * Since: 6.0
 */
GtkListBoxRow *
tepl_utils_list_box_get_row_at_index_with_filter (GtkListBox           *list_box,
						  gint                  index,
						  GtkListBoxFilterFunc  filter_func,
						  gpointer              user_data)
{
	GList *all_rows;
	GList *l;
	gint remaining_rows_to_find = index + 1;
	GtkListBoxRow *ret = NULL;

	g_return_val_if_fail (GTK_IS_LIST_BOX (list_box), NULL);
	g_return_val_if_fail (filter_func != NULL, NULL);

	if (index < 0)
	{
		return NULL;
	}

	all_rows = gtk_container_get_children (GTK_CONTAINER (list_box));

	for (l = all_rows; l != NULL; l = l->next)
	{
		GtkListBoxRow *cur_row = GTK_LIST_BOX_ROW (l->data);

		if (filter_func (cur_row, user_data))
		{
			remaining_rows_to_find--;

			if (remaining_rows_to_find == 0)
			{
				ret = cur_row;
				break;
			}
		}
	}

	g_list_free (all_rows);
	return ret;
}

/**
 * tepl_utils_list_box_get_filtered_children:
 * @list_box: a #GtkListBox.
 * @filter_func: (scope call): non-%NULL callback function.
 * @user_data: user data passed to @filter_func.
 * @n_filtered_children: (out) (optional): location to store the number of
 *   #GtkListBoxRow's present in the returned array, without counting the
 *   terminating %NULL.
 *
 * Gets an array of all the #GtkListBoxRow childen of @list_box for which
 * @filter_func returns %TRUE. The elements in the array are sorted by
 * increasing index order (as returned by gtk_list_box_row_get_index()).
 *
 * Returns: (array zero-terminated=1) (element-type GtkListBoxRow) (transfer container) (nullable):
 *   a %NULL-terminated array of #GtkListBoxRow objects, or %NULL. Free with
 *   g_free() when no longer needed.
 * Since: 6.0
 */
GtkListBoxRow **
tepl_utils_list_box_get_filtered_children (GtkListBox           *list_box,
					   GtkListBoxFilterFunc  filter_func,
					   gpointer              user_data,
					   gint                 *n_filtered_children)
{
	GPtrArray *filtered_rows;
	GList *all_rows;
	GList *l;

	g_return_val_if_fail (GTK_IS_LIST_BOX (list_box), NULL);
	g_return_val_if_fail (filter_func != NULL, NULL);

	filtered_rows = g_ptr_array_new ();
	all_rows = gtk_container_get_children (GTK_CONTAINER (list_box));

	for (l = all_rows; l != NULL; l = l->next)
	{
		GtkListBoxRow *cur_row = GTK_LIST_BOX_ROW (l->data);

		if (filter_func (cur_row, user_data))
		{
			g_ptr_array_add (filtered_rows, cur_row);
		}
	}

	g_list_free (all_rows);

	if (n_filtered_children != NULL)
	{
		*n_filtered_children = filtered_rows->len;
	}

	/* NULL-terminate the array, must be done *after* setting
	 * *n_filtered_children, of course.
	 */
	g_ptr_array_add (filtered_rows, NULL);

	return (GtkListBoxRow **) g_ptr_array_free (filtered_rows, FALSE);
}

/**
 * tepl_utils_binding_transform_func_smart_bool:
 * @binding: a #GBinding.
 * @from_value: the #GValue containing the value to transform.
 * @to_value: the #GValue in which to store the transformed value.
 * @user_data: data passed to the transform function.
 *
 * A #GBindingTransformFunc to transform between these two #GValue types:
 * - A #GValue of type #gboolean.
 * - A #GValue of type #GVariant, with the #GVariant of type boolean.
 *
 * For convenience, this function works in both directions (hence the “smart”),
 * it introspects the types of @from_value and @to_value.
 *
 * Note that if @from_value and @to_value are of the same #GValue type, this
 * function won't work and you shouldn't use a custom #GBindingTransformFunc in
 * the first place.
 *
 * Returns: %TRUE if the transformation was successful, and %FALSE otherwise.
 * Since: 5.0
 */
gboolean
tepl_utils_binding_transform_func_smart_bool (GBinding     *binding,
					      const GValue *from_value,
					      GValue       *to_value,
					      gpointer      user_data)
{
	g_return_val_if_fail (G_IS_VALUE (from_value), FALSE);
	g_return_val_if_fail (G_IS_VALUE (to_value), FALSE);

	if (G_VALUE_TYPE (from_value) == G_TYPE_BOOLEAN &&
	    G_VALUE_TYPE (to_value) == G_TYPE_VARIANT)
	{
		gboolean bool_value;

		bool_value = g_value_get_boolean (from_value);
		g_value_set_variant (to_value, g_variant_new_boolean (bool_value));

		return TRUE;
	}
	else if (G_VALUE_TYPE (from_value) == G_TYPE_VARIANT &&
		 G_VALUE_TYPE (to_value) == G_TYPE_BOOLEAN)
	{
		GVariant *variant_value;
		gboolean bool_value;

		variant_value = g_value_get_variant (from_value);
		if (variant_value == NULL)
		{
			return FALSE;
		}

		if (!g_variant_type_equal (g_variant_get_type (variant_value), G_VARIANT_TYPE_BOOLEAN))
		{
			return FALSE;
		}

		bool_value = g_variant_get_boolean (variant_value);
		g_value_set_boolean (to_value, bool_value);

		return TRUE;
	}

	return FALSE;
}
