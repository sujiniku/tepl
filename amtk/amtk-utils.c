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

/**
 * SECTION:amtk-utils
 * @title: AmtkUtils
 * @short_description: Utility functions
 *
 * Utility functions.
 */

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

static gboolean
variant_type_equal_null_safe (const GVariantType *type1,
			      const GVariantType *type2)
{
	if (type1 == NULL || type2 == NULL)
	{
		return type1 == NULL && type2 == NULL;
	}

	return g_variant_type_equal (type1, type2);
}

#define AMTK_GVARIANT_PARAM_KEY "amtk-gvariant-param-key"

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
static void
gtk_action_activate_cb (GtkAction *gtk_action,
			GAction   *g_action)
{
	GVariant *param;

	param = g_object_get_data (G_OBJECT (gtk_action), AMTK_GVARIANT_PARAM_KEY);
	g_action_activate (g_action, param);
}

/**
 * amtk_utils_bind_g_action_to_gtk_action:
 * @g_action_map: a #GActionMap.
 * @detailed_g_action_name: a detailed #GAction name; the #GAction must be
 *   present in @g_action_map.
 * @gtk_action_group: a #GtkActionGroup.
 * @gtk_action_name: a #GtkAction name present in @gtk_action_group.
 *
 * Utility function to be able to port an application gradually to #GAction,
 * when #GtkUIManager and #GtkAction are still used. Porting to #GAction should
 * be the first step.
 *
 * For @detailed_g_action_name, see the g_action_parse_detailed_name() function.
 * The `"app."` or `"win."` prefix (or any other #GActionMap prefix) must not be
 * included in @detailed_g_action_name. For example a valid
 * @detailed_g_action_name is `"open"` or `"insert-command::foobar"`.
 *
 * The same #GAction can be bound to several #GtkAction's (with different
 * parameter values for the #GAction), but the reverse is not true, one
 * #GtkAction cannot be bound to several #GAction's.
 *
 * This function:
 * - Calls g_action_activate() when the #GtkAction #GtkAction::activate signal
 *   is emitted.
 * - Binds the #GAction #GAction:enabled property to the #GtkAction
 *   #GtkAction:sensitive property. The binding is done with the
 *   %G_BINDING_BIDIRECTIONAL and %G_BINDING_SYNC_CREATE flags, the source is
 *   the #GAction and the target is the #GtkAction.
 *
 * When using this function, you should set the callback to %NULL in the
 * corresponding #GtkActionEntry.
 *
 * Since: 4.0
 */
void
amtk_utils_bind_g_action_to_gtk_action (GActionMap     *g_action_map,
					const gchar    *detailed_g_action_name,
					GtkActionGroup *gtk_action_group,
					const gchar    *gtk_action_name)
{
	gchar *g_action_name = NULL;
	GVariant *target_value = NULL;
	GAction *g_action;
	GtkAction *gtk_action;
	GError *error = NULL;

	g_return_if_fail (G_IS_ACTION_MAP (g_action_map));
	g_return_if_fail (detailed_g_action_name != NULL);
	g_return_if_fail (GTK_IS_ACTION_GROUP (gtk_action_group));
	g_return_if_fail (gtk_action_name != NULL);

	g_action_parse_detailed_name (detailed_g_action_name,
				      &g_action_name,
				      &target_value,
				      &error);

	/* The doc of g_action_parse_detailed_name() doesn't explain if it
	 * returns a floating ref for the GVariant.
	 */
	if (target_value != NULL &&
	    g_variant_is_floating (target_value))
	{
		g_variant_ref_sink (target_value);
	}

	if (error != NULL)
	{
		g_warning ("Error when parsing detailed GAction name '%s': %s",
			   detailed_g_action_name,
			   error->message);

		g_clear_error (&error);
		goto out;
	}

	g_action = g_action_map_lookup_action (g_action_map, g_action_name);
	if (g_action == NULL)
	{
		g_warn_if_reached ();
		goto out;
	}

	/* Sanity check, ensure that the GVariant target has the good type. */
	{
		const GVariantType *g_action_param_type;
		const GVariantType *target_value_type = NULL;

		g_action_param_type = g_action_get_parameter_type (g_action);

		if (target_value != NULL)
		{
			target_value_type = g_variant_get_type (target_value);
		}

		if (!variant_type_equal_null_safe (g_action_param_type, target_value_type))
		{
			g_warn_if_reached ();
			goto out;
		}
	}

	gtk_action = gtk_action_group_get_action (gtk_action_group, gtk_action_name);
	if (gtk_action == NULL)
	{
		g_warn_if_reached ();
		goto out;
	}

	if (target_value != NULL)
	{
		g_object_set_data_full (G_OBJECT (gtk_action),
					AMTK_GVARIANT_PARAM_KEY,
					g_variant_ref (target_value),
					(GDestroyNotify)g_variant_unref);
	}

	g_signal_connect_object (gtk_action,
				 "activate",
				 G_CALLBACK (gtk_action_activate_cb),
				 g_action,
				 0);

	g_object_bind_property (g_action, "enabled",
				gtk_action, "sensitive",
				G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

out:
	g_free (g_action_name);
	g_clear_pointer (&target_value, (GDestroyNotify)g_variant_unref);
}
G_GNUC_END_IGNORE_DEPRECATIONS
