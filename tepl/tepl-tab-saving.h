/* SPDX-FileCopyrightText: 2017 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
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
