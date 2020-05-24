/* SPDX-FileCopyrightText: 2017-2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_TAB_SAVING_H
#define TEPL_TAB_SAVING_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <tepl/tepl-tab.h>
#include <tepl/tepl-file-saver.h>

G_BEGIN_DECLS

G_GNUC_INTERNAL
void		_tepl_tab_saving_save_async		(TeplTab             *tab,
							 TeplFileSaver       *saver,
							 GAsyncReadyCallback  callback,
							 gpointer             user_data);

G_GNUC_INTERNAL
gboolean	_tepl_tab_saving_save_finish		(TeplTab      *tab,
							 GAsyncResult *result);

_TEPL_EXTERN
void		tepl_tab_save_async		(TeplTab             *tab,
						 GAsyncReadyCallback  callback,
						 gpointer             user_data);

_TEPL_EXTERN
gboolean	tepl_tab_save_finish		(TeplTab      *tab,
						 GAsyncResult *result);

_TEPL_EXTERN
void		tepl_tab_save_async_simple	(TeplTab *tab);

_TEPL_EXTERN
void		tepl_tab_save_as_async		(TeplTab             *tab,
						 GAsyncReadyCallback  callback,
						 gpointer             user_data);

_TEPL_EXTERN
gboolean	tepl_tab_save_as_finish		(TeplTab      *tab,
						 GAsyncResult *result);

_TEPL_EXTERN
void		tepl_tab_save_as_async_simple	(TeplTab *tab);

G_END_DECLS

#endif /* TEPL_TAB_SAVING_H */
