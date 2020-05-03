/* Copyright 2017 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_INIT_H
#define TEPL_INIT_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <glib.h>
#include <tepl/tepl-macros.h>

G_BEGIN_DECLS

_TEPL_EXTERN
void	tepl_init		(void);
_TEPL_EXTERN
void	tepl_finalize		(void);

G_END_DECLS

#endif /* TEPL_INIT_H */
