/* Copyright 2017 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_MENU_SHELL_H
#define TEPL_MENU_SHELL_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <tepl/tepl-macros.h>

G_BEGIN_DECLS

_TEPL_EXTERN
void	tepl_menu_shell_append_edit_actions	(GtkMenuShell *menu_shell);

G_END_DECLS

#endif /* TEPL_MENU_SHELL_H */
