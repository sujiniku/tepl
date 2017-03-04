/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2017 - Sébastien Wilmet <swilmet@gnome.org>
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

#include "gtef-menu-item.h"

/**
 * SECTION:menu-item
 * @Short_description: GtkMenuItem functions
 * @Title: GtefMenuItem
 *
 * #GtkMenuItem functions.
 */

#define LONG_DESCRIPTION_KEY "gtef-menu-item-long-description-key"

/**
 * gtef_menu_item_get_long_description:
 * @menu_item: a #GtkMenuItem.
 *
 * Returns: (nullable): the long description of @menu_item, previously set with
 *   gtef_menu_item_set_long_description().
 * Since: 2.0
 */
const gchar *
gtef_menu_item_get_long_description (GtkMenuItem *menu_item)
{
	g_return_val_if_fail (GTK_IS_MENU_ITEM (menu_item), NULL);

	return g_object_get_data (G_OBJECT (menu_item), LONG_DESCRIPTION_KEY);
}

/**
 * gtef_menu_item_set_long_description:
 * @menu_item: a #GtkMenuItem.
 * @long_description: (nullable): the long description, or %NULL to unset it.
 *
 * Sets the long description of @menu_item. A possible use-case is to display it
 * in a #GtkStatusbar, or as a tooltip.
 *
 * Since: 2.0
 */
void
gtef_menu_item_set_long_description (GtkMenuItem *menu_item,
				     const gchar *long_description)
{
	g_return_if_fail (GTK_IS_MENU_ITEM (menu_item));

	g_object_set_data_full (G_OBJECT (menu_item),
				LONG_DESCRIPTION_KEY,
				g_strdup (long_description),
				g_free);
}

/**
 * gtef_menu_item_set_icon_name:
 * @item: a #GtkMenuItem.
 * @icon_name: an icon name.
 *
 * Sets an icon to a #GtkMenuItem.
 *
 * Since: 2.0
 */
/* Based on gtk_model_menu_item_set_icon() from gtkmodelmenuitem.c (private
 * GTK+ class).
 * Copyright 2011, 2013 Canonical Limited
 */
void
gtef_menu_item_set_icon_name (GtkMenuItem *item,
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
