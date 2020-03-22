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

#include "config.h"
#include "tepl-close-confirm-dialog-single.h"
#include <glib/gi18n-lib.h>
#include "tepl-buffer.h"
#include "tepl-file.h"
#include "tepl-utils.h"

#define CAN_CLOSE (TRUE)
#define CANNOT_CLOSE (FALSE)

#define DIALOG_RESPONSE_SAVE	(1)
#define DIALOG_RESPONSE_SAVE_AS	(2)

/* When closing a TeplTab, show a message dialog if the buffer is modified. */

static void
save_tab_cb (GObject      *source_object,
	     GAsyncResult *result,
	     gpointer      user_data)
{
	TeplTab *tab = TEPL_TAB (source_object);
	GTask *task = G_TASK (user_data);
	gboolean can_close;

	can_close = tepl_tab_save_finish (tab, result);

	g_task_return_boolean (task, can_close);
	g_object_unref (task);
}

static void
save_tab (GTask *task)
{
	TeplTab *tab;

	tab = g_task_get_source_object (task);
	tepl_tab_save_async (tab, save_tab_cb, task);
}

static void
save_as_tab_cb (GObject      *source_object,
		GAsyncResult *result,
		gpointer      user_data)
{
	TeplTab *tab = TEPL_TAB (source_object);
	GTask *task = G_TASK (user_data);
	gboolean can_close;

	can_close = tepl_tab_save_as_finish (tab, result);

	g_task_return_boolean (task, can_close);
	g_object_unref (task);
}

static void
save_as_tab (GTask *task)
{
	TeplTab *tab;

	tab = g_task_get_source_object (task);
	tepl_tab_save_as_async (tab, save_as_tab_cb, task);
}

static void
dialog_response_cb (GtkDialog *dialog,
		    gint       response_id,
		    GTask     *task)
{
	if (response_id == DIALOG_RESPONSE_SAVE)
	{
		save_tab (task);
	}
	else if (response_id == DIALOG_RESPONSE_SAVE_AS)
	{
		save_as_tab (task);
	}
	else if (response_id == GTK_RESPONSE_CLOSE)
	{
		g_task_return_boolean (task, CAN_CLOSE);
		g_object_unref (task);
	}
	else
	{
		g_task_return_boolean (task, CANNOT_CLOSE);
		g_object_unref (task);
	}

	gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
create_dialog (GTask *task)
{
	TeplTab *tab;
	TeplBuffer *buffer;
	TeplFile *file;
	GFile *location;
	const gchar *file_short_name;
	GtkWidget *dialog;

	tab = g_task_get_source_object (task);
	buffer = tepl_tab_get_buffer (tab);
	file = tepl_buffer_get_file (buffer);
	location = tepl_file_get_location (file);

	file_short_name = tepl_file_get_short_name (file);

	dialog = gtk_message_dialog_new (NULL,
					 GTK_DIALOG_DESTROY_WITH_PARENT |
					 GTK_DIALOG_MODAL,
					 GTK_MESSAGE_WARNING,
					 GTK_BUTTONS_NONE,
					 _("Save changes to file “%s” before closing?"),
					 file_short_name);

	gtk_dialog_add_buttons (GTK_DIALOG (dialog),
				_("Close _without Saving"), GTK_RESPONSE_CLOSE,
				_("_Cancel"), GTK_RESPONSE_CANCEL,
				NULL);

	if (location != NULL)
	{
		gtk_dialog_add_button (GTK_DIALOG (dialog),
				       _("_Save"),
				       DIALOG_RESPONSE_SAVE);
	}
	else
	{
		gtk_dialog_add_button (GTK_DIALOG (dialog),
				       _("_Save As…"),
				       DIALOG_RESPONSE_SAVE_AS);
	}

	_tepl_utils_associate_secondary_window (GTK_WINDOW (dialog),
						GTK_WIDGET (tab));

	g_signal_connect (dialog,
			  "response",
			  G_CALLBACK (dialog_response_cb),
			  task);

	gtk_widget_show (dialog);
}

void
_tepl_close_confirm_dialog_single_async (TeplTab             *tab,
					 GAsyncReadyCallback  callback,
					 gpointer             user_data)
{
	GTask *task;
	TeplBuffer *buffer;

	g_return_if_fail (TEPL_IS_TAB (tab));

	task = g_task_new (tab, NULL, callback, user_data);

	buffer = tepl_tab_get_buffer (tab);
	if (!gtk_text_buffer_get_modified (GTK_TEXT_BUFFER (buffer)))
	{
		g_task_return_boolean (task, CAN_CLOSE);
		g_object_unref (task);
		return;
	}

	create_dialog (task);
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
