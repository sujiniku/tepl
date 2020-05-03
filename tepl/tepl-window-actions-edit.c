/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "tepl-window-actions-edit.h"
#include <amtk/amtk.h>

/* TeplApplicationWindow GActions for the Edit menu. */

void
_tepl_window_actions_edit_add_actions (TeplApplicationWindow *tepl_window)
{
	GtkApplicationWindow *gtk_window;

	const GActionEntry entries[] = {
	};

	g_return_if_fail (TEPL_IS_APPLICATION_WINDOW (tepl_window));

	gtk_window = tepl_application_window_get_application_window (tepl_window);
	amtk_action_map_add_action_entries_check_dups (G_ACTION_MAP (gtk_window),
						       entries,
						       G_N_ELEMENTS (entries),
						       tepl_window);
}
