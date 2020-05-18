/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <tepl/tepl.h>

static void
test_properties (void)
{
	TeplBuffer *buffer;
	TeplFile *file;
	GFile *location;
	TeplFileLoader *loader;

	buffer = tepl_buffer_new ();

	file = tepl_buffer_get_file (buffer);
	location = g_file_new_for_path ("location");
	tepl_file_set_location (file, location);

	loader = tepl_file_loader_new (buffer, file);
	g_assert_true (tepl_file_loader_get_buffer (loader) == buffer);
	g_assert_true (tepl_file_loader_get_file (loader) == file);
	g_assert_true (tepl_file_loader_get_location (loader) == location);

	g_object_unref (buffer);
	g_object_unref (location);
	g_object_unref (loader);
}

int
main (int    argc,
      char **argv)
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/file_loader/properties", test_properties);

	return g_test_run ();
}
