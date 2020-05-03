/* SPDX-FileCopyrightText: 2016 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
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
