/* Copyright 2017 - Sébastien Wilmet <swilmet@gnome.org>
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
