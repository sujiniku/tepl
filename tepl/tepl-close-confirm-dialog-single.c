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

#include "tepl-close-confirm-dialog-single.h"
#include "tepl-tab.h"

/* When closing a TeplTab, show a message dialog if the buffer is modified. */

void
_tepl_close_confirm_dialog_single_async (TeplTab             *tab,
					 GAsyncReadyCallback  callback,
					 gpointer             user_data)
{
	GTask *task;

	g_return_if_fail (TEPL_IS_TAB (tab));

	task = g_task_new (tab, NULL, callback, user_data);

	g_task_return_boolean (task, FALSE);
	g_object_unref (task);
}

/* Returns: %TRUE if @tab can be closed, %FALSE otherwise. */
gboolean
_tepl_close_confirm_dialog_single_finish (TeplTab      *tab,
					  GAsyncResult *result)
{
	g_return_val_if_fail (TEPL_IS_TAB (tab), FALSE);
	g_return_val_if_fail (g_task_is_valid (result, tab), FALSE);

	return g_task_propagate_boolean (G_TASK (result), NULL);
}
