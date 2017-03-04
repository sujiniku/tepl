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
