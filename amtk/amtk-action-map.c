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

#include "amtk-action-map.h"

/**
 * SECTION:amtk-action-map
 * @Short_description: #GActionMap wrapper functions
 * @Title: AmtkActionMap
 *
 * #GActionMap wrapper functions.
 */

static void
check_dups_in_array (const GActionEntry *entries,
		     const gchar        *action_name,
		     gint                action_num)
{
	gint i;

	for (i = 0; i < action_num; i++)
	{
		const GActionEntry *entry = &entries[i];

		if (g_strcmp0 (action_name, entry->name) == 0)
		{
			g_warning ("amtk_action_map_add_action_entries_check_dups(): "
				   "the GActionEntry array contains duplicated entries for the action name '%s'. "
				   "The first one will be dropped from the GActionMap.",
				   action_name);
			return;
		}
	}
}

/**
 * amtk_action_map_add_action_entries_check_dups:
 * @action_map: a #GActionMap.
 * @entries: (array length=n_entries) (element-type GActionEntry): a pointer to
 *           the first item in an array of #GActionEntry structs.
 * @n_entries: the length of @entries, or -1 if @entries is %NULL-terminated.
 * @user_data: the user data for signal connections.
 *
 * A wrapper function for g_action_map_add_action_entries() that checks
 * duplicates.
 *
 * This function first checks - for each entry - that the @action_map doesn't
 * already contain a #GAction with the same name. A warning is printed if an old
 * action will be dropped. In any case, it then calls
 * g_action_map_add_action_entries() with the same arguments as passed to this
 * function.
 *
 * This function also checks if there are duplicates in the @entries array
 * itself.
 *
 * Since: 2.0
 */
void
amtk_action_map_add_action_entries_check_dups (GActionMap         *action_map,
					       const GActionEntry *entries,
					       gint                n_entries,
					       gpointer            user_data)
{
	gint i;

	g_return_if_fail (G_IS_ACTION_MAP (action_map));
	g_return_if_fail (n_entries >= -1);
	g_return_if_fail (entries != NULL || n_entries == 0);

	for (i = 0; n_entries == -1 ? entries[i].name != NULL : i < n_entries; i++)
	{
		const GActionEntry *entry = &entries[i];

		if (g_action_map_lookup_action (action_map, entry->name) != NULL)
		{
			g_warning ("%s(): the GActionMap already contains a GAction with the name '%s'. "
				   "The old GAction will be dropped from the GActionMap.",
				   G_STRFUNC,
				   entry->name);
		}

		check_dups_in_array (entries, entry->name, i);
	}

	g_action_map_add_action_entries (action_map,
					 entries,
					 n_entries,
					 user_data);
}
