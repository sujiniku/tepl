/* SPDX-FileCopyrightText: 2002-2005 - Paolo Maggi
 * SPDX-FileCopyrightText: 2014, 2015, 2017 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_ENCODING_H
#define TEPL_ENCODING_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <glib-object.h>
#include <tepl/tepl-macros.h>

G_BEGIN_DECLS

#define TEPL_TYPE_ENCODING (tepl_encoding_get_type ())

typedef struct _TeplEncoding TeplEncoding;

_TEPL_EXTERN
GType			tepl_encoding_get_type			(void) G_GNUC_CONST;

_TEPL_EXTERN
TeplEncoding *		tepl_encoding_new			(const gchar *charset);

_TEPL_EXTERN
TeplEncoding *		tepl_encoding_new_utf8			(void);

_TEPL_EXTERN
TeplEncoding *		tepl_encoding_new_from_locale		(void);

_TEPL_EXTERN
TeplEncoding *		tepl_encoding_copy			(const TeplEncoding *enc);

_TEPL_EXTERN
void			tepl_encoding_free			(TeplEncoding *enc);

_TEPL_EXTERN
const gchar *		tepl_encoding_get_charset		(const TeplEncoding *enc);

_TEPL_EXTERN
const gchar *		tepl_encoding_get_name			(const TeplEncoding *enc);

_TEPL_EXTERN
gchar *			tepl_encoding_to_string			(const TeplEncoding *enc);

_TEPL_EXTERN
gboolean		tepl_encoding_is_utf8			(const TeplEncoding *enc);

_TEPL_EXTERN
gboolean		tepl_encoding_equals			(const TeplEncoding *enc1,
								 const TeplEncoding *enc2);

_TEPL_EXTERN
GSList *		tepl_encoding_get_all			(void);

_TEPL_EXTERN
GSList *		tepl_encoding_get_default_candidates	(void);

G_END_DECLS

#endif  /* TEPL_ENCODING_H */
