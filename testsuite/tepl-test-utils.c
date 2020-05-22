/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "tepl-test-utils.h"

/* Common utility functions for the unit tests. */

void
_tepl_test_utils_set_file_content (GFile       *file,
				   const gchar *content)
{
	GError *error = NULL;

	g_file_replace_contents (file,
				 content,
				 strlen (content),
				 NULL,
				 FALSE,
				 G_FILE_CREATE_REPLACE_DESTINATION,
				 NULL,
				 NULL,
				 &error);
	g_assert_no_error (error);
}
