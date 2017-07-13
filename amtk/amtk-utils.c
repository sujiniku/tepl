/*
 * This file is part of Amtk - Actions, Menus and Toolbars Kit
 *
 * Copyright 2017 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * Amtk is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * Amtk is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "amtk-utils.h"
#include <string.h>

/*
 * _amtk_utils_replace_home_dir_with_tilde:
 * @filename: the filename.
 *
 * Replaces the home directory with a tilde, if the home directory is present in
 * the @filename.
 *
 * Returns: the new filename. Free with g_free().
 */
/* This function is a copy from tepl-utils, which originally comes from gedit. */
gchar *
_amtk_utils_replace_home_dir_with_tilde (const gchar *filename)
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

/* Deep copy of @strv. */
gchar **
_amtk_utils_strv_copy (const gchar * const *strv)
{
	guint length;
	gchar **new_strv;
	guint i;

	if (strv == NULL)
	{
		return NULL;
	}

	length = g_strv_length ((gchar **)strv);

	new_strv = g_malloc ((length + 1) * sizeof (gchar *));

	for (i = 0; i < length; i++)
	{
		new_strv[i] = g_strdup (strv[i]);
	}

	new_strv[length] = NULL;

	return new_strv;
}

static gint
get_menu_item_position (GtkMenuShell *menu_shell,
			GtkMenuItem  *item)
{
	GList *children;
	GList *l;
	gint pos;
	gboolean found = FALSE;

	children = gtk_container_get_children (GTK_CONTAINER (menu_shell));

	for (l = children, pos = 0; l != NULL; l = l->next, pos++)
	{
		GtkMenuItem *cur_item = l->data;

		if (cur_item == item)
		{
			found = TRUE;
			break;
		}
	}

	g_list_free (children);

	return found ? pos : -1;
}

/**
 * amtk_utils_recent_chooser_menu_get_item_uri:
 * @menu: a #GtkRecentChooserMenu.
 * @item: a #GtkMenuItem.
 *
 * Gets the URI of @item. @item must be a child of @menu. @menu must be a
 * #GtkRecentChooserMenu.
 *
 * This function has been written because the value returned by
 * gtk_recent_chooser_get_current_uri() is not updated when #GtkMenuItem's of a
 * #GtkRecentChooserMenu are selected/deselected.
 *
 * Returns: the URI of @item. Free with g_free() when no longer needed.
 * Since: 2.0
 */
gchar *
amtk_utils_recent_chooser_menu_get_item_uri (GtkRecentChooserMenu *menu,
					     GtkMenuItem          *item)
{
	gint pos;
	gchar **all_uris;
	gsize length;
	gchar *item_uri = NULL;

	g_return_val_if_fail (GTK_IS_RECENT_CHOOSER_MENU (menu), NULL);
	g_return_val_if_fail (GTK_IS_MENU_ITEM (item), NULL);

	{
		GtkWidget *item_parent;

		item_parent = gtk_widget_get_parent (GTK_WIDGET (item));
		g_return_val_if_fail (item_parent == GTK_WIDGET (menu), NULL);
	}

	pos = get_menu_item_position (GTK_MENU_SHELL (menu), item);
	g_return_val_if_fail (pos >= 0, NULL);

	all_uris = gtk_recent_chooser_get_uris (GTK_RECENT_CHOOSER (menu), &length);

	if ((gsize)pos < length)
	{
		item_uri = g_strdup (all_uris[pos]);
	}

	g_strfreev (all_uris);
	return item_uri;
}
