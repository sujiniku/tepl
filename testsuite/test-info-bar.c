/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
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

#include <tepl/tepl.h>

static void
test_info_bar_creation (void)
{
	TeplInfoBar *info_bar;

	/* Just to see if it doesn't trigger any warnings. */
	info_bar = tepl_info_bar_new ();
	g_object_ref_sink (info_bar);
	g_object_unref (info_bar);
}

gint
main (gint    argc,
      gchar **argv)
{
	gtk_test_init (&argc, &argv, NULL);

	g_test_add_func ("/info-bar/creation", test_info_bar_creation);

	return g_test_run ();
}
