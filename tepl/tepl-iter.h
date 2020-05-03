/* Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_ITER_H
#define TEPL_ITER_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <tepl/tepl-macros.h>

G_BEGIN_DECLS

_TEPL_EXTERN
gchar *		tepl_iter_get_line_indentation		(const GtkTextIter *iter);

G_END_DECLS

#endif /* TEPL_ITER_H */
