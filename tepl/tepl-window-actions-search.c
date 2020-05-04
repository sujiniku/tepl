/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "tepl-window-actions-search.h"
#include <amtk/amtk.h>
#include "tepl-tab.h"

/* TeplApplicationWindow GActions for the Search menu. */

static void
update_goto_line_action_sensitivity (TeplApplicationWindow *tepl_window)
{
	TeplTab *active_tab;
	GActionMap *action_map;
	GAction *action;

	active_tab = tepl_tab_group_get_active_tab (TEPL_TAB_GROUP (tepl_window));

	action_map = G_ACTION_MAP (tepl_application_window_get_application_window (tepl_window));

	action = g_action_map_lookup_action (action_map, "tepl-goto-line");
	g_simple_action_set_enabled (G_SIMPLE_ACTION (action),
				     active_tab != NULL);
}

static void
update_goto_line (TeplApplicationWindow *tepl_window)
{
	GActionMap *action_map;
	GAction *goto_line_action;
	TeplTab *active_tab;
	TeplGotoLineBar *goto_line_bar;

	update_goto_line_action_sensitivity (tepl_window);

	action_map = G_ACTION_MAP (tepl_application_window_get_application_window (tepl_window));
	goto_line_action = g_action_map_lookup_action (action_map, "tepl-goto-line");

	active_tab = tepl_tab_group_get_active_tab (TEPL_TAB_GROUP (tepl_window));
	if (active_tab == NULL)
	{
		/* FIXME: should be done when the TeplTabGroup is *empty*. When
		 * the active_tab == NULL it's an approximation. More APIs need
		 * to be added to TeplTabGroup.
		 */
		g_simple_action_set_state (G_SIMPLE_ACTION (goto_line_action),
					   g_variant_new_boolean (FALSE));
		return;
	}

	/* FIXME: would be nice to call
	 * _tepl_goto_line_bar_bind_to_gaction_state() directly for all
	 * TeplTab's, when they are added to the TeplTabGroup.
	 */
	goto_line_bar = tepl_tab_get_goto_line_bar (active_tab);
	_tepl_goto_line_bar_bind_to_gaction_state (goto_line_bar, goto_line_action);
}

static void
goto_line_activate_cb (GSimpleAction *action,
		       GVariant      *parameter,
		       gpointer       user_data)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (user_data);
	TeplTab *active_tab;

	g_action_change_state (G_ACTION (action), g_variant_new_boolean (TRUE));

	active_tab = tepl_tab_group_get_active_tab (TEPL_TAB_GROUP (tepl_window));
	if (active_tab != NULL)
	{
		TeplGotoLineBar *goto_line_bar;

		goto_line_bar = tepl_tab_get_goto_line_bar (active_tab);
		tepl_goto_line_bar_grab_focus_to_entry (goto_line_bar);
	}
}

static void
goto_line_change_state_cb (GSimpleAction *action,
			   GVariant      *value,
			   gpointer       user_data)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (user_data);

	g_simple_action_set_state (action, value);
	update_goto_line (tepl_window);
}

static void
active_tab_notify_cb (TeplApplicationWindow *tepl_window,
		      GParamSpec            *pspec,
		      gpointer               user_data)
{
	update_goto_line (tepl_window);
}

void
_tepl_window_actions_search_add_actions (TeplApplicationWindow *tepl_window)
{
	GtkApplicationWindow *gtk_window;

	const GActionEntry entries[] = {
		{ "tepl-goto-line", goto_line_activate_cb, NULL, "false", goto_line_change_state_cb },
	};

	g_return_if_fail (TEPL_IS_APPLICATION_WINDOW (tepl_window));

	gtk_window = tepl_application_window_get_application_window (tepl_window);

	amtk_action_map_add_action_entries_check_dups (G_ACTION_MAP (gtk_window),
						       entries,
						       G_N_ELEMENTS (entries),
						       tepl_window);

	update_goto_line (tepl_window);

	g_signal_connect (tepl_window,
			  "notify::active-tab",
			  G_CALLBACK (active_tab_notify_cb),
			  NULL);
}
