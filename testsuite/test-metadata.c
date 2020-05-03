/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <tepl/tepl.h>

static void
test_key_is_valid (void)
{
	g_assert_true (_tepl_metadata_key_is_valid ("gedit-spell-checking-language"));
	g_assert_true (_tepl_metadata_key_is_valid ("gCSVedit_column_delimiter"));
	g_assert_true (_tepl_metadata_key_is_valid ("Fourty_Two-1337"));
	g_assert_true (_tepl_metadata_key_is_valid ("1337-beginning-with-digit"));
	g_assert_true (_tepl_metadata_key_is_valid ("a"));
	g_assert_true (_tepl_metadata_key_is_valid ("9"));

	g_assert_true (!_tepl_metadata_key_is_valid (NULL));
	g_assert_true (!_tepl_metadata_key_is_valid (""));
	g_assert_true (!_tepl_metadata_key_is_valid ("metadata::gedit-spell-checking-language"));
	g_assert_true (!_tepl_metadata_key_is_valid ("foo:bar"));
	g_assert_true (!_tepl_metadata_key_is_valid ("foo::bar"));
	g_assert_true (!_tepl_metadata_key_is_valid ("Évolution-UTF-8"));
	g_assert_true (!_tepl_metadata_key_is_valid ("a space"));
	g_assert_true (!_tepl_metadata_key_is_valid ("\t"));

	g_assert_true (!g_utf8_validate ("\xFF", -1, NULL));
	g_assert_true (!_tepl_metadata_key_is_valid ("\xFF"));
}

int
main (int    argc,
      char **argv)
{
	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/metadata/key_is_valid", test_key_is_valid);

	return g_test_run ();
}
