/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_ICU_H
#define TEPL_ICU_H

#include <glib.h>
#include <unicode/ustring.h>
#include <unicode/utrans.h>

G_BEGIN_DECLS

G_GNUC_INTERNAL
UChar *			_tepl_icu_strFromUTF8			(int32_t    *pDestLength,
								 const char *src,
								 int32_t     srcLength,
								 UErrorCode *pErrorCode);

G_GNUC_INTERNAL
char *			_tepl_icu_strToUTF8			(int32_t     *pDestLength,
								 const UChar *src,
								 int32_t      srcLength,
								 UErrorCode  *pErrorCode);

G_GNUC_INTERNAL
UChar *			_tepl_icu_strFromUTF8Simple		(const char *utf8_str);

G_GNUC_INTERNAL
char *			_tepl_icu_strToUTF8Simple		(const UChar *uchars);

G_GNUC_INTERNAL
UChar *			_tepl_icu_strdup			(const UChar *uchars);

G_GNUC_INTERNAL
UTransliterator *	_tepl_icu_trans_openUSimple		(const char *utf8_id);

G_GNUC_INTERNAL
UTransliterator *	_tepl_icu_trans_open_xml_escape		(void);

G_GNUC_INTERNAL
UChar *			_tepl_icu_trans_transUCharsSimple	(const UTransliterator *trans,
								 const UChar           *src);

G_END_DECLS

#endif /* TEPL_ICU_H */
