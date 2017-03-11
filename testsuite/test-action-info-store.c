/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2017 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * Gtef is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * Gtef is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include <gtef/gtef.h>

static void
test_add_entries (void)
{
	GtefActionInfoStore *store;
	GtefActionInfoCentralStore *central_store;
	const GtefActionInfo *info1;
	const GtefActionInfo *info2;

	const GtefActionInfoEntry entries[] =
	{
		/* action, icon, label, accel, tooltip */

		{ "app.quit", "application-exit", "_Quit", "<Control>q",
		  "Quit the application" },

		/* Tooltip field missing, must be NULL. */
		{ "win.open", "document-open", "_Open", "<Control>o" },
	};

	store = gtef_action_info_store_new (NULL);

	gtef_action_info_store_add_entries (store,
					    entries,
					    G_N_ELEMENTS (entries),
					    NULL);

	info1 = gtef_action_info_store_lookup (store, "win.open");
	g_assert (info1 != NULL);
	g_assert_cmpstr (gtef_action_info_get_icon_name (info1), ==, "document-open");
	g_assert (gtef_action_info_get_tooltip (info1) == NULL);

	central_store = gtef_action_info_central_store_get_instance ();
	info2 = gtef_action_info_central_store_lookup (central_store, "win.open");
	g_assert (info1 == info2);

	info1 = gtef_action_info_store_lookup (store, "plouf");
	g_assert (info1 == NULL);

	g_object_unref (store);
}

int
main (int argc, char **argv)
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/action-info-store/add-entries", test_add_entries);

	return g_test_run ();
}
