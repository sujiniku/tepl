/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2005 - Paolo Maggi
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GTEF_PROGRESS_INFO_BAR_H
#define GTEF_PROGRESS_INFO_BAR_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTEF_TYPE_PROGRESS_INFO_BAR (_gtef_progress_info_bar_get_type ())
G_DECLARE_FINAL_TYPE (GtefProgressInfoBar, _gtef_progress_info_bar,
		      GTEF, PROGRESS_INFO_BAR,
		      GtkInfoBar)

G_GNUC_INTERNAL
GtefProgressInfoBar *	_gtef_progress_info_bar_new			(const gchar *markup,
									 gboolean     has_cancel_button);

G_GNUC_INTERNAL
void			_gtef_progress_info_bar_set_markup		(GtefProgressInfoBar *info_bar,
									 const gchar         *markup);

G_GNUC_INTERNAL
void			_gtef_progress_info_bar_set_text		(GtefProgressInfoBar *info_bar,
									 const gchar         *text);

G_GNUC_INTERNAL
void			_gtef_progress_info_bar_set_fraction		(GtefProgressInfoBar *info_bar,
									 gdouble              fraction);

G_GNUC_INTERNAL
void			_gtef_progress_info_bar_pulse			(GtefProgressInfoBar *info_bar);

G_END_DECLS

#endif /* GTEF_PROGRESS_INFO_BAR_H */
