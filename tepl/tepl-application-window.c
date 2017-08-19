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

#include "tepl-application-window.h"
#include <amtk/amtk.h>
#include "tepl-abstract-factory.h"
#include "tepl-tab-group.h"
#include "tepl-view.h"
#include "tepl-signal-group.h"

/**
 * SECTION:application-window
 * @Short_description: An extension of GtkApplicationWindow
 * @Title: TeplApplicationWindow
 *
 * #TeplApplicationWindow extends the #GtkApplicationWindow class.
 *
 * An application needs to call tepl_application_window_set_tab_group() to
 * benefit from the #TeplTabGroup interface implemented by this class.
 *
 * Note that #TeplApplicationWindow extends the #GtkApplicationWindow class but
 * without subclassing it, because several libraries might want to extend
 * #GtkApplicationWindow and an application needs to be able to use all those
 * extensions at the same time.
 *
 * # GActions # {#tepl-application-window-gactions}
 *
 * This class adds the following #GAction's to the #GtkApplicationWindow.
 * Corresponding #AmtkActionInfo's are available with
 * tepl_application_get_tepl_action_info_store().
 *
 * ## For the File menu
 *
 * - `"win.tepl-new-file"`: creates a new #TeplTab, appends it with
 *   tepl_tab_group_append_tab() and set it as the active tab.
 *
 * ## For the Edit menu
 *
 * The following actions require the %AMTK_FACTORY_IGNORE_ACCELS_FOR_APP flag,
 * because otherwise accelerators don't work in other text widgets than the
 * active view (e.g. in a #GtkEntry):
 * - `"win.tepl-cut"`: calls tepl_view_cut_clipboard() on the active view.
 * - `"win.tepl-copy"`: calls tepl_view_copy_clipboard() on the active view.
 * - `"win.tepl-paste"`: calls tepl_view_paste_clipboard() on the active view.
 * - `"win.tepl-delete"`: calls tepl_view_delete_selection() on the active view.
 * - `"win.tepl-select-all"`: calls tepl_view_select_all() on the active view.
 *
 * See the tepl_menu_shell_append_edit_actions() convenience function.
 */

struct _TeplApplicationWindowPrivate
{
	GtkApplicationWindow *gtk_window;
	TeplTabGroup *tab_group;
	TeplSignalGroup *view_signal_group;
	TeplSignalGroup *buffer_signal_group;
};

enum
{
	PROP_0,
	PROP_APPLICATION_WINDOW,
	PROP_ACTIVE_TAB,
	PROP_ACTIVE_VIEW,
	PROP_ACTIVE_BUFFER,
};

#define TEPL_APPLICATION_WINDOW_KEY "tepl-application-window-key"

static void tepl_tab_group_interface_init (gpointer g_iface,
					   gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE (TeplApplicationWindow,
			 tepl_application_window,
			 G_TYPE_OBJECT,
			 G_ADD_PRIVATE (TeplApplicationWindow)
			 G_IMPLEMENT_INTERFACE (TEPL_TYPE_TAB_GROUP,
						tepl_tab_group_interface_init))

static void
new_file_cb (GSimpleAction *action,
	     GVariant      *parameter,
	     gpointer       user_data)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (user_data);
	TeplAbstractFactory *factory;
	TeplTab *new_tab;

	factory = tepl_abstract_factory_get_singleton ();
	new_tab = tepl_abstract_factory_create_tab (factory);
	gtk_widget_show (GTK_WIDGET (new_tab));

	tepl_tab_group_append_tab (TEPL_TAB_GROUP (tepl_window), new_tab, TRUE);
}

static void
cut_cb (GSimpleAction *action,
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
copy_cb (GSimpleAction *action,
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
paste_cb (GSimpleAction *action,
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
delete_cb (GSimpleAction *action,
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
select_all_cb (GSimpleAction *action,
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

/* @can_paste_according_to_clipboard: TRUE if calling
 * tepl_view_paste_clipboard() will paste something.
 */
static void
set_paste_action_sensitivity_according_to_clipboard (TeplApplicationWindow *tepl_window,
						     gboolean               can_paste_according_to_clipboard)
{
	TeplView *view;
	gboolean view_is_editable = FALSE;
	GActionMap *action_map;
	GAction *action;

	view = tepl_tab_group_get_active_view (TEPL_TAB_GROUP (tepl_window));

	if (view != NULL)
	{
		view_is_editable = gtk_text_view_get_editable (GTK_TEXT_VIEW (view));
	}

	action_map = G_ACTION_MAP (tepl_window->priv->gtk_window);
	action = g_action_map_lookup_action (action_map, "tepl-paste");

	/* Since this is called async, the disposal of the actions may have
	 * already happened. Ensure that we have an action before setting the
	 * state.
	 */
	if (action != NULL)
	{
		g_simple_action_set_enabled (G_SIMPLE_ACTION (action),
					     view_is_editable &&
					     can_paste_according_to_clipboard);
	}
}

static void
clipboard_targets_received_cb (GtkClipboard *clipboard,
			       GdkAtom      *atoms,
			       gint          n_atoms,
			       gpointer      user_data)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (user_data);
	TeplBuffer *active_buffer;
	GtkTargetList *target_list;
	gboolean can_paste = FALSE;
	gint i;

	active_buffer = tepl_tab_group_get_active_buffer (TEPL_TAB_GROUP (tepl_window));
	if (active_buffer == NULL)
	{
		goto end;
	}

	target_list = gtk_text_buffer_get_paste_target_list (GTK_TEXT_BUFFER (active_buffer));

	for (i = 0; i < n_atoms; i++)
	{
		if (gtk_target_list_find (target_list, atoms[i], NULL))
		{
			can_paste = TRUE;
			break;
		}
	}

end:
	set_paste_action_sensitivity_according_to_clipboard (tepl_window, can_paste);

	/* Async operation finished. */
	g_object_unref (tepl_window->priv->gtk_window);
}

/* How to test this easily: with a clipboard manager like xsel:
 * $ xsel --clipboard --clear
 * $ echo -n "bloum!" | xsel --clipboard # -> GdkAtom "TEXT"
 * Copy text in a GtkTextBuffer -> GdkAtom "GTK_TEXT_BUFFER_CONTENTS"
 */
static void
update_paste_action_sensitivity (TeplApplicationWindow *tepl_window)
{
	GtkClipboard *clipboard;
	GdkDisplay *display;

	clipboard = gtk_widget_get_clipboard (GTK_WIDGET (tepl_window->priv->gtk_window),
					      GDK_SELECTION_CLIPBOARD);
	g_return_if_fail (clipboard != NULL);

	display = gtk_clipboard_get_display (clipboard);
	if (!gdk_display_supports_selection_notification (display))
	{
		/* Do as if it can always paste, because if we set the paste
		 * action as insensitive, we won't get the notification when the
		 * clipboard contains something that we can paste (i.e.
		 * clipboard_owner_change_cb() will not be called).
		 */
		set_paste_action_sensitivity_according_to_clipboard (tepl_window, TRUE);
		return;
	}

	g_object_ref (tepl_window->priv->gtk_window);
	gtk_clipboard_request_targets (clipboard,
				       clipboard_targets_received_cb,
				       tepl_window);
}

static void
update_basic_edit_actions_sensitivity (TeplApplicationWindow *tepl_window)
{
	TeplView *view;
	TeplBuffer *buffer;
	gboolean view_is_editable = FALSE;
	gboolean buffer_has_selection = FALSE;
	GActionMap *action_map;
	GAction *action;

	view = tepl_tab_group_get_active_view (TEPL_TAB_GROUP (tepl_window));
	buffer = tepl_tab_group_get_active_buffer (TEPL_TAB_GROUP (tepl_window));

	if (view != NULL)
	{
		view_is_editable = gtk_text_view_get_editable (GTK_TEXT_VIEW (view));
	}

	if (buffer != NULL)
	{
		buffer_has_selection = gtk_text_buffer_get_has_selection (GTK_TEXT_BUFFER (buffer));
	}

	action_map = G_ACTION_MAP (tepl_window->priv->gtk_window);

	action = g_action_map_lookup_action (action_map, "tepl-cut");
	g_simple_action_set_enabled (G_SIMPLE_ACTION (action),
				     view_is_editable && buffer_has_selection);

	action = g_action_map_lookup_action (action_map, "tepl-copy");
	g_simple_action_set_enabled (G_SIMPLE_ACTION (action),
				     buffer_has_selection);

	/* tepl-paste is treated separately with
	 * update_paste_action_sensitivity(), to request the clipboard only when
	 * necessary.
	 */

	action = g_action_map_lookup_action (action_map, "tepl-delete");
	g_simple_action_set_enabled (G_SIMPLE_ACTION (action),
				     view_is_editable && buffer_has_selection);

	action = g_action_map_lookup_action (action_map, "tepl-select-all");
	g_simple_action_set_enabled (G_SIMPLE_ACTION (action),
				     buffer != NULL);
}

static void
update_actions_sensitivity (TeplApplicationWindow *tepl_window)
{
	update_basic_edit_actions_sensitivity (tepl_window);
	update_paste_action_sensitivity (tepl_window);
}

static void
add_actions (TeplApplicationWindow *tepl_window)
{
	/* The actions need to be namespaced, to not conflict with the
	 * application or other libraries.
	 *
	 * Do not forget to document each action in the TeplApplicationWindow
	 * class description, and to add the corresponding AmtkActionInfoEntry
	 * in tepl-application.c.
	 */
	const GActionEntry entries[] = {
		/* File menu */
		{ "tepl-new-file", new_file_cb },

		/* Edit menu */
		{ "tepl-cut", cut_cb },
		{ "tepl-copy", copy_cb },
		{ "tepl-paste", paste_cb },
		{ "tepl-delete", delete_cb },
		{ "tepl-select-all", select_all_cb },
	};

	amtk_action_map_add_action_entries_check_dups (G_ACTION_MAP (tepl_window->priv->gtk_window),
						       entries,
						       G_N_ELEMENTS (entries),
						       tepl_window);

	update_actions_sensitivity (tepl_window);
}

static void
tepl_application_window_get_property (GObject    *object,
				      guint       prop_id,
				      GValue     *value,
				      GParamSpec *pspec)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (object);
	TeplTabGroup *tab_group = TEPL_TAB_GROUP (object);

	switch (prop_id)
	{
		case PROP_APPLICATION_WINDOW:
			g_value_set_object (value, tepl_application_window_get_application_window (tepl_window));
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
tepl_application_window_set_property (GObject      *object,
				      guint         prop_id,
				      const GValue *value,
				      GParamSpec   *pspec)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (object);
	TeplTabGroup *tab_group = TEPL_TAB_GROUP (object);

	switch (prop_id)
	{
		case PROP_APPLICATION_WINDOW:
			g_assert (tepl_window->priv->gtk_window == NULL);
			tepl_window->priv->gtk_window = g_value_get_object (value);
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
clipboard_owner_change_cb (GtkClipboard          *clipboard,
			   GdkEvent              *event,
			   TeplApplicationWindow *tepl_window)
{
	update_paste_action_sensitivity (tepl_window);
}

static void
tepl_application_window_constructed (GObject *object)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (object);
	GtkClipboard *clipboard;

	if (G_OBJECT_CLASS (tepl_application_window_parent_class)->constructed != NULL)
	{
		G_OBJECT_CLASS (tepl_application_window_parent_class)->constructed (object);
	}

	add_actions (tepl_window);

	clipboard = gtk_widget_get_clipboard (GTK_WIDGET (tepl_window->priv->gtk_window),
					      GDK_SELECTION_CLIPBOARD);

	g_signal_connect_object (clipboard,
				 "owner-change",
				 G_CALLBACK (clipboard_owner_change_cb),
				 tepl_window,
				 0);
}

static void
tepl_application_window_dispose (GObject *object)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (object);

	tepl_window->priv->gtk_window = NULL;
	g_clear_object (&tepl_window->priv->tab_group);
	_tepl_signal_group_clear (&tepl_window->priv->view_signal_group);
	_tepl_signal_group_clear (&tepl_window->priv->buffer_signal_group);

	G_OBJECT_CLASS (tepl_application_window_parent_class)->dispose (object);
}

static void
tepl_application_window_class_init (TeplApplicationWindowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = tepl_application_window_get_property;
	object_class->set_property = tepl_application_window_set_property;
	object_class->constructed = tepl_application_window_constructed;
	object_class->dispose = tepl_application_window_dispose;

	/**
	 * TeplApplicationWindow:application-window:
	 *
	 * The #GtkApplicationWindow.
	 *
	 * Since: 2.0
	 */
	g_object_class_install_property (object_class,
					 PROP_APPLICATION_WINDOW,
					 g_param_spec_object ("application-window",
							      "GtkApplicationWindow",
							      "",
							      GTK_TYPE_APPLICATION_WINDOW,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY |
							      G_PARAM_STATIC_STRINGS));

	g_object_class_override_property (object_class, PROP_ACTIVE_TAB, "active-tab");
	g_object_class_override_property (object_class, PROP_ACTIVE_VIEW, "active-view");
	g_object_class_override_property (object_class, PROP_ACTIVE_BUFFER, "active-buffer");
}

static GList *
tepl_application_window_get_tabs (TeplTabGroup *tab_group)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (tab_group);

	if (tepl_window->priv->tab_group == NULL)
	{
		return NULL;
	}

	return tepl_tab_group_get_tabs (tepl_window->priv->tab_group);
}

static TeplTab *
tepl_application_window_get_active_tab (TeplTabGroup *tab_group)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (tab_group);

	if (tepl_window->priv->tab_group == NULL)
	{
		return NULL;
	}

	return tepl_tab_group_get_active_tab (tepl_window->priv->tab_group);
}

static void
tepl_application_window_set_active_tab (TeplTabGroup *tab_group,
					TeplTab      *tab)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (tab_group);

	if (tepl_window->priv->tab_group != NULL)
	{
		tepl_tab_group_set_active_tab (tepl_window->priv->tab_group, tab);
	}
}

static void
tepl_application_window_append_tab_vfunc (TeplTabGroup *tab_group,
					  TeplTab      *tab)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (tab_group);

	if (tepl_window->priv->tab_group != NULL)
	{
		tepl_tab_group_append_tab (tepl_window->priv->tab_group, tab, FALSE);
	}
}

static void
tepl_tab_group_interface_init (gpointer g_iface,
			       gpointer iface_data)
{
	TeplTabGroupInterface *interface = g_iface;

	interface->get_tabs = tepl_application_window_get_tabs;
	interface->get_active_tab = tepl_application_window_get_active_tab;
	interface->set_active_tab = tepl_application_window_set_active_tab;
	interface->append_tab_vfunc = tepl_application_window_append_tab_vfunc;
}

static void
tepl_application_window_init (TeplApplicationWindow *tepl_window)
{
	tepl_window->priv = tepl_application_window_get_instance_private (tepl_window);
}

/**
 * tepl_application_window_get_from_gtk_application_window:
 * @gtk_window: a #GtkApplicationWindow.
 *
 * Returns the #TeplApplicationWindow of @gtk_window. The returned object is
 * guaranteed to be the same for the lifetime of @gtk_window.
 *
 * Returns: (transfer none): the #TeplApplicationWindow of @gtk_window.
 * Since: 2.0
 */
TeplApplicationWindow *
tepl_application_window_get_from_gtk_application_window (GtkApplicationWindow *gtk_window)
{
	TeplApplicationWindow *tepl_window;

	g_return_val_if_fail (GTK_IS_APPLICATION_WINDOW (gtk_window), NULL);

	tepl_window = g_object_get_data (G_OBJECT (gtk_window), TEPL_APPLICATION_WINDOW_KEY);

	if (tepl_window == NULL)
	{
		tepl_window = g_object_new (TEPL_TYPE_APPLICATION_WINDOW,
					    "application-window", gtk_window,
					    NULL);

		g_object_set_data_full (G_OBJECT (gtk_window),
					TEPL_APPLICATION_WINDOW_KEY,
					tepl_window,
					g_object_unref);
	}

	g_return_val_if_fail (TEPL_IS_APPLICATION_WINDOW (tepl_window), NULL);
	return tepl_window;
}

/**
 * tepl_application_window_get_application_window:
 * @tepl_window: a #TeplApplicationWindow.
 *
 * Returns: (transfer none): the #GtkApplicationWindow of @tepl_window.
 * Since: 2.0
 */
GtkApplicationWindow *
tepl_application_window_get_application_window (TeplApplicationWindow *tepl_window)
{
	g_return_val_if_fail (TEPL_IS_APPLICATION_WINDOW (tepl_window), NULL);

	return tepl_window->priv->gtk_window;
}

static void
active_tab_changed (TeplApplicationWindow *tepl_window)
{
	update_basic_edit_actions_sensitivity (tepl_window);
	update_paste_action_sensitivity (tepl_window);
}

static void
active_view_editable_notify_cb (GtkTextView           *active_view,
				GParamSpec            *pspec,
				TeplApplicationWindow *tepl_window)
{
	update_basic_edit_actions_sensitivity (tepl_window);
	update_paste_action_sensitivity (tepl_window);
}

static void
active_view_changed (TeplApplicationWindow *tepl_window)
{
	TeplView *active_view;

	_tepl_signal_group_clear (&tepl_window->priv->view_signal_group);

	active_view = tepl_tab_group_get_active_view (TEPL_TAB_GROUP (tepl_window));
	if (active_view == NULL)
	{
		return;
	}

	tepl_window->priv->view_signal_group = _tepl_signal_group_new (G_OBJECT (active_view));

	_tepl_signal_group_add (tepl_window->priv->view_signal_group,
				g_signal_connect (active_view,
						  "notify::editable",
						  G_CALLBACK (active_view_editable_notify_cb),
						  tepl_window));
}

static void
active_buffer_has_selection_notify_cb (GtkTextBuffer         *buffer,
				       GParamSpec            *pspec,
				       TeplApplicationWindow *tepl_window)
{
	update_basic_edit_actions_sensitivity (tepl_window);
}

static void
active_buffer_changed (TeplApplicationWindow *tepl_window)
{
	TeplBuffer *active_buffer;

	_tepl_signal_group_clear (&tepl_window->priv->buffer_signal_group);

	active_buffer = tepl_tab_group_get_active_buffer (TEPL_TAB_GROUP (tepl_window));
	if (active_buffer == NULL)
	{
		goto end;
	}

	tepl_window->priv->buffer_signal_group = _tepl_signal_group_new (G_OBJECT (active_buffer));

	_tepl_signal_group_add (tepl_window->priv->buffer_signal_group,
				g_signal_connect (active_buffer,
						  "notify::has-selection",
						  G_CALLBACK (active_buffer_has_selection_notify_cb),
						  tepl_window));

end:
	update_basic_edit_actions_sensitivity (tepl_window);
}

static void
active_tab_notify_cb (TeplTabGroup          *tab_group,
		      GParamSpec            *pspec,
		      TeplApplicationWindow *tepl_window)
{
	active_tab_changed (tepl_window);
	g_object_notify (G_OBJECT (tepl_window), "active-tab");
}

static void
active_view_notify_cb (TeplTabGroup          *tab_group,
		       GParamSpec            *pspec,
		       TeplApplicationWindow *tepl_window)
{
	active_view_changed (tepl_window);
	g_object_notify (G_OBJECT (tepl_window), "active-view");
}

static void
active_buffer_notify_cb (TeplTabGroup          *tab_group,
			 GParamSpec            *pspec,
			 TeplApplicationWindow *tepl_window)
{
	active_buffer_changed (tepl_window);
	g_object_notify (G_OBJECT (tepl_window), "active-buffer");
}

/**
 * tepl_application_window_set_tab_group:
 * @tepl_window: a #TeplApplicationWindow.
 * @tab_group: a #TeplTabGroup.
 *
 * Sets the #TeplTabGroup of @tepl_window. This function can be called only
 * once, it is not possible to change the #TeplTabGroup afterwards (this
 * restriction may be lifted in the future if there is a compelling use-case).
 *
 * #TeplApplicationWindow implements the #TeplTabGroup interface by delegating
 * the requests to @tab_group.
 *
 * Since: 3.0
 */
void
tepl_application_window_set_tab_group (TeplApplicationWindow *tepl_window,
				       TeplTabGroup          *tab_group)
{
	TeplTab *active_tab;

	g_return_if_fail (TEPL_IS_APPLICATION_WINDOW (tepl_window));
	g_return_if_fail (TEPL_IS_TAB_GROUP (tab_group));

	if (tepl_window->priv->tab_group != NULL)
	{
		g_warning ("%s(): the TeplTabGroup has already been set, it can be set only once.",
			   G_STRFUNC);
		return;
	}

	tepl_window->priv->tab_group = g_object_ref_sink (tab_group);

	g_signal_connect_object (tab_group,
				 "notify::active-tab",
				 G_CALLBACK (active_tab_notify_cb),
				 tepl_window,
				 0);

	g_signal_connect_object (tab_group,
				 "notify::active-view",
				 G_CALLBACK (active_view_notify_cb),
				 tepl_window,
				 0);

	g_signal_connect_object (tab_group,
				 "notify::active-buffer",
				 G_CALLBACK (active_buffer_notify_cb),
				 tepl_window,
				 0);

	active_tab = tepl_tab_group_get_active_tab (tab_group);
	if (active_tab != NULL)
	{
		active_tab_changed (tepl_window);
		g_object_notify (G_OBJECT (tepl_window), "active-tab");

		active_view_changed (tepl_window);
		g_object_notify (G_OBJECT (tepl_window), "active-view");

		active_buffer_changed (tepl_window);
		g_object_notify (G_OBJECT (tepl_window), "active-buffer");
	}
}

/* ex:set ts=8 noet: */
