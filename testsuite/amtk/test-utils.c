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

#include "amtk/amtk-utils.h"

static void
check_strv_equal (const gchar * const *strv1,
		  const gchar * const *strv2)
{
	gint i;

	if (strv1 == NULL || strv2 == NULL)
	{
		g_assert (strv1 == NULL && strv2 == NULL);
		return;
	}

	for (i = 0; strv1[i] != NULL && strv2[i] != NULL; i++)
	{
		g_assert_cmpstr (strv1[i], ==, strv2[i]);
	}

	g_assert (strv1[i] == NULL);
	g_assert (strv2[i] == NULL);
}

static void
test_strv_copy (void)
{
	const gchar *stack_strv_empty[] = { NULL };
	const gchar *stack_strv_nonempty[] = { "a", "b", NULL };
	GPtrArray *ptr_array;
	gchar **heap_strv;
	gchar **strv_copy;

	/* NULL */
	strv_copy = _amtk_utils_strv_copy (NULL);
	g_assert (strv_copy == NULL);

	/* Empty */
	strv_copy = _amtk_utils_strv_copy (stack_strv_empty);
	check_strv_equal (stack_strv_empty, (const gchar * const *)strv_copy);
	g_strfreev (strv_copy);

	/* Non-empty */
	strv_copy = _amtk_utils_strv_copy (stack_strv_nonempty);
	check_strv_equal (stack_strv_nonempty, (const gchar * const *)strv_copy);
	g_strfreev (strv_copy);

	/* Created from a GPtrArray */
	ptr_array = g_ptr_array_new ();
	g_ptr_array_add (ptr_array, g_strdup (""));
	g_ptr_array_add (ptr_array, g_strdup ("non-empty"));
	g_ptr_array_add (ptr_array, g_strdup ("bathory"));
	g_ptr_array_add (ptr_array, NULL);

	heap_strv = (gchar **)g_ptr_array_free (ptr_array, FALSE);

	strv_copy = _amtk_utils_strv_copy ((const gchar * const *)heap_strv);
	check_strv_equal ((const gchar * const *)heap_strv,
			  (const gchar * const *)strv_copy);
	g_strfreev (strv_copy);

	g_strfreev (heap_strv);
}

int
main (int    argc,
      char **argv)
{
	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/utils/strv-copy", test_strv_copy);

	return g_test_run ();
}
