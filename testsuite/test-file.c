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

#include <gtef/gtef.h>

static void
test_set_metadata (void)
{
	GtefFile *file;
	const gchar *key;
	gchar *value;

	file = gtef_file_new ();

	key = "gtef-test-key";
	value = gtef_file_get_metadata (file, key);
	g_assert (value == NULL);

	gtef_file_set_metadata (file, key, "zippy");
	value = gtef_file_get_metadata (file, key);
	g_assert_cmpstr (value, ==, "zippy");
	g_free (value);

	value = gtef_file_get_metadata (file, "gtef-test-other-key");
	g_assert (value == NULL);

	gtef_file_set_metadata (file, key, "zippiness");
	value = gtef_file_get_metadata (file, key);
	g_assert_cmpstr (value, ==, "zippiness");
	g_free (value);

	/* Unset */
	gtef_file_set_metadata (file, key, NULL);
	value = gtef_file_get_metadata (file, key);
	g_assert (value == NULL);

	/* Unset non-set metadata */
	key = "gtef-test-other-key";
	gtef_file_set_metadata (file, key, NULL);
	value = gtef_file_get_metadata (file, key);
	g_assert (value == NULL);

	g_object_unref (file);
}

gint
main (gint    argc,
      gchar **argv)
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/file/set_metadata", test_set_metadata);

	return g_test_run ();
}
