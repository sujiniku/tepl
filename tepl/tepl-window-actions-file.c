/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2020 - Sébastien Wilmet <swilmet@gnome.org>
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

#include "tepl-window-actions-file.h"
#include <amtk/amtk.h>
#include "tepl-abstract-factory.h"
#include "tepl-tab-group.h"

/* TeplApplicationWindow GActions for the File menu. */

static void
new_file_activate_cb (GSimpleAction *action,
		      GVariant      *parameter,
		      gpointer       user_data)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (user_data);
	TeplAbstractFactory *factory;
	TeplTab *new_tab;

	factory = tepl_abstract_factory_get_singleton ();
	new_tab = tepl_abstract_factory_create_tab (factory);
	gtk_widget_show (GTK_WIDGET (new_tab));

	tepl_tab_group_append_tab (TEPL_TAB_GROUP (tepl_window), new_tab, TRUE);
}

void
_tepl_window_actions_file_add_actions (TeplApplicationWindow *tepl_window)
{
	GtkApplicationWindow *gtk_window;

	const GActionEntry entries[] = {
		{ "tepl-new-file", new_file_activate_cb },
	};

	g_return_if_fail (TEPL_IS_APPLICATION_WINDOW (tepl_window));

	gtk_window = tepl_application_window_get_application_window (tepl_window);

	amtk_action_map_add_action_entries_check_dups (G_ACTION_MAP (gtk_window),
						       entries,
						       G_N_ELEMENTS (entries),
						       tepl_window);
}
