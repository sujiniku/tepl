/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "tepl-window-actions-edit.h"
#include <amtk/amtk.h>
#include "tepl-tab-group.h"
#include "tepl-view.h"

/* TeplApplicationWindow GActions for the Edit menu. */

static void
undo_activate_cb (GSimpleAction *action,
		  GVariant      *parameter,
		  gpointer       user_data)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (user_data);
	TeplView *view;

	view = tepl_tab_group_get_active_view (TEPL_TAB_GROUP (tepl_window));

	if (view != NULL)
	{
		TeplBuffer *buffer;

		buffer = tepl_tab_group_get_active_buffer (TEPL_TAB_GROUP (tepl_window));

		gtk_source_buffer_undo (GTK_SOURCE_BUFFER (buffer));
		tepl_view_scroll_to_cursor (view);
		gtk_widget_grab_focus (GTK_WIDGET (view));
	}
}

static void
redo_activate_cb (GSimpleAction *action,
		  GVariant      *parameter,
		  gpointer       user_data)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (user_data);
	TeplView *view;

	view = tepl_tab_group_get_active_view (TEPL_TAB_GROUP (tepl_window));

	if (view != NULL)
	{
		TeplBuffer *buffer;

		buffer = tepl_tab_group_get_active_buffer (TEPL_TAB_GROUP (tepl_window));

		gtk_source_buffer_redo (GTK_SOURCE_BUFFER (buffer));
		tepl_view_scroll_to_cursor (view);
		gtk_widget_grab_focus (GTK_WIDGET (view));
	}
}

void
_tepl_window_actions_edit_add_actions (TeplApplicationWindow *tepl_window)
{
	GtkApplicationWindow *gtk_window;

	const GActionEntry entries[] = {
		{ "tepl-undo", undo_activate_cb },
		{ "tepl-redo", redo_activate_cb },
	};

	g_return_if_fail (TEPL_IS_APPLICATION_WINDOW (tepl_window));

	gtk_window = tepl_application_window_get_application_window (tepl_window);
	amtk_action_map_add_action_entries_check_dups (G_ACTION_MAP (gtk_window),
						       entries,
						       G_N_ELEMENTS (entries),
						       tepl_window);
}
