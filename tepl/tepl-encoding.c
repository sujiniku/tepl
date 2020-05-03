/* SPDX-FileCopyrightText: 2002-2005 - Paolo Maggi
 * SPDX-FileCopyrightText: 2014, 2015, 2017 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "config.h"
#include "tepl-encoding.h"
#include "tepl-encoding-private.h"
#include <glib/gi18n-lib.h>

/**
 * SECTION:encoding
 * @Short_description: Character encoding
 * @Title: TeplEncoding
 * @See_also: #TeplFileSaver, #TeplFileLoader
 *
 * The #TeplEncoding boxed type represents a character encoding. It is used
 * for example by #TeplFile. Note that the text in GTK widgets is always
 * encoded in UTF-8.
 *
 * #TeplEncoding is a fork of #GtkSourceEncoding with a different API to
 * accommodate the new #TeplFileLoader implementation.
 */

/* API design:
 *
 * Why not using a simple string to store a charset? With for example a
 * TeplEncodingTable singleton to retrieve the information from the table.
 * An API like that would have some ugliness:
 * - To compare charsets, g_ascii_strcasecmp() must be used. TeplEncoding hides
 *   that with tepl_encoding_equals().
 * - The result of `iconv --list` contains duplicates, for example "UTF8" and
 *   "UTF-8". Since UTF-8 is an important case, TeplEncoding has special cases
 *   and special functions for it: tepl_encoding_new_utf8() and
 *   tepl_encoding_is_utf8().
 *
 * An earlier implementation of this class (the first implementation of
 * GtkSourceEncoding, which came from gedit) didn't support other charsets than
 * those listed in the table (plus UTF-8 and the locale encoding).
 * gtk_source_encoding_get_from_charset() returned NULL in case the charset was
 * not found. The problem is that the table is not complete, it just contains
 * common encodings. For GtkSourceEncoding and the implementation of
 * GtkSourceFileLoader, this was not a problem: the file loader tried each
 * candidate GtkSourceEncoding one by one. But the new file loader
 * implementation uses an external library (uchardet) to determine the encoding.
 * uchardet returns an iconv-compatible charset, as a string, which is not
 * guaranteed to be in the table. So the charset passed to tepl_encoding_new()
 * is copied as-is, to not loose any information from uchardet. If the table
 * contains that charset, fine, we also have a name like "Unicode"; otherwise
 * it's not a problem, we have a TeplEncoding encapsulating the charset.
 *
 * It's a boxed type, not a GObject. Because signals are not needed, so a boxed
 * type is lighter. tepl_encoding_get_all() creates a lot of TeplEncoding's. It
 * would be slightly more convenient to have ref counting, but TeplEncoding can
 * be seen as a string: instead of g_strdup()/g_free(), it's
 * tepl_encoding_copy()/tepl_encoding_free().
 */

struct _TeplEncoding
{
	/* Must never be NULL. */
	gchar *charset;

	/* The name stored here must already be translated.
	 * NULL if unknown.
	 */
	gchar *translated_name;
};

typedef struct _EncodingData EncodingData;
struct _EncodingData
{
	const gchar *charset;

	/* The name stored here is not yet translated and must be translated. */
	const gchar *name_to_translate;
};

#define UTF8_CANONICAL_FORM "UTF-8"

G_DEFINE_BOXED_TYPE (TeplEncoding, tepl_encoding,
                     tepl_encoding_copy,
                     tepl_encoding_free)

/* This table should not contain duplicates: iconv supports for example "utf8",
 * "UTF8", "utf-8" and "UTF-8", they are equivalent (as far as I've tested) but
 * the table contains only "UTF-8". As a result, a function like
 * tepl_encoding_get_all() doesn't return duplicates, which is important to not
 * try several times the same encoding when loading a file, or to not show
 * duplicated encodings in a GtkComboBox when choosing manually an encoding.
 *
 * The original version of this table comes from profterm.
 * SPDX-FileCopyrightText: (C) 2002 Red Hat, Inc.
 */
static const EncodingData encodings_table[] =
{
	/* UTF-8 first, so that it's the first encoding returned by
	 * tepl_encoding_get_all().
	 */
	{ UTF8_CANONICAL_FORM, N_("Unicode") },

	{ "ISO-8859-1", N_("Western") },
	{ "ISO-8859-2", N_("Central European") },
	{ "ISO-8859-3", N_("South European") },
	{ "ISO-8859-4", N_("Baltic") },
	{ "ISO-8859-5", N_("Cyrillic") },
	{ "ISO-8859-6", N_("Arabic") },
	{ "ISO-8859-7", N_("Greek") },
	{ "ISO-8859-8", N_("Hebrew Visual") },
	{ "ISO-8859-9", N_("Turkish") },
	{ "ISO-8859-10", N_("Nordic") },
	{ "ISO-8859-13", N_("Baltic") },
	{ "ISO-8859-14", N_("Celtic") },
	{ "ISO-8859-15", N_("Western") },
	{ "ISO-8859-16", N_("Romanian") },

	{ "UTF-7", N_("Unicode") },
	{ "UTF-16", N_("Unicode") },
	{ "UTF-16BE", N_("Unicode") },
	{ "UTF-16LE", N_("Unicode") },
	{ "UTF-32", N_("Unicode") },
	{ "UCS-2", N_("Unicode") },
	{ "UCS-4", N_("Unicode") },

	{ "ARMSCII-8", N_("Armenian") },
	{ "BIG5", N_("Chinese Traditional") },
	{ "BIG5-HKSCS", N_("Chinese Traditional") },
	{ "CP866", N_("Cyrillic/Russian") },

	{ "EUC-JP", N_("Japanese") },
	{ "EUC-JP-MS", N_("Japanese") },
	{ "CP932", N_("Japanese") },

	{ "EUC-KR", N_("Korean") },
	{ "EUC-TW", N_("Chinese Traditional") },

	{ "GB18030", N_("Chinese Simplified") },
	{ "GB2312", N_("Chinese Simplified") },
	{ "GBK", N_("Chinese Simplified") },
	{ "GEORGIAN-ACADEMY", N_("Georgian") }, /* FIXME GEOSTD8 ? */

	{ "IBM850", N_("Western") },
	{ "IBM852", N_("Central European") },
	{ "IBM855", N_("Cyrillic") },
	{ "IBM857", N_("Turkish") },
	{ "IBM862", N_("Hebrew") },
	{ "IBM864", N_("Arabic") },

	{ "ISO-2022-JP", N_("Japanese") },
	{ "ISO-2022-KR", N_("Korean") },
	{ "ISO-IR-111", N_("Cyrillic") },
	{ "JOHAB", N_("Korean") },
	{ "KOI8R", N_("Cyrillic") },
	{ "KOI8-R", N_("Cyrillic") },
	{ "KOI8U", N_("Cyrillic/Ukrainian") },

	{ "SHIFT_JIS", N_("Japanese") },
	{ "TCVN", N_("Vietnamese") },
	{ "TIS-620", N_("Thai") },
	{ "UHC", N_("Korean") },
	{ "VISCII", N_("Vietnamese") },

	{ "WINDOWS-1250", N_("Central European") },
	{ "WINDOWS-1251", N_("Cyrillic") },
	{ "WINDOWS-1252", N_("Western") },
	{ "WINDOWS-1253", N_("Greek") },
	{ "WINDOWS-1254", N_("Turkish") },
	{ "WINDOWS-1255", N_("Hebrew") },
	{ "WINDOWS-1256", N_("Arabic") },
	{ "WINDOWS-1257", N_("Baltic") },
	{ "WINDOWS-1258", N_("Vietnamese") }
};

static TeplEncoding *
_tepl_encoding_new_full (const gchar *charset,
			 const gchar *translated_name)
{
	TeplEncoding *enc;

	g_assert (charset != NULL);

	enc = g_new (TeplEncoding, 1);
	enc->charset = g_strdup (charset);
	enc->translated_name = g_strdup (translated_name);

	return enc;
}

/**
 * tepl_encoding_copy:
 * @enc: a #TeplEncoding.
 *
 * Returns: (transfer full): a copy of @enc.
 * Since: 2.0
 */
TeplEncoding *
tepl_encoding_copy (const TeplEncoding *enc)
{
	g_return_val_if_fail (enc != NULL, NULL);

	return _tepl_encoding_new_full (enc->charset,
					enc->translated_name);
}

/**
 * tepl_encoding_free:
 * @enc: (nullable): a #TeplEncoding, or %NULL.
 *
 * Since: 2.0
 */
void
tepl_encoding_free (TeplEncoding *enc)
{
	if (enc != NULL)
	{
		g_free (enc->charset);
		g_free (enc->translated_name);
		g_free (enc);
	}
}

static gboolean
is_utf8_charset (const gchar *charset)
{
	return (g_ascii_strcasecmp (charset, "UTF-8") == 0 ||
		g_ascii_strcasecmp (charset, "UTF8") == 0);
}

static const gchar *
get_translated_name (const gchar *charset)
{
	guint i;

	if (is_utf8_charset (charset))
	{
		charset = UTF8_CANONICAL_FORM;
	}

	for (i = 0; i < G_N_ELEMENTS (encodings_table); i++)
	{
		const EncodingData *cur_data = encodings_table + i;

		if (g_ascii_strcasecmp (cur_data->charset, charset) == 0)
		{
			return _(cur_data->name_to_translate);
		}
	}

	if (g_ascii_strcasecmp (charset, "ANSI_X3.4-1968") == 0)
	{
		/* US-ASCII is an equivalent charset. Since it's a charset, we
		 * do not translate it.
		 */
		return "US-ASCII";
	}

	return NULL;
}

/**
 * tepl_encoding_new:
 * @charset: a character set.
 *
 * Creates a new #TeplEncoding from a character set such as "UTF-8" or
 * "ISO-8859-1".
 *
 * The tepl_encoding_get_charset() function will return exactly the same string
 * as the @charset passed in to this constructor.
 *
 * Returns: the new #TeplEncoding. Free with tepl_encoding_free().
 * Since: 2.0
 */
TeplEncoding *
tepl_encoding_new (const gchar *charset)
{
	const gchar *translated_name;

	g_return_val_if_fail (charset != NULL, NULL);

	translated_name = get_translated_name (charset);

	return _tepl_encoding_new_full (charset, translated_name);
}

/**
 * tepl_encoding_new_utf8:
 *
 * Creates a new #TeplEncoding with the "UTF-8" character set.
 *
 * Returns: the new #TeplEncoding. Free with tepl_encoding_free().
 * Since: 2.0
 */
TeplEncoding *
tepl_encoding_new_utf8 (void)
{
	return tepl_encoding_new (UTF8_CANONICAL_FORM);
}

/**
 * tepl_encoding_new_from_locale:
 *
 * Creates a new #TeplEncoding from the current locale, as returned by
 * g_get_charset().
 *
 * Returns: the new #TeplEncoding. Free with tepl_encoding_free().
 * Since: 2.0
 */
TeplEncoding *
tepl_encoding_new_from_locale (void)
{
	const gchar *locale_charset;

	if (g_get_charset (&locale_charset))
	{
		return tepl_encoding_new_utf8 ();
	}

	return tepl_encoding_new (locale_charset);
}

/**
 * tepl_encoding_get_charset:
 * @enc: a #TeplEncoding.
 *
 * Gets the character set of the #TeplEncoding, such as "UTF-8" or "ISO-8859-1".
 *
 * Returns: the character set of the #TeplEncoding.
 * Since: 2.0
 */
const gchar *
tepl_encoding_get_charset (const TeplEncoding *enc)
{
	g_return_val_if_fail (enc != NULL, NULL);
	g_assert (enc->charset != NULL);

	return enc->charset;
}

/**
 * tepl_encoding_get_name:
 * @enc: a #TeplEncoding.
 *
 * Gets the name of the #TeplEncoding such as "Unicode" or "Western". If the
 * charset is unknown by #TeplEncoding, "Unknown" is returned. The return value
 * is already translated by gettext.
 *
 * Returns: the name of the #TeplEncoding.
 * Since: 2.0
 */
const gchar *
tepl_encoding_get_name (const TeplEncoding *enc)
{
	g_return_val_if_fail (enc != NULL, NULL);

	return (enc->translated_name != NULL) ? enc->translated_name : _("Unknown");
}

/**
 * tepl_encoding_to_string:
 * @enc: a #TeplEncoding.
 *
 * Returns the encoding name with the charset in parenthesis, for example
 * "Unicode (UTF-8)". If the name is unknown, just the charset is returned. The
 * name is translated by gettext.
 *
 * Returns: a string representation. Free with g_free() when no longer needed.
 * Since: 2.0
 */
gchar *
tepl_encoding_to_string (const TeplEncoding *enc)
{
	g_return_val_if_fail (enc != NULL, NULL);
	g_assert (enc->charset != NULL);

	if (enc->translated_name != NULL)
	{
		return g_strdup_printf ("%s (%s)", enc->translated_name, enc->charset);
	}

	return g_strdup (enc->charset);
}

/**
 * tepl_encoding_is_utf8:
 * @enc: a #TeplEncoding.
 *
 * Returns whether @enc is a UTF-8 encoding.
 *
 * If @enc was created with tepl_encoding_new_utf8(), the charset is "UTF-8".
 * But iconv supports other variants: "UTF8", "utf-8" and "utf8". This function
 * returns %TRUE for all UTF-8 variants supported by iconv.
 *
 * Returns: whether @enc is a UTF-8 encoding.
 * Since: 2.0
 */
gboolean
tepl_encoding_is_utf8 (const TeplEncoding *enc)
{
	g_return_val_if_fail (enc != NULL, FALSE);
	g_assert (enc->charset != NULL);

	return is_utf8_charset (enc->charset);
}

/**
 * tepl_encoding_equals:
 * @enc1: (nullable): a #TeplEncoding, or %NULL.
 * @enc2: (nullable): a #TeplEncoding, or %NULL.
 *
 * Returns whether @enc1 and @enc2 are equal. It returns %TRUE iff:
 * - Both @enc1 and @enc2 are %NULL;
 * - Or both @enc1 and @enc2 have a UTF-8 charset (see
 *   tepl_encoding_is_utf8());
 * - Or the charsets are equal according to g_ascii_strcasecmp() (because
 *   iconv-compatible charsets are case insensitive).
 *
 * Returns: whether @enc1 and @enc2 are equal.
 * Since: 2.0
 */
gboolean
tepl_encoding_equals (const TeplEncoding *enc1,
		      const TeplEncoding *enc2)
{
	if (enc1 == NULL || enc2 == NULL)
	{
		return enc1 == NULL && enc2 == NULL;
	}

	g_assert (enc1->charset != NULL);
	g_assert (enc2->charset != NULL);

	if (tepl_encoding_is_utf8 (enc1) &&
	    tepl_encoding_is_utf8 (enc2))
	{
		return TRUE;
	}

	return g_ascii_strcasecmp (enc1->charset, enc2->charset) == 0;
}

static gboolean
present_in_list (const GSList       *list,
		 const TeplEncoding *enc)
{
	const GSList *l;

	for (l = list; l != NULL; l = l->next)
	{
		const TeplEncoding *cur_enc = l->data;

		if (tepl_encoding_equals (cur_enc, enc))
		{
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * tepl_encoding_get_all:
 *
 * Gets a list of all encodings known by #TeplEncoding.
 *
 * Returns: (transfer full) (element-type TeplEncoding): a list of
 * #TeplEncoding's.
 * Since: 2.0
 */
GSList *
tepl_encoding_get_all (void)
{
	GSList *list = NULL;
	TeplEncoding *locale_enc;
	gint i;

	for (i = G_N_ELEMENTS (encodings_table) - 1; i >= 0; i--)
	{
		const EncodingData *cur_data = encodings_table + i;
		TeplEncoding *enc;

		enc = _tepl_encoding_new_full (cur_data->charset,
					       _(cur_data->name_to_translate));

		list = g_slist_prepend (list, enc);
	}

	/* Add locale encoding first. */

	locale_enc = tepl_encoding_new_from_locale ();

	if (present_in_list (list, locale_enc))
	{
		tepl_encoding_free (locale_enc);
	}
	else
	{
		list = g_slist_prepend (list, locale_enc);
	}

	return list;
}

static GSList *
remove_duplicates_keep_first (GSList *list)
{
	GSList *new_list = NULL;
	GSList *l;

	for (l = list; l != NULL; l = l->next)
	{
		TeplEncoding *cur_enc = l->data;

		if (present_in_list (new_list, cur_enc))
		{
			tepl_encoding_free (cur_enc);
		}
		else
		{
			new_list = g_slist_prepend (new_list, cur_enc);
		}
	}

	new_list = g_slist_reverse (new_list);

	g_slist_free (list);
	return new_list;
}

static GSList *
remove_duplicates_keep_last (GSList *list)
{
	GSList *new_list = NULL;
	GSList *l;

	list = g_slist_reverse (list);

	for (l = list; l != NULL; l = l->next)
	{
		TeplEncoding *cur_enc = l->data;

		if (present_in_list (new_list, cur_enc))
		{
			tepl_encoding_free (cur_enc);
		}
		else
		{
			new_list = g_slist_prepend (new_list, cur_enc);
		}
	}

	g_slist_free (list);
	return new_list;
}

/*
 * _tepl_encoding_remove_duplicates:
 * @list: (transfer full) (element-type TeplEncoding): a list of
 * #TeplEncoding's.
 * @removal_type: the #TeplEncodingDuplicates.
 *
 * A convenience function to remove duplicated encodings in a list.
 *
 * Returns: (transfer full) (element-type TeplEncoding): the new start of the
 * #GSList.
 * Since: 2.0
 */
GSList *
_tepl_encoding_remove_duplicates (GSList                 *list,
				  TeplEncodingDuplicates  removal_type)
{
	switch (removal_type)
	{
		case TEPL_ENCODING_DUPLICATES_KEEP_FIRST:
			return remove_duplicates_keep_first (list);

		case TEPL_ENCODING_DUPLICATES_KEEP_LAST:
			return remove_duplicates_keep_last (list);

		default:
			break;
	}

	g_return_val_if_reached (list);
}

/* Returns: (transfer full) (element-type TeplEncoding). */
static GSList *
default_candidates_strv_to_list (const gchar * const *enc_str)
{
	GSList *all_encodings;
	GSList *list = NULL;
	gchar **p;

	all_encodings = tepl_encoding_get_all ();

	for (p = (gchar **)enc_str; p != NULL && *p != NULL; p++)
	{
		const gchar *charset = *p;
		TeplEncoding *enc;

		if (g_str_equal (charset, "CURRENT"))
		{
			enc = tepl_encoding_new_from_locale ();
		}
		else
		{
			enc = tepl_encoding_new (charset);
		}

		if (present_in_list (list, enc))
		{
			tepl_encoding_free (enc);
			continue;
		}

		/* If the translator has not translated correctly the list of
		 * default candidate encodings, ensure that it won't trigger a
		 * file loading error.
		 */
		if (!present_in_list (all_encodings, enc))
		{
			g_warning ("TeplEncoding: unknown charset '%s', "
				   "ignoring it for the default candidates.",
				   charset);

			tepl_encoding_free (enc);
			continue;
		}

		list = g_slist_prepend (list, enc);
	}

	g_slist_free_full (all_encodings, (GDestroyNotify)tepl_encoding_free);

	return g_slist_reverse (list);
}

/**
 * tepl_encoding_get_default_candidates:
 *
 * Gets the list of default candidate encodings to try when loading a file. See
 * gtk_source_file_loader_set_candidate_encodings().
 *
 * This function returns a different list depending on the current locale (i.e.
 * language, country and default encoding). The UTF-8 encoding and the current
 * locale encoding are guaranteed to be present in the returned list.
 *
 * Note that the returned list doesn't contain all encodings known by
 * #TeplEncoding, it is a limited list that contains only the encodings that
 * have the most likelihood to fit for the current locale.
 *
 * Returns: (transfer full) (element-type TeplEncoding): the list of default
 * candidate encodings.
 * Since: 2.0
 */
/* TODO s/gtk_source_file_loader_set_candidate_encodings/tepl_.../ when the
 * latter exists.
 */
GSList *
tepl_encoding_get_default_candidates (void)
{
	const gchar *encodings_str;
	const gchar *encodings_str_translated;
	GVariant *encodings_variant;
	const gchar **encodings_strv;
	GSList *encodings_list;
	GError *error = NULL;

	/* Translators: This is the sorted list of encodings used by Tepl for
	 * automatic detection of the file encoding. You may want to customize
	 * it adding encodings that are common in your country, for instance the
	 * GB18030 encoding for the Chinese translation. You may also want to
	 * remove the ISO-8859-15 encoding (covering English and most Western
	 * European languages) if you think people in your country will rarely
	 * use it. 'CURRENT' is a magic value used by Tepl and it represents
	 * the encoding for the current locale, so please don't translate the
	 * 'CURRENT' term. Keep the same format: square brackets, single quotes,
	 * commas.
	 */
	encodings_str = N_("['UTF-8', 'CURRENT', 'ISO-8859-15', 'UTF-16']");

	encodings_str_translated = _(encodings_str);

	encodings_variant = g_variant_parse (G_VARIANT_TYPE_STRING_ARRAY,
					     encodings_str_translated,
					     NULL,
					     NULL,
					     &error);

	if (error != NULL)
	{
		const gchar * const *language_names = g_get_language_names ();

		g_warning ("Error while parsing encodings list for locale %s:\n"
			   "Translated list: %s\n"
			   "Error message: %s",
			   language_names[0],
			   encodings_str_translated,
			   error->message);

		g_clear_error (&error);

		encodings_variant = g_variant_parse (G_VARIANT_TYPE_STRING_ARRAY,
						     encodings_str,
						     NULL,
						     NULL,
						     &error);

		g_assert_no_error (error);
	}

	g_variant_ref_sink (encodings_variant);

	encodings_strv = g_variant_get_strv (encodings_variant, NULL);
	encodings_list = default_candidates_strv_to_list (encodings_strv);
	g_free ((gpointer) encodings_strv);

	/* Ensure that UTF-8 and CURRENT are present. */
	encodings_list = g_slist_prepend (encodings_list, tepl_encoding_new_from_locale ());
	encodings_list = g_slist_prepend (encodings_list, tepl_encoding_new_utf8 ());
	encodings_list = _tepl_encoding_remove_duplicates (encodings_list,
							   TEPL_ENCODING_DUPLICATES_KEEP_LAST);

	g_variant_unref (encodings_variant);
	return encodings_list;
}
