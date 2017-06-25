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

#include "tepl-tab-group.h"
#include "tepl-tab.h"

/**
 * SECTION:tab-group
 * @Short_description: Interface for a group of #TeplTab's
 * @Title: TeplTabGroup
 *
 * The tepl_tab_group_get_tabs() function permits to get the list of #TeplTab's.
 * The tepl_tab_group_get_active_tab() function permits to get the #TeplTab
 * currently shown in the #TeplTabGroup.
 *
 * #TeplTabGroup also contains convenience functions to get #TeplView's and
 * #TeplBuffer's instead of #TeplTab's.
 */

G_DEFINE_INTERFACE (TeplTabGroup, tepl_tab_group, G_TYPE_OBJECT)

static GList *
tepl_tab_group_get_tabs_default (TeplTabGroup *tab_group)
{
	return NULL;
}

static TeplTab *
tepl_tab_group_get_active_tab_default (TeplTabGroup *tab_group)
{
	return NULL;
}

static void
tepl_tab_group_default_init (TeplTabGroupInterface *interface)
{
	interface->get_tabs = tepl_tab_group_get_tabs_default;
	interface->get_active_tab = tepl_tab_group_get_active_tab_default;
}

/**
 * tepl_tab_group_get_tabs:
 * @tab_group: a #TeplTabGroup.
 *
 * Gets the list of #TeplTab's contained in @tab_group.
 *
 * If @tab_group contains non-#TeplTab children, those will not be present in the
 * returned list. In other words, it is <emphasis>not</emphasis> guaranteed that
 * the index of a #TeplTab in the returned #GList has the same child index in
 * the @tab_group container.
 *
 * Returns: (transfer container) (element-type TeplTab): the list of all the
 * #TeplTab's contained in @tab_group.
 * Since: 3.0
 */
GList *
tepl_tab_group_get_tabs (TeplTabGroup *tab_group)
{
	g_return_val_if_fail (TEPL_IS_TAB_GROUP (tab_group), NULL);

	return TEPL_TAB_GROUP_GET_INTERFACE (tab_group)->get_tabs (tab_group);
}

/**
 * tepl_tab_group_get_views:
 * @tab_group: a #TeplTabGroup.
 *
 * Convenience function.
 *
 * Returns: (transfer container) (element-type TeplView): like
 * tepl_tab_group_get_tabs(), but returns #TeplView's.
 * Since: 3.0
 */
GList *
tepl_tab_group_get_views (TeplTabGroup *tab_group)
{
	GList *tabs;
	GList *views = NULL;
	GList *l;

	g_return_val_if_fail (TEPL_IS_TAB_GROUP (tab_group), NULL);

	tabs = tepl_tab_group_get_tabs (tab_group);

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
 * tepl_tab_group_get_buffers:
 * @tab_group: a #TeplTabGroup.
 *
 * Convenience function.
 *
 * Returns: (transfer container) (element-type TeplBuffer): like
 * tepl_tab_group_get_tabs(), but returns #TeplBuffer's.
 * Since: 3.0
 */
GList *
tepl_tab_group_get_buffers (TeplTabGroup *tab_group)
{
	GList *tabs;
	GList *buffers = NULL;
	GList *l;

	g_return_val_if_fail (TEPL_IS_TAB_GROUP (tab_group), NULL);

	tabs = tepl_tab_group_get_tabs (tab_group);

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
 * tepl_tab_group_get_active_tab:
 * @tab_group: a #TeplTabGroup.
 *
 * Returns: (transfer none) (nullable): the #TeplTab currently shown in
 * @tab_group.
 * Since: 3.0
 */
TeplTab *
tepl_tab_group_get_active_tab (TeplTabGroup *tab_group)
{
	g_return_val_if_fail (TEPL_IS_TAB_GROUP (tab_group), NULL);

	return TEPL_TAB_GROUP_GET_INTERFACE (tab_group)->get_active_tab (tab_group);
}

/**
 * tepl_tab_group_get_active_view:
 * @tab_group: a #TeplTabGroup.
 *
 * Convenience function.
 *
 * Returns: (transfer none) (nullable): the #TeplView of the active tab.
 * Since: 3.0
 */
TeplView *
tepl_tab_group_get_active_view (TeplTabGroup *tab_group)
{
	TeplTab *active_tab;

	g_return_val_if_fail (TEPL_IS_TAB_GROUP (tab_group), NULL);

	active_tab = tepl_tab_group_get_active_tab (tab_group);

	return active_tab != NULL ? tepl_tab_get_view (active_tab) : NULL;
}

/**
 * tepl_tab_group_get_active_buffer:
 * @tab_group: a #TeplTabGroup.
 *
 * Convenience function.
 *
 * Returns: (transfer none) (nullable): the #TeplBuffer of the active tab.
 * Since: 3.0
 */
TeplBuffer *
tepl_tab_group_get_active_buffer (TeplTabGroup *tab_group)
{
	TeplTab *active_tab;

	g_return_val_if_fail (TEPL_IS_TAB_GROUP (tab_group), NULL);

	active_tab = tepl_tab_group_get_active_tab (tab_group);

	return active_tab != NULL ? tepl_tab_get_buffer (active_tab) : NULL;
}