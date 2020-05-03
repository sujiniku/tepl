/* Copyright 2005 - Paolo Maggi
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
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
