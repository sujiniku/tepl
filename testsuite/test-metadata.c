/* Copyright 2020 - Sébastien Wilmet <swilmet@gnome.org>
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
