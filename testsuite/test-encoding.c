/* SPDX-FileCopyrightText: 2014, 2017 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <tepl/tepl.h>
#include "tepl/tepl-encoding-private.h"

static void
test_remove_duplicates (void)
{
	GSList *list = NULL;
	TeplEncoding *utf8;
	TeplEncoding *iso;

	utf8 = tepl_encoding_new_utf8 ();
	iso = tepl_encoding_new ("ISO-8859-15");

	/* Before: [UTF-8, ISO-8859-15, UTF-8] */
	list = g_slist_prepend (list, tepl_encoding_copy (utf8));
	list = g_slist_prepend (list, tepl_encoding_copy (iso));
	list = g_slist_prepend (list, tepl_encoding_copy (utf8));

	/* After: [UTF-8, ISO-8859-15] */
	list = _tepl_encoding_remove_duplicates (list, TEPL_ENCODING_DUPLICATES_KEEP_FIRST);

	g_assert_cmpint (2, ==, g_slist_length (list));
	g_assert_true (tepl_encoding_equals (list->data, utf8));
	g_assert_true (tepl_encoding_equals (list->next->data, iso));

	/* Before: [UTF-8, ISO-8859-15, UTF-8] */
	list = g_slist_append (list, tepl_encoding_copy (utf8));

	/* After: [ISO-8859-15, UTF-8] */
	list = _tepl_encoding_remove_duplicates (list, TEPL_ENCODING_DUPLICATES_KEEP_LAST);

	g_assert_cmpint (2, ==, g_slist_length (list));
	g_assert_true (tepl_encoding_equals (list->data, iso));
	g_assert_true (tepl_encoding_equals (list->next->data, utf8));

	g_slist_free_full (list, (GDestroyNotify)tepl_encoding_free);
	tepl_encoding_free (utf8);
	tepl_encoding_free (iso);
}

int
main (int argc, char **argv)
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/Encoding/remove_duplicates", test_remove_duplicates);

	return g_test_run ();
}
