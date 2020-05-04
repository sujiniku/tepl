/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "tepl-window-actions-edit.h"
#include <amtk/amtk.h>
#include "tepl-signal-group.h"
#include "tepl-tab-group.h"
#include "tepl-view.h"

/* TeplApplicationWindow GActions for the Edit menu. */

struct _TeplWindowActionsEdit
{
	TeplApplicationWindow *tepl_window; /* unowned */

	TeplSignalGroup *tepl_window_signal_group;
	TeplSignalGroup *view_signal_group;
	TeplSignalGroup *buffer_signal_group;
};

/******************************************************************************/
/* Activate callbacks */

static void
undo_activate_cb (GSimpleAction *action,
		  GVariant      *parameter,
		  gpointer       user_data)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (user_data);
	TeplView *view;

	view = tepl_tab_group_get_active_view (TEPL_TAB_GROUP (tepl_window));

	if (view != NULL)
	{
		TeplBuffer *buffer;

		buffer = tepl_tab_group_get_active_buffer (TEPL_TAB_GROUP (tepl_window));

		gtk_source_buffer_undo (GTK_SOURCE_BUFFER (buffer));
		tepl_view_scroll_to_cursor (view);
		gtk_widget_grab_focus (GTK_WIDGET (view));
	}
}

static void
redo_activate_cb (GSimpleAction *action,
		  GVariant      *parameter,
		  gpointer       user_data)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (user_data);
	TeplView *view;

	view = tepl_tab_group_get_active_view (TEPL_TAB_GROUP (tepl_window));

	if (view != NULL)
	{
		TeplBuffer *buffer;

		buffer = tepl_tab_group_get_active_buffer (TEPL_TAB_GROUP (tepl_window));

		gtk_source_buffer_redo (GTK_SOURCE_BUFFER (buffer));
		tepl_view_scroll_to_cursor (view);
		gtk_widget_grab_focus (GTK_WIDGET (view));
	}
}

static void
cut_activate_cb (GSimpleAction *action,
		 GVariant      *parameter,
		 gpointer       user_data)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (user_data);
	TeplView *active_view;

	active_view = tepl_tab_group_get_active_view (TEPL_TAB_GROUP (tepl_window));

	if (active_view != NULL)
	{
		tepl_view_cut_clipboard (active_view);
	}
}

static void
copy_activate_cb (GSimpleAction *action,
		  GVariant      *parameter,
		  gpointer       user_data)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (user_data);
	TeplView *active_view;

	active_view = tepl_tab_group_get_active_view (TEPL_TAB_GROUP (tepl_window));

	if (active_view != NULL)
	{
		tepl_view_copy_clipboard (active_view);
	}
}

static void
paste_activate_cb (GSimpleAction *action,
		   GVariant      *parameter,
		   gpointer       user_data)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (user_data);
	TeplView *active_view;

	active_view = tepl_tab_group_get_active_view (TEPL_TAB_GROUP (tepl_window));

	if (active_view != NULL)
	{
		tepl_view_paste_clipboard (active_view);
	}
}

static void
delete_activate_cb (GSimpleAction *action,
		    GVariant      *parameter,
		    gpointer       user_data)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (user_data);
	TeplView *active_view;

	active_view = tepl_tab_group_get_active_view (TEPL_TAB_GROUP (tepl_window));

	if (active_view != NULL)
	{
		tepl_view_delete_selection (active_view);
	}
}

static void
select_all_activate_cb (GSimpleAction *action,
			GVariant      *parameter,
			gpointer       user_data)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (user_data);
	TeplView *active_view;

	active_view = tepl_tab_group_get_active_view (TEPL_TAB_GROUP (tepl_window));

	if (active_view != NULL)
	{
		tepl_view_select_all (active_view);
	}
}

static void
indent_activate_cb (GSimpleAction *action,
		    GVariant      *parameter,
		    gpointer       user_data)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (user_data);
	TeplView *view;

	view = tepl_tab_group_get_active_view (TEPL_TAB_GROUP (tepl_window));

	if (view != NULL)
	{
		TeplBuffer *buffer;
		GtkTextIter start;
		GtkTextIter end;

		buffer = tepl_tab_group_get_active_buffer (TEPL_TAB_GROUP (tepl_window));
		gtk_text_buffer_get_selection_bounds (GTK_TEXT_BUFFER (buffer), &start, &end);
		gtk_source_view_indent_lines (GTK_SOURCE_VIEW (view), &start, &end);
	}
}

static void
unindent_activate_cb (GSimpleAction *action,
		      GVariant      *parameter,
		      gpointer       user_data)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (user_data);
	TeplView *view;

	view = tepl_tab_group_get_active_view (TEPL_TAB_GROUP (tepl_window));

	if (view != NULL)
	{
		TeplBuffer *buffer;
		GtkTextIter start;
		GtkTextIter end;

		buffer = tepl_tab_group_get_active_buffer (TEPL_TAB_GROUP (tepl_window));
		gtk_text_buffer_get_selection_bounds (GTK_TEXT_BUFFER (buffer), &start, &end);
		gtk_source_view_unindent_lines (GTK_SOURCE_VIEW (view), &start, &end);
	}
}

/******************************************************************************/
/* Update sensitivity */

static void
update_undo_redo_actions_sensitivity (TeplApplicationWindow *tepl_window)
{
	TeplView *view;
	gboolean view_is_editable = FALSE;
	GtkSourceBuffer *buffer;
	GActionMap *action_map;
	GAction *action;

	view = tepl_tab_group_get_active_view (TEPL_TAB_GROUP (tepl_window));
	if (view != NULL)
	{
		view_is_editable = gtk_text_view_get_editable (GTK_TEXT_VIEW (view));
	}

	buffer = GTK_SOURCE_BUFFER (tepl_tab_group_get_active_buffer (TEPL_TAB_GROUP (tepl_window)));

	action_map = G_ACTION_MAP (tepl_application_window_get_application_window (tepl_window));

	action = g_action_map_lookup_action (action_map, "tepl-undo");
	g_simple_action_set_enabled (G_SIMPLE_ACTION (action),
				     view_is_editable &&
				     buffer != NULL &&
				     gtk_source_buffer_can_undo (buffer));

	action = g_action_map_lookup_action (action_map, "tepl-redo");
	g_simple_action_set_enabled (G_SIMPLE_ACTION (action),
				     view_is_editable &&
				     buffer != NULL &&
				     gtk_source_buffer_can_redo (buffer));
}

static void
active_view_editable_notify_cb (GtkTextView           *active_view,
				GParamSpec            *pspec,
				TeplWindowActionsEdit *window_actions_edit)
{
	update_undo_redo_actions_sensitivity (window_actions_edit->tepl_window);
}

static void
active_view_changed (TeplWindowActionsEdit *window_actions_edit)
{
	TeplView *active_view;

	_tepl_signal_group_clear (&window_actions_edit->view_signal_group);

	active_view = tepl_tab_group_get_active_view (TEPL_TAB_GROUP (window_actions_edit->tepl_window));
	if (active_view == NULL)
	{
		goto end;
	}

	window_actions_edit->view_signal_group = _tepl_signal_group_new (G_OBJECT (active_view));

	_tepl_signal_group_add (window_actions_edit->view_signal_group,
				g_signal_connect (active_view,
						  "notify::editable",
						  G_CALLBACK (active_view_editable_notify_cb),
						  window_actions_edit));

end:
	update_undo_redo_actions_sensitivity (window_actions_edit->tepl_window);
}

static void
active_view_notify_cb (TeplApplicationWindow *tepl_window,
		       GParamSpec            *pspec,
		       TeplWindowActionsEdit *window_actions_edit)
{
	active_view_changed (window_actions_edit);
}

static void
active_buffer_can_undo_notify_cb (GtkSourceBuffer       *buffer,
				  GParamSpec            *pspec,
				  TeplWindowActionsEdit *window_actions_edit)
{
	update_undo_redo_actions_sensitivity (window_actions_edit->tepl_window);
}

static void
active_buffer_can_redo_notify_cb (GtkSourceBuffer       *buffer,
				  GParamSpec            *pspec,
				  TeplWindowActionsEdit *window_actions_edit)
{
	update_undo_redo_actions_sensitivity (window_actions_edit->tepl_window);
}

static void
active_buffer_changed (TeplWindowActionsEdit *window_actions_edit)
{
	TeplBuffer *active_buffer;

	_tepl_signal_group_clear (&window_actions_edit->buffer_signal_group);

	active_buffer = tepl_tab_group_get_active_buffer (TEPL_TAB_GROUP (window_actions_edit->tepl_window));
	if (active_buffer == NULL)
	{
		goto end;
	}

	window_actions_edit->buffer_signal_group = _tepl_signal_group_new (G_OBJECT (active_buffer));

	_tepl_signal_group_add (window_actions_edit->buffer_signal_group,
				g_signal_connect (active_buffer,
						  "notify::can-undo",
						  G_CALLBACK (active_buffer_can_undo_notify_cb),
						  window_actions_edit));

	_tepl_signal_group_add (window_actions_edit->buffer_signal_group,
				g_signal_connect (active_buffer,
						  "notify::can-redo",
						  G_CALLBACK (active_buffer_can_redo_notify_cb),
						  window_actions_edit));

end:
	update_undo_redo_actions_sensitivity (window_actions_edit->tepl_window);
}

static void
active_buffer_notify_cb (TeplApplicationWindow *tepl_window,
			 GParamSpec            *pspec,
			 TeplWindowActionsEdit *window_actions_edit)
{
	active_buffer_changed (window_actions_edit);
}

/******************************************************************************/
/* Public functions */

TeplWindowActionsEdit *
_tepl_window_actions_edit_new (TeplApplicationWindow *tepl_window)
{
	GtkApplicationWindow *gtk_window;
	TeplWindowActionsEdit *window_actions_edit;

	const GActionEntry entries[] = {
		{ "tepl-undo", undo_activate_cb },
		{ "tepl-redo", redo_activate_cb },
		{ "tepl-cut", cut_activate_cb },
		{ "tepl-copy", copy_activate_cb },
		{ "tepl-paste", paste_activate_cb },
		{ "tepl-delete", delete_activate_cb },
		{ "tepl-select-all", select_all_activate_cb },
		{ "tepl-indent", indent_activate_cb },
		{ "tepl-unindent", unindent_activate_cb },
	};

	g_return_val_if_fail (TEPL_IS_APPLICATION_WINDOW (tepl_window), NULL);

	gtk_window = tepl_application_window_get_application_window (tepl_window);
	amtk_action_map_add_action_entries_check_dups (G_ACTION_MAP (gtk_window),
						       entries,
						       G_N_ELEMENTS (entries),
						       tepl_window);

	window_actions_edit = g_new0 (TeplWindowActionsEdit, 1);
	window_actions_edit->tepl_window = tepl_window;
	window_actions_edit->tepl_window_signal_group = _tepl_signal_group_new (G_OBJECT (tepl_window));

	_tepl_signal_group_add (window_actions_edit->tepl_window_signal_group,
				g_signal_connect (tepl_window,
						  "notify::active-view",
						  G_CALLBACK (active_view_notify_cb),
						  window_actions_edit));

	_tepl_signal_group_add (window_actions_edit->tepl_window_signal_group,
				g_signal_connect (tepl_window,
						  "notify::active-buffer",
						  G_CALLBACK (active_buffer_notify_cb),
						  window_actions_edit));

	active_view_changed (window_actions_edit);
	active_buffer_changed (window_actions_edit);

	return window_actions_edit;
}

static void
window_actions_edit_free (TeplWindowActionsEdit *window_actions_edit)
{
	if (window_actions_edit == NULL)
	{
		return;
	}

	_tepl_signal_group_clear (&window_actions_edit->tepl_window_signal_group);
	_tepl_signal_group_clear (&window_actions_edit->view_signal_group);
	_tepl_signal_group_clear (&window_actions_edit->buffer_signal_group);
	g_free (window_actions_edit);
}

void
_tepl_window_actions_edit_clear (TeplWindowActionsEdit **window_actions_edit_p)
{
	g_return_if_fail (window_actions_edit_p != NULL);

	window_actions_edit_free (*window_actions_edit_p);
	*window_actions_edit_p = NULL;
}
