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

#include <tepl/tepl.h>

static void
metadata_store_load_cb (GObject      *source_object,
			GAsyncResult *result,
			gpointer      user_data)
{
	TeplMetadataStore *store = TEPL_METADATA_STORE (source_object);
	GError *error = NULL;

	tepl_metadata_store_load_finish (store, result, &error);
	g_assert_no_error (error);

	gtk_main_quit ();
}

static void
test_basic (void)
{
	TeplMetadataStore *store;
	GFile *store_file;

	store = tepl_metadata_store_get_singleton ();

	store_file = g_file_new_for_path ("gcsvedit-metadata.xml");
	tepl_metadata_store_set_store_file (store, store_file);
	g_object_unref (store_file);

	tepl_metadata_store_load_async (store,
					G_PRIORITY_DEFAULT,
					NULL,
					metadata_store_load_cb,
					NULL);
	gtk_main ();

	_tepl_metadata_store_unref_singleton ();
}

int
main (int    argc,
      char **argv)
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/metadata-store/basic", test_basic);

	return g_test_run ();
}
