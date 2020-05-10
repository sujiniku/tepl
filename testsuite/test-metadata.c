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

static void
check_get (TeplMetadata *metadata,
	   const gchar  *key,
	   const gchar  *expected_value)
{
	gchar *received_value;

	received_value = tepl_metadata_get (metadata, key);
	g_assert_cmpstr (expected_value, ==, received_value);
	g_free (received_value);
}

static void
test_get_set (void)
{
	TeplMetadata *metadata;

	metadata = tepl_metadata_new ();
	check_get (metadata, "keyA", NULL);

	tepl_metadata_set (metadata, "keyA", "valueA1");
	check_get (metadata, "keyA", "valueA1");

	tepl_metadata_set (metadata, "keyB", "valueB");
	check_get (metadata, "keyA", "valueA1");
	check_get (metadata, "keyB", "valueB");
	check_get (metadata, "keyC", NULL);

	tepl_metadata_set (metadata, "keyA", "valueA2");
	check_get (metadata, "keyA", "valueA2");
	check_get (metadata, "keyB", "valueB");

	tepl_metadata_set (metadata, "keyB", NULL);
	check_get (metadata, "keyA", "valueA2");
	check_get (metadata, "keyB", NULL);
	check_get (metadata, "keyC", NULL);

	g_object_unref (metadata);
}

int
main (int    argc,
      char **argv)
{
	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/metadata/key_is_valid", test_key_is_valid);
	g_test_add_func ("/metadata/get_set", test_get_set);

	return g_test_run ();
}
