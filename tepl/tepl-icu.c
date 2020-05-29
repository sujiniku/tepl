/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "tepl-icu.h"

/* Wrapper around u_strFromUTF8() that handles the pre-flighting.
 *
 * Returns: (transfer full) (nullable): the newly-allocated buffer with the
 * right size. Free with g_free() when no longer needed.
 */
UChar *
_tepl_icu_strFromUTF8 (int32_t    *pDestLength,
		       const char *src,
		       int32_t     srcLength,
		       UErrorCode *pErrorCode)
{
	int32_t my_DestLength = 0;
	UErrorCode my_ErrorCode = U_ZERO_ERROR;
	UChar *dest = NULL;

	u_strFromUTF8 (NULL, 0, &my_DestLength,
		       src, srcLength,
		       &my_ErrorCode);

	if (my_ErrorCode != U_BUFFER_OVERFLOW_ERROR &&
	    my_ErrorCode != U_STRING_NOT_TERMINATED_WARNING)
	{
		if (pDestLength != NULL)
		{
			*pDestLength = my_DestLength;
		}
		if (pErrorCode != NULL)
		{
			*pErrorCode = my_ErrorCode;
		}

		return NULL;
	}

	dest = g_new0 (UChar, my_DestLength + 1);

	u_strFromUTF8 (dest, my_DestLength + 1, pDestLength,
		       src, srcLength,
		       pErrorCode);

	return dest;
}

/* Wrapper around u_strToUTF8() that handles the pre-flighting.
 *
 * Returns: (transfer full) (nullable): the newly-allocated string with the
 * right size. Free with g_free() when no longer needed.
 */
char *
_tepl_icu_strToUTF8 (int32_t     *pDestLength,
		     const UChar *src,
		     int32_t      srcLength,
		     UErrorCode  *pErrorCode)
{
	int32_t my_DestLength = 0;
	UErrorCode my_ErrorCode = U_ZERO_ERROR;
	char *dest = NULL;

	u_strToUTF8 (NULL, 0, &my_DestLength,
		     src, srcLength,
		     &my_ErrorCode);

	if (my_ErrorCode != U_BUFFER_OVERFLOW_ERROR &&
	    my_ErrorCode != U_STRING_NOT_TERMINATED_WARNING)
	{
		if (pDestLength != NULL)
		{
			*pDestLength = my_DestLength;
		}
		if (pErrorCode != NULL)
		{
			*pErrorCode = my_ErrorCode;
		}

		return NULL;
	}

	dest = g_malloc0 (my_DestLength + 1);

	u_strToUTF8 (dest, my_DestLength + 1, pDestLength,
		     src, srcLength,
		     pErrorCode);

	return dest;
}

/* Returns: (transfer full) (nullable): a nul-terminated UTF-16 string. Free
 * with g_free() when no longer needed.
 */
UChar *
_tepl_icu_strFromUTF8Simple (const char *utf8_str)
{
	UChar *uchars;
	UErrorCode error_code = U_ZERO_ERROR;

	uchars = _tepl_icu_strFromUTF8 (NULL, utf8_str, -1, &error_code);

	if (U_FAILURE (error_code))
	{
		g_free (uchars);
		return NULL;
	}

	return uchars;
}

/* Returns: (transfer full) (nullable): a nul-terminated UTF-8 string. Free with
 * g_free() when no longer needed.
 */
char *
_tepl_icu_strToUTF8Simple (const UChar *uchars)
{
	char *utf8_str;
	UErrorCode error_code = U_ZERO_ERROR;

	utf8_str = _tepl_icu_strToUTF8 (NULL, uchars, -1, &error_code);

	if (U_FAILURE (error_code))
	{
		g_free (utf8_str);
		return NULL;
	}

	return utf8_str;
}

/* Returns: (transfer full) (nullable): a copy of @uchars. Free with g_free()
 * when no longer needed.
 */
UChar *
_tepl_icu_strdup (const UChar *uchars)
{
	int32_t length;
	UChar *copy;

	if (uchars == NULL)
	{
		return NULL;
	}

	length = u_strlen (uchars);
	copy = g_new0 (UChar, length + 1);

	return u_strncpy (copy, uchars, length + 1);
}

/* A wrapper around utrans_openU(). */
UTransliterator *
_tepl_icu_trans_openUSimple (const char *utf8_id)
{
	UChar *id;
	UTransliterator *transliterator;
	UErrorCode error_code = U_ZERO_ERROR;

	id = _tepl_icu_strFromUTF8Simple (utf8_id);
	g_return_val_if_fail (id != NULL, NULL);

	transliterator = utrans_openU (id, -1,
				       UTRANS_FORWARD,
				       NULL, 0,
				       NULL, &error_code);
	g_free (id);

	if (U_FAILURE (error_code))
	{
		g_warn_if_reached ();

		if (transliterator != NULL)
		{
			utrans_close (transliterator);
		}

		return NULL;
	}

	return transliterator;
}

UTransliterator *
_tepl_icu_trans_open_xml_escape (void)
{
	/* Don't escape all the characters, keep certain printable ASCII
	 * characters as is. That way it's a bit easier to understand when
	 * reading/debugging the XML content.
	 *
	 * The ICU transliterator/transform can be tested easily with the uconv
	 * command, including a round-trip:
	 * $ echo -n -e '\t' | uconv -x '[^a-zA-Z0-9.,;/_\x2D\x3A] Any-Hex/XML' | uconv -x 'Hex-Any/XML'
	 *
	 * "\\x2D" is '-' and "\\x3A" is ':'.
	 */
	return _tepl_icu_trans_openUSimple ("[^a-zA-Z0-9.,;/_\\x2D\\x3A] Any-Hex/XML");
}

/* Like utrans_transUChars(), but simpler to use.
 * @src must be nul-terminated, and is not modified.
 *
 * Returns: (transfer full) (nullable): the transformed string, as a
 * newly-allocated nul-terminated buffer of the right size. Free with g_free()
 * when no longer needed.
 */
UChar *
_tepl_icu_trans_transUCharsSimple (const UTransliterator *trans,
				   const UChar           *src)
{
	UChar *src_copy;
	int32_t src_length;
	int32_t text_length;
	int32_t limit;
	int32_t dest_capacity;
	UChar *dest;
	UErrorCode error_code = U_ZERO_ERROR;

	/* Pre-flighting */

	src_copy = _tepl_icu_strdup (src);
	src_length = u_strlen (src);
	text_length = src_length;

	limit = src_length;

	utrans_transUChars (trans,
			    src_copy, &text_length, src_length + 1,
			    0, &limit,
			    &error_code);

	g_free (src_copy);

	if (error_code != U_BUFFER_OVERFLOW_ERROR &&
	    error_code != U_STRING_NOT_TERMINATED_WARNING &&
	    U_FAILURE (error_code))
	{
		g_warn_if_reached ();
		return NULL;
	}

	/* Do the real transform */

	dest_capacity = MAX (text_length + 1, src_length + 1);
	dest = g_new0 (UChar, dest_capacity);
	u_strncpy (dest, src, src_length + 1);

	limit = src_length;
	error_code = U_ZERO_ERROR;

	utrans_transUChars (trans,
			    dest, NULL, dest_capacity,
			    0, &limit,
			    &error_code);

	if (U_FAILURE (error_code))
	{
		g_warn_if_reached ();
		g_free (dest);
		return NULL;
	}

	return dest;
}
