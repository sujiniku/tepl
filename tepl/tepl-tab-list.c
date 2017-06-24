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

#include "tepl-tab-list.h"
#include "tepl-tab.h"

/**
 * SECTION:tab-list
 * @Short_description: Interface for a list of #TeplTab's
 * @Title: TeplTabList
 *
 * The tepl_tab_list_get_tabs() function permits to get the list of #TeplTab's.
 * The tepl_tab_list_get_active_tab() function permits to get the #TeplTab
 * currently shown in the #TeplTabList.
 *
 * #TeplTabList also contains convenience functions to get #TeplView's and
 * #TeplBuffer's instead of #TeplTab's.
 */

G_DEFINE_INTERFACE (TeplTabList, tepl_tab_list, G_TYPE_OBJECT)

static GList *
tepl_tab_list_get_tabs_default (TeplTabList *tab_list)
{
	return NULL;
}

static TeplTab *
tepl_tab_list_get_active_tab_default (TeplTabList *tab_list)
{
	return NULL;
}

static void
tepl_tab_list_default_init (TeplTabListInterface *interface)
{
	interface->get_tabs = tepl_tab_list_get_tabs_default;
	interface->get_active_tab = tepl_tab_list_get_active_tab_default;
}

/**
 * tepl_tab_list_get_tabs:
 * @tab_list: a #TeplTabList.
 *
 * Gets the list of #TeplTab's contained in @tab_list.
 *
 * If @tab_list contains non-#TeplTab children, those will not be present in the
 * returned list. In other words, it is <emphasis>not</emphasis> guaranteed that
 * the index of a #TeplTab in the returned #GList has the same child index in
 * the @tab_list container.
 *
 * Returns: (transfer container) (element-type TeplTab): the list of all the
 * #TeplTab's contained in @tab_list.
 * Since: 3.0
 */
GList *
tepl_tab_list_get_tabs (TeplTabList *tab_list)
{
	g_return_val_if_fail (TEPL_IS_TAB_LIST (tab_list), NULL);

	return TEPL_TAB_LIST_GET_INTERFACE (tab_list)->get_tabs (tab_list);
}

/**
 * tepl_tab_list_get_views:
 * @tab_list: a #TeplTabList.
 *
 * Convenience function.
 *
 * Returns: (transfer container) (element-type TeplView): like
 * tepl_tab_list_get_tabs(), but returns #TeplView's.
 * Since: 3.0
 */
GList *
tepl_tab_list_get_views (TeplTabList *tab_list)
{
	GList *tabs;
	GList *views = NULL;
	GList *l;

	g_return_val_if_fail (TEPL_IS_TAB_LIST (tab_list), NULL);

	tabs = tepl_tab_list_get_tabs (tab_list);

	for (l = tabs; l != NULL; l = l->next)
	{
		TeplTab *cur_tab = l->data;
		views = g_list_prepend (views, tepl_tab_get_view (cur_tab));
	}

	views = g_list_reverse (views);

	g_list_free (tabs);
	return views;
}

/**
 * tepl_tab_list_get_buffers:
 * @tab_list: a #TeplTabList.
 *
 * Convenience function.
 *
 * Returns: (transfer container) (element-type TeplBuffer): like
 * tepl_tab_list_get_tabs(), but returns #TeplBuffer's.
 * Since: 3.0
 */
GList *
tepl_tab_list_get_buffers (TeplTabList *tab_list)
{
	GList *tabs;
	GList *buffers = NULL;
	GList *l;

	g_return_val_if_fail (TEPL_IS_TAB_LIST (tab_list), NULL);

	tabs = tepl_tab_list_get_tabs (tab_list);

	for (l = tabs; l != NULL; l = l->next)
	{
		TeplTab *cur_tab = l->data;
		buffers = g_list_prepend (buffers, tepl_tab_get_buffer (cur_tab));
	}

	buffers = g_list_reverse (buffers);

	g_list_free (tabs);
	return buffers;
}

/**
 * tepl_tab_list_get_active_tab:
 * @tab_list: a #TeplTabList.
 *
 * Returns: (transfer none) (nullable): the #TeplTab currently shown in
 * @tab_list.
 * Since: 3.0
 */
TeplTab *
tepl_tab_list_get_active_tab (TeplTabList *tab_list)
{
	g_return_val_if_fail (TEPL_IS_TAB_LIST (tab_list), NULL);

	return TEPL_TAB_LIST_GET_INTERFACE (tab_list)->get_active_tab (tab_list);
}

/**
 * tepl_tab_list_get_active_view:
 * @tab_list: a #TeplTabList.
 *
 * Convenience function.
 *
 * Returns: (transfer none) (nullable): the #TeplView of the active tab.
 * Since: 3.0
 */
TeplView *
tepl_tab_list_get_active_view (TeplTabList *tab_list)
{
	TeplTab *active_tab;

	g_return_val_if_fail (TEPL_IS_TAB_LIST (tab_list), NULL);

	active_tab = tepl_tab_list_get_active_tab (tab_list);

	return active_tab != NULL ? tepl_tab_get_view (active_tab) : NULL;
}

/**
 * tepl_tab_list_get_active_buffer:
 * @tab_list: a #TeplTabList.
 *
 * Convenience function.
 *
 * Returns: (transfer none) (nullable): the #TeplBuffer of the active tab.
 * Since: 3.0
 */
TeplBuffer *
tepl_tab_list_get_active_buffer (TeplTabList *tab_list)
{
	TeplTab *active_tab;

	g_return_val_if_fail (TEPL_IS_TAB_LIST (tab_list), NULL);

	active_tab = tepl_tab_list_get_active_tab (tab_list);

	return active_tab != NULL ? tepl_tab_get_buffer (active_tab) : NULL;
}
