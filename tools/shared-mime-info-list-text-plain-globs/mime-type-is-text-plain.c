/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <gio/gio.h>
#include <stdlib.h>

int
main (int    argc,
      char **argv)
{
	const gchar *mime_type;

	if (argc != 2)
	{
		g_printerr ("Usage: %s <mime-type>\n", argv[0]);
		return EXIT_FAILURE;
	}

	mime_type = argv[1];

	if (g_content_type_is_a (mime_type, "text/plain"))
	{
		return EXIT_SUCCESS;
	}

	return EXIT_FAILURE;
}
