/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2005 - Paolo Maggi
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

#ifndef TEPL_PROGRESS_INFO_BAR_H
#define TEPL_PROGRESS_INFO_BAR_H

#include <gtk/gtk.h>
#include "tepl-info-bar.h"

G_BEGIN_DECLS

#define TEPL_TYPE_PROGRESS_INFO_BAR (_tepl_progress_info_bar_get_type ())
G_DECLARE_FINAL_TYPE (TeplProgressInfoBar, _tepl_progress_info_bar,
		      TEPL, PROGRESS_INFO_BAR,
		      TeplInfoBar)

G_GNUC_INTERNAL
TeplProgressInfoBar *	_tepl_progress_info_bar_new			(const gchar *markup,
									 gboolean     has_cancel_button);

G_GNUC_INTERNAL
void			_tepl_progress_info_bar_set_markup		(TeplProgressInfoBar *info_bar,
									 const gchar         *markup);

G_GNUC_INTERNAL
void			_tepl_progress_info_bar_set_text		(TeplProgressInfoBar *info_bar,
									 const gchar         *text);

G_GNUC_INTERNAL
void			_tepl_progress_info_bar_set_fraction		(TeplProgressInfoBar *info_bar,
									 gdouble              fraction);

G_GNUC_INTERNAL
void			_tepl_progress_info_bar_pulse			(TeplProgressInfoBar *info_bar);

G_END_DECLS

#endif /* TEPL_PROGRESS_INFO_BAR_H */
