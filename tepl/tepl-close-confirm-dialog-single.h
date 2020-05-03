/* Copyright 2017 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_CLOSE_CONFIRM_DIALOG_SINGLE_H
#define TEPL_CLOSE_CONFIRM_DIALOG_SINGLE_H

#include <gtk/gtk.h>
#include "tepl-tab.h"

G_BEGIN_DECLS

G_GNUC_INTERNAL
void		_tepl_close_confirm_dialog_single_async		(TeplTab             *tab,
								 GAsyncReadyCallback  callback,
								 gpointer             user_data);

G_GNUC_INTERNAL
gboolean	_tepl_close_confirm_dialog_single_finish	(TeplTab      *tab,
								 GAsyncResult *result);

G_END_DECLS

#endif /* TEPL_CLOSE_CONFIRM_DIALOG_SINGLE_H */
