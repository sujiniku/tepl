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

#include "tepl-application-window-actions.h"
#include "tepl-application-window.h"
#include "tepl-action-map.h"
#include "tepl-tab-group.h"
#include "tepl-view.h"

/* GAction implementations for TeplApplicationWindow. This is implemented in a
 * separate file to not clutter TeplApplicationWindow.
 */

static void
select_all_cb (GSimpleAction *action,
	       GVariant      *parameter,
	       gpointer       user_data)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (user_data);
	TeplView *active_view;

	active_view = tepl_tab_group_get_active_view (TEPL_TAB_GROUP (tepl_window));

	if (active_view != NULL)
	{
		tepl_view_select_all (active_view);
	}
}

void
_tepl_application_window_add_actions (TeplApplicationWindow *tepl_window)
{
	GtkApplicationWindow *gtk_window;

	/* The actions need to be namespaced, to not conflict with the
	 * application or other libraries.
	 *
	 * Do not forget to document each action in the TeplApplicationWindow
	 * class description.
	 */
	const GActionEntry entries[] = {
		{ "tepl-select-all", select_all_cb },
	};

	gtk_window = tepl_application_window_get_application_window (tepl_window);

	tepl_action_map_add_action_entries_check_dups (G_ACTION_MAP (gtk_window),
						       entries,
						       G_N_ELEMENTS (entries),
						       tepl_window);
}
