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

#ifndef TEPL_TAB_LIST_H
#define TEPL_TAB_LIST_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <glib-object.h>
#include <tepl/tepl-types.h>

G_BEGIN_DECLS

#define TEPL_TYPE_TAB_LIST               (tepl_tab_list_get_type ())
#define TEPL_TAB_LIST(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_TAB_LIST, TeplTabList))
#define TEPL_IS_TAB_LIST(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_TAB_LIST))
#define TEPL_TAB_LIST_GET_INTERFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), TEPL_TYPE_TAB_LIST, TeplTabListInterface))

typedef struct _TeplTabListInterface TeplTabListInterface;

/**
 * TeplTabListInterface:
 * @parent_interface: The parent interface.
 * @get_tabs: Virtual function pointer for tepl_tab_list_get_tabs(). By default,
 *   %NULL is returned.
 * @get_active_tab: Virtual function pointer for tepl_tab_list_get_active_tab().
 *   By default, %NULL is returned.
 *
 * The virtual function table for #TeplTabList.
 *
 * Since: 3.0
 */
struct _TeplTabListInterface
{
	GTypeInterface parent_interface;

	GList *		(*get_tabs)		(TeplTabList *tab_list);

	TeplTab *	(*get_active_tab)	(TeplTabList *tab_list);
};

GType		tepl_tab_list_get_type		(void);

GList *		tepl_tab_list_get_tabs		(TeplTabList *tab_list);

GList *		tepl_tab_list_get_views		(TeplTabList *tab_list);

GList *		tepl_tab_list_get_buffers	(TeplTabList *tab_list);

TeplTab *	tepl_tab_list_get_active_tab	(TeplTabList *tab_list);

TeplView *	tepl_tab_list_get_active_view	(TeplTabList *tab_list);

TeplBuffer *	tepl_tab_list_get_active_buffer	(TeplTabList *tab_list);

G_END_DECLS

#endif /* TEPL_TAB_LIST_H */
