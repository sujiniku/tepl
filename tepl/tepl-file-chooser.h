/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_FILE_CHOOSER_H
#define TEPL_FILE_CHOOSER_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <tepl/tepl-macros.h>

G_BEGIN_DECLS

_TEPL_EXTERN
void	tepl_file_chooser_set_modal		(GtkFileChooser *chooser,
						 gboolean        modal);

_TEPL_EXTERN
void	tepl_file_chooser_set_parent		(GtkFileChooser *chooser,
						 GtkWindow      *parent);

_TEPL_EXTERN
void	tepl_file_chooser_show			(GtkFileChooser *chooser);

G_END_DECLS

#endif /* TEPL_FILE_CHOOSER_H */
