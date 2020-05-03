/* Copyright 2017 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "tepl-tab-group.h"

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
tepl_tab_group_set_active_tab_default (TeplTabGroup *tab_group,
				       TeplTab      *tab)
{
}

static void
tepl_tab_group_append_tab_vfunc_default (TeplTabGroup *tab_group,
					 TeplTab      *tab)
{
	g_warning ("Appending a TeplTab to this TeplTabGroup is not supported. "
		   "Use for example TeplNotebook.");
}

static void
tepl_tab_group_default_init (TeplTabGroupInterface *interface)
{
	interface->get_tabs = tepl_tab_group_get_tabs_default;
	interface->get_active_tab = tepl_tab_group_get_active_tab_default;
	interface->set_active_tab = tepl_tab_group_set_active_tab_default;
	interface->append_tab_vfunc = tepl_tab_group_append_tab_vfunc_default;

	/**
	 * TeplTabGroup:active-tab:
	 *
	 * The #TeplTab currently shown.
	 *
	 * Since: 3.0
	 */
	g_object_interface_install_property (interface,
					     g_param_spec_object ("active-tab",
								  "Active Tab",
								  "",
								  TEPL_TYPE_TAB,
								  G_PARAM_READWRITE |
								  G_PARAM_STATIC_STRINGS));

	/**
	 * TeplTabGroup:active-view:
	 *
	 * The #TeplView of the active tab.
	 *
	 * Since: 3.0
	 */
	g_object_interface_install_property (interface,
					     g_param_spec_object ("active-view",
								  "Active View",
								  "",
								  TEPL_TYPE_VIEW,
								  G_PARAM_READABLE |
								  G_PARAM_STATIC_STRINGS));

	/**
	 * TeplTabGroup:active-buffer:
	 *
	 * The #TeplBuffer of the active tab.
	 *
	 * Since: 3.0
	 */
	g_object_interface_install_property (interface,
					     g_param_spec_object ("active-buffer",
								  "Active Buffer",
								  "",
								  TEPL_TYPE_BUFFER,
								  G_PARAM_READABLE |
								  G_PARAM_STATIC_STRINGS));
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
 * tepl_tab_group_set_active_tab:
 * @tab_group: a #TeplTabGroup.
 * @tab: a #TeplTab part of @tab_group.
 *
 * Sets the #TeplTabGroup:active-tab. @tab must be part of @tab_group.
 *
 * Since: 3.0
 */
void
tepl_tab_group_set_active_tab (TeplTabGroup *tab_group,
			       TeplTab      *tab)
{
	GList *all_tabs;
	gboolean tab_in_tab_group;

	g_return_if_fail (TEPL_IS_TAB_GROUP (tab_group));
	g_return_if_fail (TEPL_IS_TAB (tab));

	all_tabs = tepl_tab_group_get_tabs (tab_group);
	tab_in_tab_group = g_list_find (all_tabs, tab) != NULL;
	g_list_free (all_tabs);
	g_return_if_fail (tab_in_tab_group);

	TEPL_TAB_GROUP_GET_INTERFACE (tab_group)->set_active_tab (tab_group, tab);
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

/**
 * tepl_tab_group_append_tab:
 * @tab_group: a #TeplTabGroup.
 * @tab: a #TeplTab.
 * @jump_to: whether to set @tab as the active tab after appending it.
 *
 * Appends @tab to @tab_group.
 *
 * Since: 3.0
 */
void
tepl_tab_group_append_tab (TeplTabGroup *tab_group,
			   TeplTab      *tab,
			   gboolean      jump_to)
{
	g_return_if_fail (TEPL_IS_TAB_GROUP (tab_group));
	g_return_if_fail (TEPL_IS_TAB (tab));

	TEPL_TAB_GROUP_GET_INTERFACE (tab_group)->append_tab_vfunc (tab_group, tab);

	if (jump_to)
	{
		TeplView *view;

		tepl_tab_group_set_active_tab (tab_group, tab);

		view = tepl_tab_get_view (tab);
		gtk_widget_grab_focus (GTK_WIDGET (view));
	}
}
