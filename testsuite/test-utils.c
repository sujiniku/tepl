/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "gtef/gtef-utils.h"

static void
test_replace_home_dir_with_tilde (void)
{
	const gchar *homedir = g_get_home_dir ();
	gchar *before;
	gchar *after;

	before = g_build_filename (homedir, "blah", NULL);
	after = _gtef_utils_replace_home_dir_with_tilde (before);
	g_assert_cmpstr (after, ==, "~/blah");
	g_free (before);
	g_free (after);

	after = _gtef_utils_replace_home_dir_with_tilde (homedir);
	g_assert_cmpstr (after, ==, "~");
	g_free (after);

	after = _gtef_utils_replace_home_dir_with_tilde ("/blah");
	g_assert_cmpstr (after, ==, "/blah");
	g_free (after);
}

gint
main (gint    argc,
      gchar **argv)
{
	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/utils/replace-home-dir-with-tilde", test_replace_home_dir_with_tilde);

	return g_test_run ();
}
