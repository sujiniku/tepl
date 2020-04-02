/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2016-2020 - Sébastien Wilmet <swilmet@gnome.org>
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
#include "tepl-tab.h"
#include <glib/gi18n-lib.h>
#include "tepl-close-confirm-dialog-single.h"
#include "tepl-file-loader.h"
#include "tepl-file-saver.h"
#include "tepl-info-bar.h"
#include "tepl-tab-group.h"
#include "tepl-tab-saving.h"
#include "tepl-utils.h"

/**
 * SECTION:tab
 * @Short_description: Contains a TeplView and GtkInfoBars
 * @Title: TeplTab
 * @See_also: #TeplInfoBar
 *
 * #TeplTab is meant to be the content of one tab in the text editor (if the
 * text editor has a Tabbed Document Interface). It is a #GtkGrid container that
 * contains the #TeplView and can contain one or several #GtkInfoBar's. Since it
 * is a #GtkGrid, an application can of course add any other widget to it.
 *
 * To create a new #GtkInfoBar, it is recommended to use #TeplInfoBar (but
 * #TeplTab doesn't enforce it).
 *
 * By default:
 * - #TeplTab has a vertical #GtkOrientation.
 * - The main child widget of #TeplTab is a #GtkScrolledWindow which contains
 *   the #TeplView.
 * - #GtkInfoBar's are added on top of the #GtkScrolledWindow.
 *
 * The way that the #TeplView is packed into the #TeplTab is customizable with
 * the ::pack_view virtual function. Similarly, the way that #GtkInfoBar's are
 * added can be customized with ::pack_info_bar.
 *
 * # TeplTabGroup implementation
 *
 * #TeplTab implements the #TeplTabGroup interface, for a group of only one tab.
 * It is useful for text editors that open each file in a separate window, or
 * for applications that don't require to open more than one file. But the
 * tepl_tab_group_append_tab() operation is not supported, so some higher-level
 * features of Tepl don't work with #TeplTab as the #TeplTabGroup of the window.
 * This will maybe be improved in the future by creating automatically a new
 * window.
 */

struct _TeplTabPrivate
{
	TeplView *view;
};

enum
{
	PROP_0,
	PROP_VIEW,
	PROP_ACTIVE_TAB,
	PROP_ACTIVE_VIEW,
	PROP_ACTIVE_BUFFER,
};

enum
{
	SIGNAL_CLOSE_REQUEST,
	N_SIGNALS
};

static guint signals[N_SIGNALS];

static void tepl_tab_group_interface_init (gpointer g_iface,
					   gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE (TeplTab,
			 tepl_tab,
			 GTK_TYPE_GRID,
			 G_ADD_PRIVATE (TeplTab)
			 G_IMPLEMENT_INTERFACE (TEPL_TYPE_TAB_GROUP,
						tepl_tab_group_interface_init))

static GtkScrolledWindow *
create_scrolled_window (void)
{
	GtkScrolledWindow *scrolled_window;

	scrolled_window = GTK_SCROLLED_WINDOW (gtk_scrolled_window_new (NULL, NULL));

	/* Disable overlay scrolling, it doesn't work well with GtkTextView. For
	 * example to place the cursor with the mouse on the last character of a
	 * line.
	 */
	gtk_scrolled_window_set_overlay_scrolling (scrolled_window, FALSE);

	g_object_set (scrolled_window,
		      "expand", TRUE,
		      NULL);

	gtk_widget_show (GTK_WIDGET (scrolled_window));

	return scrolled_window;
}

static void
tepl_tab_pack_view_default (TeplTab  *tab,
			    TeplView *view)
{
	GtkScrolledWindow *scrolled_window;

	scrolled_window = create_scrolled_window ();

	gtk_container_add (GTK_CONTAINER (scrolled_window),
			   GTK_WIDGET (view));

	gtk_container_add (GTK_CONTAINER (tab),
			   GTK_WIDGET (scrolled_window));
}

static void
tepl_tab_pack_info_bar_default (TeplTab    *tab,
				GtkInfoBar *info_bar)
{
	GList *children;
	GList *l;
	GtkWidget *sibling = NULL;

	children = gtk_container_get_children (GTK_CONTAINER (tab));

	for (l = children; l != NULL; l = l->next)
	{
		GtkWidget *child = l->data;

		if (!GTK_IS_INFO_BAR (child))
		{
			sibling = child;
			break;
		}
	}

	g_list_free (children);

	if (sibling != NULL)
	{
		gtk_grid_insert_next_to (GTK_GRID (tab), sibling, GTK_POS_TOP);

		gtk_grid_attach_next_to (GTK_GRID (tab),
					 GTK_WIDGET (info_bar),
					 sibling,
					 GTK_POS_TOP,
					 1, 1);
	}
	else
	{
		gtk_container_add (GTK_CONTAINER (tab), GTK_WIDGET (info_bar));
	}
}

static void
close_confirm_dialog_single_cb (GObject      *source_object,
				GAsyncResult *result,
				gpointer      user_data)
{
	TeplTab *tab = TEPL_TAB (source_object);

	if (_tepl_close_confirm_dialog_single_finish (tab, result))
	{
		gtk_widget_destroy (GTK_WIDGET (tab));
	}
}

static void
tepl_tab_close_request_default (TeplTab *tab)
{
	_tepl_close_confirm_dialog_single_async (tab, close_confirm_dialog_single_cb, NULL);
}

static void
buffer_notify_cb (GtkTextView *view,
		  GParamSpec  *pspec,
		  TeplTab     *tab)
{
	g_object_notify (G_OBJECT (tab), "active-buffer");
}

static void
set_view (TeplTab  *tab,
	  TeplView *view)
{
	if (view == NULL)
	{
		/* For tepl_tab_new(). */
		view = TEPL_VIEW (tepl_view_new ());
		gtk_widget_show (GTK_WIDGET (view));
	}

	g_return_if_fail (TEPL_IS_VIEW (view));

	g_assert (tab->priv->view == NULL);
	tab->priv->view = g_object_ref_sink (view);

	TEPL_TAB_GET_CLASS (tab)->pack_view (tab, view);

	g_signal_connect_object (view,
				 "notify::buffer",
				 G_CALLBACK (buffer_notify_cb),
				 tab,
				 0);

	g_object_notify (G_OBJECT (tab), "view");
}

static void
tepl_tab_get_property (GObject    *object,
                       guint       prop_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
	TeplTab *tab = TEPL_TAB (object);
	TeplTabGroup *tab_group = TEPL_TAB_GROUP (object);

	switch (prop_id)
	{
		case PROP_VIEW:
			g_value_set_object (value, tepl_tab_get_view (tab));
			break;

		case PROP_ACTIVE_TAB:
			g_value_set_object (value, tepl_tab_group_get_active_tab (tab_group));
			break;

		case PROP_ACTIVE_VIEW:
			g_value_set_object (value, tepl_tab_group_get_active_view (tab_group));
			break;

		case PROP_ACTIVE_BUFFER:
			g_value_set_object (value, tepl_tab_group_get_active_buffer (tab_group));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_tab_set_property (GObject      *object,
                       guint         prop_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
	TeplTab *tab = TEPL_TAB (object);
	TeplTabGroup *tab_group = TEPL_TAB_GROUP (object);

	switch (prop_id)
	{
		case PROP_VIEW:
			set_view (tab, g_value_get_object (value));
			break;

		case PROP_ACTIVE_TAB:
			tepl_tab_group_set_active_tab (tab_group, g_value_get_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_tab_dispose (GObject *object)
{
	TeplTab *tab = TEPL_TAB (object);

	g_clear_object (&tab->priv->view);

	G_OBJECT_CLASS (tepl_tab_parent_class)->dispose (object);
}

static void
tepl_tab_class_init (TeplTabClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = tepl_tab_get_property;
	object_class->set_property = tepl_tab_set_property;
	object_class->dispose = tepl_tab_dispose;

	klass->pack_view = tepl_tab_pack_view_default;
	klass->pack_info_bar = tepl_tab_pack_info_bar_default;
	klass->close_request = tepl_tab_close_request_default;

	/**
	 * TeplTab:view:
	 *
	 * The #TeplView contained in the tab. When this property is set, the
	 * ::pack_view virtual function is called.
	 *
	 * Since: 3.0
	 */
	g_object_class_install_property (object_class,
					 PROP_VIEW,
					 g_param_spec_object ("view",
							      "View",
							      "",
							      TEPL_TYPE_VIEW,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY |
							      G_PARAM_STATIC_STRINGS));

	g_object_class_override_property (object_class, PROP_ACTIVE_TAB, "active-tab");
	g_object_class_override_property (object_class, PROP_ACTIVE_VIEW, "active-view");
	g_object_class_override_property (object_class, PROP_ACTIVE_BUFFER, "active-buffer");

	/**
	 * TeplTab::close-request:
	 * @tab: the #TeplTab emitting the signal.
	 *
	 * The ::close-request signal is emitted when there is a request to
	 * close the #TeplTab, for example if the user clicks on a close button.
	 *
	 * The default object method handler does the following:
	 * - If the buffer is not modified (according to
	 *   gtk_text_buffer_get_modified()), close the tab.
	 * - Else, show a message dialog to propose to save the file before
	 *   closing.
	 *
	 * To override the default object method handler, either override the
	 * virtual function in a #TeplTab subclass or connect to the signal and
	 * call g_signal_stop_emission_by_name().
	 *
	 * Since: 3.0
	 */
	signals[SIGNAL_CLOSE_REQUEST] =
		g_signal_new ("close-request",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (TeplTabClass, close_request),
			      NULL, NULL, NULL,
			      G_TYPE_NONE, 0);
}

static GList *
tepl_tab_get_tabs (TeplTabGroup *tab_group)
{
	return g_list_append (NULL, TEPL_TAB (tab_group));
}

static TeplTab *
tepl_tab_get_active_tab (TeplTabGroup *tab_group)
{
	return TEPL_TAB (tab_group);
}

static void
tepl_tab_group_interface_init (gpointer g_iface,
			       gpointer iface_data)
{
	TeplTabGroupInterface *interface = g_iface;

	interface->get_tabs = tepl_tab_get_tabs;
	interface->get_active_tab = tepl_tab_get_active_tab;
}

static void
tepl_tab_init (TeplTab *tab)
{
	tab->priv = tepl_tab_get_instance_private (tab);

	gtk_orientable_set_orientation (GTK_ORIENTABLE (tab), GTK_ORIENTATION_VERTICAL);
}

/**
 * tepl_tab_new:
 *
 * Creates a new #TeplTab with a new #TeplView. The new #TeplView can be
 * retrieved afterwards with tepl_tab_get_view().
 *
 * Returns: a new #TeplTab.
 * Since: 3.0
 */
TeplTab *
tepl_tab_new (void)
{
	return g_object_new (TEPL_TYPE_TAB, NULL);
}

/**
 * tepl_tab_new_with_view:
 * @view: the #TeplView that will be contained in the tab.
 *
 * Returns: a new #TeplTab.
 * Since: 3.0
 */
TeplTab *
tepl_tab_new_with_view (TeplView *view)
{
	g_return_val_if_fail (TEPL_IS_VIEW (view), NULL);

	return g_object_new (TEPL_TYPE_TAB,
			     "view", view,
			     NULL);
}

/**
 * tepl_tab_get_view:
 * @tab: a #TeplTab.
 *
 * Returns: (transfer none): the #TeplView contained in @tab.
 * Since: 3.0
 */
TeplView *
tepl_tab_get_view (TeplTab *tab)
{
	g_return_val_if_fail (TEPL_IS_TAB (tab), NULL);

	return tab->priv->view;
}

/**
 * tepl_tab_get_buffer:
 * @tab: a #TeplTab.
 *
 * A convenience function that calls gtk_text_view_get_buffer() on the
 * #TeplTab:view associated with the @tab.
 *
 * Returns: (transfer none): the #TeplBuffer of the #TeplTab:view.
 * Since: 3.0
 */
TeplBuffer *
tepl_tab_get_buffer (TeplTab *tab)
{
	g_return_val_if_fail (TEPL_IS_TAB (tab), NULL);

	return TEPL_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (tab->priv->view)));
}

/**
 * tepl_tab_add_info_bar:
 * @tab: a #TeplTab.
 * @info_bar: a #GtkInfoBar.
 *
 * Attaches @info_bar to @tab.
 *
 * This function calls the ::pack_info_bar virtual function.
 *
 * Since: 1.0
 */
void
tepl_tab_add_info_bar (TeplTab    *tab,
		       GtkInfoBar *info_bar)
{
	g_return_if_fail (TEPL_IS_TAB (tab));
	g_return_if_fail (GTK_IS_INFO_BAR (info_bar));

	_tepl_info_bar_set_size_request (info_bar);

	TEPL_TAB_GET_CLASS (tab)->pack_info_bar (tab, info_bar);
}

static void
load_file_content_cb (GObject      *source_object,
		      GAsyncResult *result,
		      gpointer      user_data)
{
	TeplFileLoader *loader = TEPL_FILE_LOADER (source_object);
	TeplTab *tab = TEPL_TAB (user_data);
	GError *error = NULL;

	if (tepl_file_loader_load_finish (loader, result, &error))
	{
		TeplBuffer *buffer;
		TeplFile *file;

		buffer = tepl_tab_get_buffer (tab);
		file = tepl_buffer_get_file (buffer);
		tepl_file_add_uri_to_recent_manager (file);
	}

	if (error != NULL)
	{
		TeplInfoBar *info_bar;

		info_bar = tepl_info_bar_new_simple (GTK_MESSAGE_ERROR,
						     _("Error when loading the file."),
						     error->message);

		tepl_tab_add_info_bar (tab, GTK_INFO_BAR (info_bar));
		gtk_widget_show (GTK_WIDGET (info_bar));

		g_clear_error (&error);
	}

	g_object_unref (loader);
	g_object_unref (tab);
}

/**
 * tepl_tab_load_file:
 * @tab: a #TeplTab.
 * @location: a #GFile.
 *
 * Unconditionally loads a file in @tab, regardless if there are unsaved changes
 * in the #GtkTextBuffer. The previous buffer content is lost.
 *
 * This function is asynchronous, there is no way to know when the file loading
 * is finished.
 *
 * Since: 4.0
 */
void
tepl_tab_load_file (TeplTab *tab,
		    GFile   *location)
{
	TeplBuffer *buffer;
	TeplFile *file;
	TeplFileLoader *loader;

	g_return_if_fail (TEPL_IS_TAB (tab));
	g_return_if_fail (G_IS_FILE (location));

	buffer = tepl_tab_get_buffer (tab);
	file = tepl_buffer_get_file (buffer);

	tepl_file_set_location (file, location);
	loader = tepl_file_loader_new (buffer, file);

	tepl_file_loader_load_async (loader,
				     G_PRIORITY_DEFAULT,
				     NULL, /* cancellable */
				     NULL, NULL, NULL, /* progress */
				     load_file_content_cb,
				     g_object_ref (tab));
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
