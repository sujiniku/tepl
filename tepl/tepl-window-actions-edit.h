/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_WINDOW_ACTIONS_EDIT_H
#define TEPL_WINDOW_ACTIONS_EDIT_H

#include "tepl-application-window.h"

G_BEGIN_DECLS

typedef struct _TeplWindowActionsEdit TeplWindowActionsEdit;

G_GNUC_INTERNAL
TeplWindowActionsEdit *	_tepl_window_actions_edit_new	(TeplApplicationWindow *tepl_window);

G_GNUC_INTERNAL
void			_tepl_window_actions_edit_clear	(TeplWindowActionsEdit **window_actions_edit_p);

G_END_DECLS

#endif /* TEPL_WINDOW_ACTIONS_EDIT_H */
