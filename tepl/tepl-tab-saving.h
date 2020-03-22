/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2017 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef TEPL_TAB_SAVING_H
#define TEPL_TAB_SAVING_H

#include <gio/gio.h>
#include "tepl-tab.h"
#include "tepl-file-saver.h"

G_BEGIN_DECLS

G_GNUC_INTERNAL
void		_tepl_tab_saving_save_async		(TeplTab             *tab,
							 TeplFileSaver       *saver,
							 GAsyncReadyCallback  callback,
							 gpointer             user_data);

G_GNUC_INTERNAL
gboolean	_tepl_tab_saving_save_finish		(TeplTab      *tab,
							 GAsyncResult *result);

G_GNUC_INTERNAL
void		_tepl_tab_saving_save_async_simple	(TeplTab       *tab,
							 TeplFileSaver *saver);

G_END_DECLS

#endif /* TEPL_TAB_SAVING_H */
