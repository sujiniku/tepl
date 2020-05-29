/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "tepl/tepl-icu.h"

static void
check_str_from_and_to_utf8_raw (const gchar *utf8_str,
				gboolean     expect_success)
{
	UChar *uchars;
	gchar *utf8_str_after_round_trip;
	UErrorCode error_code = U_ZERO_ERROR;

	uchars = _tepl_icu_strFromUTF8 (NULL, utf8_str, -1, &error_code);
	if (expect_success)
	{
		g_assert_true (U_SUCCESS (error_code));
	}
	else
	{
		g_assert_true (U_FAILURE (error_code));
		g_free (uchars);
		return;
	}

	error_code = U_ZERO_ERROR;
	utf8_str_after_round_trip = _tepl_icu_strToUTF8 (NULL, uchars, -1, &error_code);
	g_assert_cmpstr (utf8_str, ==, utf8_str_after_round_trip);

	g_free (uchars);
	g_free (utf8_str_after_round_trip);
}

static void
check_str_from_and_to_utf8_simple (const gchar *utf8_str,
				   gboolean     expect_success)
{
	UChar *uchars;
	gchar *utf8_str_after_round_trip;

	uchars = _tepl_icu_strFromUTF8Simple (utf8_str);
	if (expect_success)
	{
		g_assert_true (uchars != NULL);
	}
	else
	{
		g_assert_true (uchars == NULL);
		return;
	}

	utf8_str_after_round_trip = _tepl_icu_strToUTF8Simple (uchars);
	g_assert_cmpstr (utf8_str, ==, utf8_str_after_round_trip);

	g_free (uchars);
	g_free (utf8_str_after_round_trip);
}

static void
check_str_from_and_to_utf8 (const gchar *utf8_str,
			    gboolean     expect_success)
{
	check_str_from_and_to_utf8_raw (utf8_str, expect_success);
	check_str_from_and_to_utf8_simple (utf8_str, expect_success);
}

static void
test_str_from_and_to_utf8 (void)
{
	check_str_from_and_to_utf8 (NULL, FALSE);
	check_str_from_and_to_utf8 ("", TRUE);
	check_str_from_and_to_utf8 ("ASCII", TRUE);
	check_str_from_and_to_utf8 ("À ski", TRUE);

	/* Not valid UTF-8. */
	check_str_from_and_to_utf8 ("\xFF", FALSE);
}

static void
test_strdup (void)
{
	UChar *uchars;
	UChar *uchars_copy;
	gchar *utf8_str;

	uchars = _tepl_icu_strFromUTF8Simple ("Évo");
	uchars_copy = _tepl_icu_strdup (uchars);
	utf8_str = _tepl_icu_strToUTF8Simple (uchars_copy);
	g_assert_cmpstr (utf8_str, ==, "Évo");

	g_free (uchars);
	g_free (uchars_copy);
	g_free (utf8_str);
}

static void
test_trans_open (void)
{
	UTransliterator *transliterator;

	transliterator = _tepl_icu_trans_open_xml_escape ();
	g_assert_true (transliterator != NULL);
	utrans_close (transliterator);
}

int
main (int    argc,
      char **argv)
{
	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/icu/str_from_and_to_utf8", test_str_from_and_to_utf8);
	g_test_add_func ("/icu/strdup", test_strdup);
	g_test_add_func ("/icu/trans_open", test_trans_open);

	return g_test_run ();
}
