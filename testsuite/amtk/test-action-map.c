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

#include <amtk/amtk.h>

static void
test_add_action_entries_check_dups (void)
{
	GSimpleActionGroup *group;
	const GActionEntry entries[] =
	{
		{ "quit" },
		{ "about" }
	};

	group = g_simple_action_group_new ();
	amtk_action_map_add_action_entries_check_dups (G_ACTION_MAP (group),
						       entries,
						       G_N_ELEMENTS (entries),
						       NULL);
	g_object_unref (group);

	g_test_trap_subprocess ("/action-map/add-action-entries-check-dups/subprocess/add-two-times", 0, 0);
	g_test_trap_assert_failed ();
	g_test_trap_assert_stderr ("*the GActionMap already contains a GAction with the name*");

	g_test_trap_subprocess ("/action-map/add-action-entries-check-dups/subprocess/dups-in-array", 0, 0);
	g_test_trap_assert_failed ();
	g_test_trap_assert_stderr ("*the GActionEntry array contains duplicated entries for the action name*");
}

static void
test_add_action_entries_check_dups__add_two_times (void)
{
	GSimpleActionGroup *group;
	const GActionEntry entries[] =
	{
		{ "quit" }
	};

	group = g_simple_action_group_new ();

	amtk_action_map_add_action_entries_check_dups (G_ACTION_MAP (group),
						       entries,
						       G_N_ELEMENTS (entries),
						       NULL);
	amtk_action_map_add_action_entries_check_dups (G_ACTION_MAP (group),
						       entries,
						       G_N_ELEMENTS (entries),
						       NULL);

	g_object_unref (group);
}

static void
test_add_action_entries_check_dups__dups_in_array (void)
{
	GSimpleActionGroup *group;
	const GActionEntry entries[] =
	{
		{ "quit" },
		{ "quit" }
	};

	group = g_simple_action_group_new ();

	amtk_action_map_add_action_entries_check_dups (G_ACTION_MAP (group),
						       entries,
						       G_N_ELEMENTS (entries),
						       NULL);

	g_object_unref (group);
}

int
main (int    argc,
      char **argv)
{
	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/action-map/add-action-entries-check-dups",
			 test_add_action_entries_check_dups);
	g_test_add_func ("/action-map/add-action-entries-check-dups/subprocess/add-two-times",
			 test_add_action_entries_check_dups__add_two_times);
	g_test_add_func ("/action-map/add-action-entries-check-dups/subprocess/dups-in-array",
			 test_add_action_entries_check_dups__dups_in_array);

	return g_test_run ();
}
