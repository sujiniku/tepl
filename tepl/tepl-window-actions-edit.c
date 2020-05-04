/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "tepl-window-actions-edit.h"
#include <amtk/amtk.h>
#include "tepl-tab-group.h"
#include "tepl-view.h"

/* TeplApplicationWindow GActions for the Edit menu. */

struct _TeplWindowActionsEdit
{
	TeplApplicationWindow *tepl_window; /* unowned */
};

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

	return window_actions_edit;
}

static void
window_actions_edit_free (TeplWindowActionsEdit *window_actions_edit)
{
	if (window_actions_edit == NULL)
	{
		return;
	}

	g_free (window_actions_edit);
}

void
_tepl_window_actions_edit_clear (TeplWindowActionsEdit **window_actions_edit_p)
{
	g_return_if_fail (window_actions_edit_p != NULL);

	window_actions_edit_free (*window_actions_edit_p);
	*window_actions_edit_p = NULL;
}
