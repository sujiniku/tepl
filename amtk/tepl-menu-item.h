/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2017 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef TEPL_MENU_ITEM_H
#define TEPL_MENU_ITEM_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

const gchar *	tepl_menu_item_get_long_description	(GtkMenuItem *menu_item);

void		tepl_menu_item_set_long_description	(GtkMenuItem *menu_item,
							 const gchar *long_description);

void		tepl_menu_item_set_icon_name		(GtkMenuItem *item,
							 const gchar *icon_name);

G_END_DECLS

#endif  /* TEPL_MENU_ITEM_H */
