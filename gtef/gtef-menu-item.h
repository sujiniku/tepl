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

#ifndef GTEF_MENU_ITEM_H
#define GTEF_MENU_ITEM_H

#if !defined (GTEF_H_INSIDE) && !defined (GTEF_COMPILATION)
#error "Only <gtef/gtef.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

const gchar *	gtef_menu_item_get_long_description	(GtkMenuItem *menu_item);

void		gtef_menu_item_set_long_description	(GtkMenuItem *menu_item,
							 const gchar *long_description);

void		gtef_menu_item_set_icon_name		(GtkMenuItem *item,
							 const gchar *icon_name);

G_END_DECLS

#endif  /* GTEF_MENU_ITEM_H */
