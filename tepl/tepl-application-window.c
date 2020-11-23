/* SPDX-FileCopyrightText: 2017-2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "config.h"
#include "tepl-application-window.h"
#include <glib/gi18n-lib.h>
#include "tepl-abstract-factory.h"
#include "tepl-buffer.h"
#include "tepl-signal-group.h"
#include "tepl-tab-loading.h"
#include "tepl-window-actions-file.h"
#include "tepl-window-actions-edit.h"
#include "tepl-window-actions-search.h"

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
 * - `"win.tepl-open"`: shows a #GtkFileChooser to open a new file.
 * - `"win.tepl-save"`: saves the current file.
 * - `"win.tepl-save-as"`: shows a #GtkFileChooser to save the current file to a
 *   different location.
 *
 * ## For the Edit menu
 *
 * - `"win.tepl-undo"`: calls gtk_source_buffer_undo() on the active buffer.
 * - `"win.tepl-redo"`: calls gtk_source_buffer_redo() on the active buffer.
 *
 * The following actions require the %AMTK_FACTORY_IGNORE_ACCELS_FOR_APP flag,
 * because otherwise accelerators don't work in other text widgets than the
 * active view (e.g. in a #GtkEntry):
 * - `"win.tepl-cut"`: calls tepl_view_cut_clipboard() on the active view.
 * - `"win.tepl-copy"`: calls tepl_view_copy_clipboard() on the active view.
 * - `"win.tepl-paste"`: calls tepl_view_paste_clipboard() on the active view.
 * - `"win.tepl-delete"`: calls tepl_view_delete_selection() on the active view.
 * - `"win.tepl-select-all"`: calls tepl_view_select_all() on the active view.
 * - `"win.tepl-indent"`: calls gtk_source_view_indent_lines() on the selected
 *   text of the active view.
 * - `"win.tepl-unindent"`: calls gtk_source_view_unindent_lines() on the
 *   selected text of the active view.
 *
 * See the tepl_menu_shell_append_edit_actions() convenience function.
 *
 * ## For the Search menu
 *
 * - `"win.tepl-goto-line"`: shows the #TeplGotoLineBar of all #TeplTab's
 *   belonging to #TeplApplicationWindow. Even though each #TeplTab has a
 *   different #TeplGotoLineBar, all #TeplGotoLineBar's of the #TeplTabGroup
 *   have their #GtkWidget:visible state synchronized, so when one
 *   #TeplGotoLineBar is hidden, all the other #TeplGotoLineBar's are hidden as
 *   well. The user may think that there is only one #TeplGotoLineBar per
 *   window, with the #TeplGotoLineBar remembering a different state (mainly the
 *   content of the #GtkSearchEntry) for each #TeplTab. To remember the state
 *   for each #TeplTab, the easiest is to have a different widget for each
 *   #TeplTab, hence the current implementation.
 */

struct _TeplApplicationWindowPrivate
{
	GtkApplicationWindow *gtk_window;

	TeplWindowActionsEdit *window_actions_edit;

	GtkWindowGroup *window_group;

	TeplTabGroup *tab_group;
	TeplSignalGroup *view_signal_group;
	TeplSignalGroup *buffer_signal_group;

	guint handle_title : 1;
};

enum
{
	PROP_0,
	PROP_APPLICATION_WINDOW,
	PROP_ACTIVE_TAB,
	PROP_ACTIVE_VIEW,
	PROP_ACTIVE_BUFFER,
	PROP_HANDLE_TITLE,
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
update_title (TeplApplicationWindow *tepl_window)
{
	TeplView *active_view;

	if (!tepl_window->priv->handle_title)
	{
		return;
	}

	active_view = tepl_tab_group_get_active_view (TEPL_TAB_GROUP (tepl_window));

	if (active_view == NULL)
	{
		gtk_window_set_title (GTK_WINDOW (tepl_window->priv->gtk_window),
				      g_get_application_name ());
	}
	else
	{
		TeplBuffer *active_buffer;
		gchar *buffer_title;
		gboolean read_only;
		gchar *read_only_str = NULL;
		gchar *window_title;

		active_buffer = tepl_tab_group_get_active_buffer (TEPL_TAB_GROUP (tepl_window));

		/* It is fine to call gtk_window_set_title() with a too long
		 * string, but in that case the application name is not visible.
		 *
		 * Possible improvement: pass an additional int parameter to
		 * tepl_buffer_get_full_title() to middle-truncate its longest
		 * component (either the filename or the directory).
		 */
		buffer_title = tepl_buffer_get_full_title (active_buffer);

		read_only = !gtk_text_view_get_editable (GTK_TEXT_VIEW (active_view));
		if (read_only)
		{
			read_only_str = g_strdup_printf (" [%s]", _("Read-Only"));
		}

		window_title = g_strdup_printf ("%s%s - %s",
						buffer_title,
						read_only ? read_only_str : "",
						g_get_application_name ());

		gtk_window_set_title (GTK_WINDOW (tepl_window->priv->gtk_window),
				      window_title);

		g_free (buffer_title);
		g_free (read_only_str);
		g_free (window_title);
	}
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

		case PROP_HANDLE_TITLE:
			g_value_set_boolean (value, tepl_application_window_get_handle_title (tepl_window));
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

		case PROP_HANDLE_TITLE:
			tepl_application_window_set_handle_title (tepl_window,
								  g_value_get_boolean (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_application_window_constructed (GObject *object)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (object);

	if (G_OBJECT_CLASS (tepl_application_window_parent_class)->constructed != NULL)
	{
		G_OBJECT_CLASS (tepl_application_window_parent_class)->constructed (object);
	}

	_tepl_window_actions_file_add_actions (tepl_window);
	_tepl_window_actions_search_add_actions (tepl_window);

	g_assert (tepl_window->priv->window_actions_edit == NULL);
	tepl_window->priv->window_actions_edit = _tepl_window_actions_edit_new (tepl_window);
}

static void
tepl_application_window_dispose (GObject *object)
{
	TeplApplicationWindow *tepl_window = TEPL_APPLICATION_WINDOW (object);

	tepl_window->priv->gtk_window = NULL;

	_tepl_window_actions_edit_clear (&tepl_window->priv->window_actions_edit);

	g_clear_object (&tepl_window->priv->window_group);

	g_clear_object (&tepl_window->priv->tab_group);
	tepl_signal_group_clear (&tepl_window->priv->view_signal_group);
	tepl_signal_group_clear (&tepl_window->priv->buffer_signal_group);

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

	/**
	 * TeplApplicationWindow:handle-title:
	 *
	 * Whether to handle the #GtkWindow:title. The title is probably not
	 * appropriate if a #GtkHeaderBar is used, the title is meant to be used
	 * only for applications with a traditional UI.
	 *
	 * If %TRUE, the title will contain:
	 * - the #TeplBuffer:tepl-full-title of the active buffer.
	 * - if the active view is not #GtkTextView:editable, the
	 *   `"[Read-Only]"` string.
	 * - the application name as returned by g_get_application_name().
	 *
	 * If the active view is %NULL, the title contains only the application
	 * name.
	 *
	 * Since: 4.0
	 */
	g_object_class_install_property (object_class,
					 PROP_HANDLE_TITLE,
					 g_param_spec_boolean ("handle-title",
							       "handle-title",
							       "",
							       FALSE,
							       G_PARAM_READWRITE |
							       G_PARAM_CONSTRUCT |
							       G_PARAM_STATIC_STRINGS));
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
	update_title (tepl_window);
}

static void
active_view_editable_notify_cb (GtkTextView           *active_view,
				GParamSpec            *pspec,
				TeplApplicationWindow *tepl_window)
{
	update_title (tepl_window);
}

static void
active_view_changed (TeplApplicationWindow *tepl_window)
{
	TeplView *active_view;

	tepl_signal_group_clear (&tepl_window->priv->view_signal_group);

	active_view = tepl_tab_group_get_active_view (TEPL_TAB_GROUP (tepl_window));
	if (active_view == NULL)
	{
		return;
	}

	tepl_window->priv->view_signal_group = tepl_signal_group_new (G_OBJECT (active_view));

	tepl_signal_group_add (tepl_window->priv->view_signal_group,
			       g_signal_connect (active_view,
						 "notify::editable",
						 G_CALLBACK (active_view_editable_notify_cb),
						 tepl_window));
}

static void
active_buffer_full_title_notify_cb (TeplBuffer            *buffer,
				    GParamSpec            *pspec,
				    TeplApplicationWindow *tepl_window)
{
	update_title (tepl_window);
}

static void
active_buffer_changed (TeplApplicationWindow *tepl_window)
{
	TeplBuffer *active_buffer;

	tepl_signal_group_clear (&tepl_window->priv->buffer_signal_group);

	active_buffer = tepl_tab_group_get_active_buffer (TEPL_TAB_GROUP (tepl_window));
	if (active_buffer == NULL)
	{
		goto end;
	}

	tepl_window->priv->buffer_signal_group = tepl_signal_group_new (G_OBJECT (active_buffer));

	tepl_signal_group_add (tepl_window->priv->buffer_signal_group,
			       g_signal_connect (active_buffer,
						 "notify::tepl-full-title",
						 G_CALLBACK (active_buffer_full_title_notify_cb),
						 tepl_window));

end:
	update_title (tepl_window);
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

/**
 * tepl_application_window_is_main_window:
 * @gtk_window: a #GtkApplicationWindow.
 *
 * Returns %TRUE iff @gtk_window has an associated #TeplTabGroup (i.e. if
 * tepl_application_window_set_tab_group() has been called).
 *
 * This function takes a #GtkApplicationWindow parameter to avoid creating the
 * #TeplApplicationWindow object if it hasn't been created.
 *
 * Returns: whether @gtk_window is considered a main application window.
 * Since: 4.0
 */
gboolean
tepl_application_window_is_main_window (GtkApplicationWindow *gtk_window)
{
	TeplApplicationWindow *tepl_window;

	g_return_val_if_fail (GTK_IS_APPLICATION_WINDOW (gtk_window), FALSE);

	tepl_window = g_object_get_data (G_OBJECT (gtk_window), TEPL_APPLICATION_WINDOW_KEY);
	if (tepl_window == NULL)
	{
		return FALSE;
	}

	return tepl_window->priv->tab_group != NULL;
}

/**
 * tepl_application_window_get_window_group:
 * @tepl_window: a #TeplApplicationWindow.
 *
 * Gets the #GtkWindowGroup in which @tepl_window resides.
 *
 * You should call this function only on main windows, to add secondary windows
 * to the #GtkWindowGroup.
 *
 * Returns: (transfer none): the #GtkWindowGroup.
 * Since: 4.0
 */
GtkWindowGroup *
tepl_application_window_get_window_group (TeplApplicationWindow *tepl_window)
{
	g_return_val_if_fail (TEPL_IS_APPLICATION_WINDOW (tepl_window), NULL);

	/* Lazy init.
	 *
	 * If the GtkWindowGroup was created in constructed() instead, this can
	 * be dangerous because the mere fact of calling
	 * tepl_application_window_get_from_gtk_application_window() would add
	 * the window to a different GtkWindowGroup. If for one reason or
	 * another the TeplApplicationWindow object is created for a secondary
	 * window, it should not cause problems.
	 *
	 * It is not a problem if a main window is still part of the default
	 * window group (i.e. if this function has never been called on that
	 * main window). For example when creating a modal dialog, this function
	 * will be called on the corresponding main window, and it'll still be
	 * possible to interact with the other main windows that are part of the
	 * default window group.
	 */
	if (tepl_window->priv->window_group == NULL)
	{
		tepl_window->priv->window_group = gtk_window_group_new ();
		gtk_window_group_add_window (tepl_window->priv->window_group,
					     GTK_WINDOW (tepl_window->priv->gtk_window));
	}

	return tepl_window->priv->window_group;
}

/**
 * tepl_application_window_get_handle_title:
 * @tepl_window: a #TeplApplicationWindow.
 *
 * Returns: the value of the #TeplApplicationWindow:handle-title property.
 * Since: 4.0
 */
gboolean
tepl_application_window_get_handle_title (TeplApplicationWindow *tepl_window)
{
	g_return_val_if_fail (TEPL_IS_APPLICATION_WINDOW (tepl_window), FALSE);

	return tepl_window->priv->handle_title;
}

/**
 * tepl_application_window_set_handle_title:
 * @tepl_window: a #TeplApplicationWindow.
 * @handle_title: the new value.
 *
 * Sets the #TeplApplicationWindow:handle-title property.
 *
 * Since: 4.0
 */
void
tepl_application_window_set_handle_title (TeplApplicationWindow *tepl_window,
					  gboolean               handle_title)
{
	g_return_if_fail (TEPL_IS_APPLICATION_WINDOW (tepl_window));

	handle_title = handle_title != FALSE;

	if (tepl_window->priv->handle_title != handle_title)
	{
		tepl_window->priv->handle_title = handle_title;
		update_title (tepl_window);
		g_object_notify (G_OBJECT (tepl_window), "handle-title");
	}
}

/**
 * tepl_application_window_open_file:
 * @tepl_window: a #TeplApplicationWindow.
 * @location: a #GFile.
 * @jump_to: whether to set the tab where the file is loaded as the active tab.
 *
 * Opens a file in @tepl_window. If the active tab is untouched (see
 * tepl_buffer_is_untouched()), then the file is loaded in that tab. Otherwise a
 * new tab is created.
 *
 * This function is asynchronous, the file loading is done with the
 * tepl_tab_load_file() function. There is no way to know when the file loading
 * is finished.
 *
 * Since: 4.0
 */
void
tepl_application_window_open_file (TeplApplicationWindow *tepl_window,
				   GFile                 *location,
				   gboolean               jump_to)
{
	TeplTab *tab;
	TeplBuffer *buffer;

	g_return_if_fail (TEPL_IS_APPLICATION_WINDOW (tepl_window));
	g_return_if_fail (G_IS_FILE (location));

	tab = tepl_tab_group_get_active_tab (TEPL_TAB_GROUP (tepl_window));
	buffer = tepl_tab_group_get_active_buffer (TEPL_TAB_GROUP (tepl_window));

	if (buffer == NULL ||
	    !tepl_buffer_is_untouched (buffer))
	{
		TeplAbstractFactory *factory;

		factory = tepl_abstract_factory_get_singleton ();
		tab = tepl_abstract_factory_create_tab (factory);
		gtk_widget_show (GTK_WIDGET (tab));

		tepl_tab_group_append_tab (TEPL_TAB_GROUP (tepl_window), tab, jump_to);
	}

	tepl_tab_load_file (tab, location);
}
