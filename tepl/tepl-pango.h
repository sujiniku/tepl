/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_PANGO_H
#define TEPL_PANGO_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <tepl/tepl-macros.h>
#include <pango/pango.h>

G_BEGIN_DECLS

_TEPL_EXTERN
gchar *		tepl_pango_font_description_to_css	(const PangoFontDescription *desc);

G_END_DECLS

#endif /* TEPL_PANGO_H */
