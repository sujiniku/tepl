/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * Gtef is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * Gtef is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GTEF_INFO_BAR_H
#define GTEF_INFO_BAR_H

#if !defined (GTEF_H_INSIDE) && !defined (GTEF_COMPILATION)
#error "Only <gtef/gtef.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTEF_TYPE_INFO_BAR (gtef_info_bar_get_type ())
G_DECLARE_DERIVABLE_TYPE (GtefInfoBar, gtef_info_bar,
			  GTEF, INFO_BAR,
			  GtkInfoBar)

struct _GtefInfoBarClass
{
	GtkInfoBarClass parent_class;

	gpointer padding[12];
};

GtefInfoBar *		gtef_info_bar_new				(void);

void			gtef_info_bar_add_icon				(GtefInfoBar *info_bar);

void			gtef_info_bar_add_primary_message		(GtefInfoBar *info_bar,
									 const gchar *primary_msg);

void			gtef_info_bar_add_secondary_message		(GtefInfoBar *info_bar,
									 const gchar *secondary_msg);

GtkLabel *		gtef_info_bar_create_label			(void);

G_END_DECLS

#endif /* GTEF_INFO_BAR_H */
