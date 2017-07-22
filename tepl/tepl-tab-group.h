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

#ifndef TEPL_TAB_GROUP_H
#define TEPL_TAB_GROUP_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <glib-object.h>
#include <tepl/tepl-types.h>

G_BEGIN_DECLS

#define TEPL_TYPE_TAB_GROUP               (tepl_tab_group_get_type ())
#define TEPL_TAB_GROUP(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_TAB_GROUP, TeplTabGroup))
#define TEPL_IS_TAB_GROUP(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_TAB_GROUP))
#define TEPL_TAB_GROUP_GET_INTERFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), TEPL_TYPE_TAB_GROUP, TeplTabGroupInterface))

typedef struct _TeplTabGroupInterface TeplTabGroupInterface;

/**
 * TeplTabGroupInterface:
 * @parent_interface: The parent interface.
 * @get_tabs: Virtual function pointer for tepl_tab_group_get_tabs(). By default,
 *   %NULL is returned. When implementing this vfunc, #GList nodes must be
 *   created only for #TeplTab children; if the #TeplTabGroup contains a
 *   non-#TeplTab child, it must be skipped. See the documentation of
 *   tepl_tab_group_get_tabs().
 * @get_active_tab: Virtual function pointer for tepl_tab_group_get_active_tab().
 *   By default, %NULL is returned.
 * @set_active_tab: Virtual function pointer for
 *   tepl_tab_group_set_active_tab(). Does nothing by default.
 * @append_tab: Virtual function pointer for tepl_tab_group_append_tab(). Does
 *   nothing by default. The @jump_to parameter is already implemented with
 *   tepl_tab_group_set_active_tab().
 *
 * The virtual function table for #TeplTabGroup. When implementing one of the
 * vfunc, you can assume that the pre-conditions are already checked (the
 * parameters are valid).
 *
 * Since: 3.0
 */
struct _TeplTabGroupInterface
{
	GTypeInterface parent_interface;

	GList *		(*get_tabs)		(TeplTabGroup *tab_group);

	TeplTab *	(*get_active_tab)	(TeplTabGroup *tab_group);

	void		(*set_active_tab)	(TeplTabGroup *tab_group,
						 TeplTab      *tab);

	void		(*append_tab)		(TeplTabGroup *tab_group,
						 TeplTab      *tab);
};

GType		tepl_tab_group_get_type			(void);

GList *		tepl_tab_group_get_tabs			(TeplTabGroup *tab_group);

GList *		tepl_tab_group_get_views		(TeplTabGroup *tab_group);

GList *		tepl_tab_group_get_buffers		(TeplTabGroup *tab_group);

TeplTab *	tepl_tab_group_get_active_tab		(TeplTabGroup *tab_group);

void		tepl_tab_group_set_active_tab		(TeplTabGroup *tab_group,
							 TeplTab      *tab);

TeplView *	tepl_tab_group_get_active_view		(TeplTabGroup *tab_group);

TeplBuffer *	tepl_tab_group_get_active_buffer	(TeplTabGroup *tab_group);

void		tepl_tab_group_append_tab		(TeplTabGroup *tab_group,
							 TeplTab      *tab,
							 gboolean      jump_to);

G_END_DECLS

#endif /* TEPL_TAB_GROUP_H */
