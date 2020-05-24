/* SPDX-FileCopyrightText: 2017-2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "config.h"
#include "tepl-tab-saving.h"
#include <glib/gi18n-lib.h>
#include "tepl-file.h"
#include "tepl-info-bar.h"
#include "tepl-utils.h"

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

/**
 * tepl_tab_save_async:
 * @tab: a #TeplTab.
 * @callback: (scope async): a #GAsyncReadyCallback to call when the request is
 *   satisfied.
 * @user_data: user data to pass to @callback.
 *
 * Saves asynchronously the content of the @tab. The #TeplFile:location must not
 * be %NULL.
 *
 * See the #GAsyncResult documentation to know how to use this function.
 *
 * Since: 4.0
 */
void
tepl_tab_save_async (TeplTab             *tab,
		     GAsyncReadyCallback  callback,
		     gpointer             user_data)
{
	TeplBuffer *buffer;
	TeplFile *file;
	GFile *location;
	TeplFileSaver *saver;

	g_return_if_fail (TEPL_IS_TAB (tab));

	buffer = tepl_tab_get_buffer (tab);
	file = tepl_buffer_get_file (buffer);
	location = tepl_file_get_location (file);
	g_return_if_fail (location != NULL);

	saver = tepl_file_saver_new (buffer, file);
	_tepl_tab_saving_save_async (tab, saver, callback, user_data);
	g_object_unref (saver);
}

/**
 * tepl_tab_save_finish:
 * @tab: a #TeplTab.
 * @result: a #GAsyncResult.
 *
 * Finishes a tab saving started with tepl_tab_save_async().
 *
 * Returns: whether the tab was saved successfully.
 * Since: 4.0
 */
gboolean
tepl_tab_save_finish (TeplTab      *tab,
		      GAsyncResult *result)
{
	return _tepl_tab_saving_save_finish (tab, result);
}

static void
save_async_simple_cb (GObject      *source_object,
		      GAsyncResult *result,
		      gpointer      user_data)
{
	TeplTab *tab = TEPL_TAB (source_object);

	tepl_tab_save_finish (tab, result);
	g_object_unref (tab);
}

/**
 * tepl_tab_save_async_simple:
 * @tab: a #TeplTab.
 *
 * The same as tepl_tab_save_async(), but without callback.
 *
 * This function is useful when you don't need to know:
 * - when the operation is finished;
 * - and whether the operation ran successfully.
 *
 * Since: 4.0
 */
void
tepl_tab_save_async_simple (TeplTab *tab)
{
	g_return_if_fail (TEPL_IS_TAB (tab));

	g_object_ref (tab);
	tepl_tab_save_async (tab,
			     save_async_simple_cb,
			     NULL);
}

static void
save_as_cb (GObject      *source_object,
	    GAsyncResult *result,
	    gpointer      user_data)
{
	TeplTab *tab = TEPL_TAB (source_object);
	GTask *task = G_TASK (user_data);
	gboolean ok;

	ok = _tepl_tab_saving_save_finish (tab, result);

	g_task_return_boolean (task, ok);
	g_object_unref (task);
}

static void
save_file_chooser_response_cb (GtkFileChooserDialog *file_chooser_dialog,
			       gint                  response_id,
			       GTask                *task)
{
	if (response_id == GTK_RESPONSE_ACCEPT)
	{
		TeplTab *tab;
		TeplBuffer *buffer;
		TeplFile *file;
		GFile *location;
		TeplFileSaver *saver;

		tab = g_task_get_source_object (task);
		buffer = tepl_tab_get_buffer (tab);
		file = tepl_buffer_get_file (buffer);

		location = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (file_chooser_dialog));

		saver = tepl_file_saver_new_with_target (buffer, file, location);
		g_object_unref (location);

		_tepl_tab_saving_save_async (tab, saver, save_as_cb, task);
		g_object_unref (saver);
	}
	else
	{
		g_task_return_boolean (task, FALSE);
		g_object_unref (task);
	}

	gtk_widget_destroy (GTK_WIDGET (file_chooser_dialog));
}

/**
 * tepl_tab_save_as_async:
 * @tab: a #TeplTab.
 * @callback: (scope async): a #GAsyncReadyCallback to call when the request is
 *   satisfied.
 * @user_data: user data to pass to @callback.
 *
 * Shows a #GtkFileChooser to save the @tab to a different location, creates an
 * appropriate #TeplFileSaver and asynchronously runs it.
 *
 * See the #GAsyncResult documentation to know how to use this function.
 *
 * Since: 4.0
 */
void
tepl_tab_save_as_async (TeplTab             *tab,
			GAsyncReadyCallback  callback,
			gpointer             user_data)
{
	GTask *task;
	GtkWidget *file_chooser_dialog;
	GtkFileChooser *file_chooser;

	g_return_if_fail (TEPL_IS_TAB (tab));

	task = g_task_new (tab, NULL, callback, user_data);

	file_chooser_dialog = gtk_file_chooser_dialog_new (_("Save File"),
							   NULL,
							   GTK_FILE_CHOOSER_ACTION_SAVE,
							   _("_Cancel"), GTK_RESPONSE_CANCEL,
							   _("_Save"), GTK_RESPONSE_ACCEPT,
							   NULL);

	gtk_dialog_set_default_response (GTK_DIALOG (file_chooser_dialog), GTK_RESPONSE_ACCEPT);

	/* Prevent tab from being destroyed. */
	gtk_window_set_modal (GTK_WINDOW (file_chooser_dialog), TRUE);

	_tepl_utils_associate_secondary_window (GTK_WINDOW (file_chooser_dialog),
						GTK_WIDGET (tab));

	file_chooser = GTK_FILE_CHOOSER (file_chooser_dialog);

	gtk_file_chooser_set_do_overwrite_confirmation (file_chooser, TRUE);
	gtk_file_chooser_set_local_only (file_chooser, FALSE);

	g_signal_connect (file_chooser_dialog,
			  "response",
			  G_CALLBACK (save_file_chooser_response_cb),
			  task);

	gtk_widget_show (file_chooser_dialog);
}

/**
 * tepl_tab_save_as_finish:
 * @tab: a #TeplTab.
 * @result: a #GAsyncResult.
 *
 * Finishes a tab saving started with tepl_tab_save_as_async().
 *
 * Returns: whether the tab was saved successfully.
 * Since: 4.0
 */
gboolean
tepl_tab_save_as_finish (TeplTab      *tab,
			 GAsyncResult *result)
{
	g_return_val_if_fail (TEPL_IS_TAB (tab), FALSE);
	g_return_val_if_fail (g_task_is_valid (result, tab), FALSE);

	return g_task_propagate_boolean (G_TASK (result), NULL);
}

static void
save_as_async_simple_cb (GObject      *source_object,
			 GAsyncResult *result,
			 gpointer      user_data)
{
	TeplTab *tab = TEPL_TAB (source_object);

	tepl_tab_save_as_finish (tab, result);
	g_object_unref (tab);
}

/**
 * tepl_tab_save_as_async_simple:
 * @tab: a #TeplTab.
 *
 * The same as tepl_tab_save_as_async(), but without callback.
 *
 * This function is useful when you don't need to know:
 * - when the operation is finished;
 * - and whether the operation ran successfully.
 *
 * Since: 4.0
 */
void
tepl_tab_save_as_async_simple (TeplTab *tab)
{
	g_return_if_fail (TEPL_IS_TAB (tab));

	g_object_ref (tab);
	tepl_tab_save_as_async (tab,
				save_as_async_simple_cb,
				NULL);
}
