/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_TAB_LOADING_H
#define TEPL_TAB_LOADING_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <tepl/tepl-tab.h>

G_BEGIN_DECLS

_TEPL_EXTERN
void	tepl_tab_load_file	(TeplTab *tab,
				 GFile   *location);

G_END_DECLS

#endif /* TEPL_TAB_LOADING_H */
