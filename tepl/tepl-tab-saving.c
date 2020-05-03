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

#include "config.h"
#include "tepl-tab-saving.h"
#include <glib/gi18n-lib.h>
#include "tepl-file.h"
#include "tepl-info-bar.h"

/* The functions in this file permits to run a TeplFileSaver, shows
 * TeplInfoBar's, until the operation is successful or if there is an
 * unrecoverable error.
 *
 * If this becomes a class, a good name would be TeplTabSaver.
 */

static void
launch_saver_cb (GObject      *source_object,
		 GAsyncResult *result,
		 gpointer      user_data)
{
	TeplFileSaver *saver = TEPL_FILE_SAVER (source_object);
	GTask *task = G_TASK (user_data);
	TeplTab *tab;
	GApplication *app;
	GError *error = NULL;
	gboolean success;

	tab = g_task_get_source_object (task);

	success = tepl_file_saver_save_finish (saver, result, &error);

	if (success)
	{
		TeplFile *file;

		file = tepl_file_saver_get_file (saver);
		tepl_file_add_uri_to_recent_manager (file);
	}

	if (error != NULL)
	{
		TeplInfoBar *info_bar;

		info_bar = tepl_info_bar_new_simple (GTK_MESSAGE_ERROR,
						     _("Error when saving the file."),
						     error->message);
		tepl_info_bar_add_close_button (info_bar);
		tepl_tab_add_info_bar (tab, GTK_INFO_BAR (info_bar));
		gtk_widget_show (GTK_WIDGET (info_bar));

		g_clear_error (&error);
	}

	app = g_application_get_default ();
	g_application_unmark_busy (app);
	g_application_release (app);

	g_task_return_boolean (task, success);
	g_object_unref (task);
}

static void
launch_saver (GTask *task)
{
	TeplFileSaver *saver;
	GApplication *app;

	saver = g_task_get_task_data (task);

	app = g_application_get_default ();
	g_application_hold (app);
	g_application_mark_busy (app);

	tepl_file_saver_save_async (saver,
				    G_PRIORITY_DEFAULT,
				    NULL,
				    NULL,
				    NULL,
				    NULL,
				    launch_saver_cb,
				    task);
}

void
_tepl_tab_saving_save_async (TeplTab             *tab,
			     TeplFileSaver       *saver,
			     GAsyncReadyCallback  callback,
			     gpointer             user_data)
{
	GTask *task;

	g_return_if_fail (TEPL_IS_TAB (tab));
	g_return_if_fail (TEPL_IS_FILE_SAVER (saver));

	task = g_task_new (tab, NULL, callback, user_data);
	g_task_set_task_data (task,
			      g_object_ref (saver),
			      g_object_unref);

	launch_saver (task);
}

gboolean
_tepl_tab_saving_save_finish (TeplTab      *tab,
			      GAsyncResult *result)
{
	g_return_val_if_fail (TEPL_IS_TAB (tab), FALSE);
	g_return_val_if_fail (g_task_is_valid (result, tab), FALSE);

	return g_task_propagate_boolean (G_TASK (result), NULL);
}

static void
save_async_simple_cb (GObject      *source_object,
		      GAsyncResult *result,
		      gpointer      user_data)
{
	TeplTab *tab = TEPL_TAB (source_object);

	_tepl_tab_saving_save_finish (tab, result);
	g_object_unref (tab);
}

/* Useful when we don't care about:
 * - when the operation is finished.
 * - the result.
 */
void
_tepl_tab_saving_save_async_simple (TeplTab       *tab,
				    TeplFileSaver *saver)
{
	g_return_if_fail (TEPL_IS_TAB (tab));
	g_return_if_fail (TEPL_IS_FILE_SAVER (saver));

	g_object_ref (tab);
	_tepl_tab_saving_save_async (tab,
				     saver,
				     save_async_simple_cb,
				     NULL);
}
