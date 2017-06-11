/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * Tepl is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * Tepl is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TEPL_TAB_H
#define TEPL_TAB_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TEPL_TYPE_TAB (tepl_tab_get_type ())
G_DECLARE_DERIVABLE_TYPE (TeplTab, tepl_tab,
			  TEPL, TAB,
			  GtkGrid)

struct _TeplTabClass
{
	GtkGridClass parent_class;

	gpointer padding[12];
};

TeplTab *		tepl_tab_new					(GtkWidget *main_widget);

void			tepl_tab_add_info_bar				(TeplTab    *tab,
									 GtkInfoBar *info_bar);

G_END_DECLS

#endif /* TEPL_TAB_H */
