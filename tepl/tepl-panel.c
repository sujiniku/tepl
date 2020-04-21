/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2020 - Sébastien Wilmet <swilmet@gnome.org>
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
#include "tepl-panel.h"
#include <glib/gi18n-lib.h>
#include "tepl-utils.h"

/**
 * SECTION:panel
 * @Title: TeplPanel
 * @Short_description: Side or bottom panel container
 *
 * #TeplPanel permits to create a side or bottom panel that contains several
 * components.
 *
 * #TeplPanel is a #GtkGrid subclass containing only one child #GtkWidget by
 * default: the #GtkStack that can be retrieved with tepl_panel_get_stack().
 */

struct _TeplPanelPrivate
{
	GtkStack *stack;

	GSettings *settings;
	gchar *active_component_setting_key;
};

G_DEFINE_TYPE_WITH_PRIVATE (TeplPanel, tepl_panel, GTK_TYPE_GRID)

static void
tepl_panel_dispose (GObject *object)
{
	TeplPanel *panel = TEPL_PANEL (object);

	panel->priv->stack = NULL;

	g_clear_object (&panel->priv->settings);
	g_clear_pointer (&panel->priv->active_component_setting_key, g_free);

	G_OBJECT_CLASS (tepl_panel_parent_class)->dispose (object);
}

static void
tepl_panel_class_init (TeplPanelClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = tepl_panel_dispose;
}

static void
tepl_panel_init (TeplPanel *panel)
{
	panel->priv = tepl_panel_get_instance_private (panel);

	panel->priv->stack = GTK_STACK (gtk_stack_new ());
	gtk_widget_show (GTK_WIDGET (panel->priv->stack));
	gtk_container_add (GTK_CONTAINER (panel),
			   GTK_WIDGET (panel->priv->stack));
}

/**
 * tepl_panel_new:
 *
 * Creates a new #TeplPanel containing only an empty #GtkStack that can be
 * retrieved with tepl_panel_get_stack().
 *
 * Returns: (transfer floating): a new #TeplPanel.
 * Since: 5.0
 */
TeplPanel *
tepl_panel_new (void)
{
	return g_object_new (TEPL_TYPE_PANEL, NULL);
}

static void
close_button_clicked_cb (GtkButton *close_button,
			 TeplPanel *panel)
{
	gtk_widget_hide (GTK_WIDGET (panel));
}

static GtkWidget *
create_close_button (TeplPanel *panel)
{
	GtkWidget *close_button;

	close_button = tepl_utils_create_close_button ();
	gtk_widget_set_tooltip_text (close_button, _("Hide panel"));

	g_signal_connect_object (close_button,
				 "clicked",
				 G_CALLBACK (close_button_clicked_cb),
				 panel,
				 0);

	return close_button;
}

static TeplPanel *
new_for_side_panel (void)
{
	TeplPanel *panel;
	GtkStackSwitcher *stack_switcher;
	GtkActionBar *action_bar;

	panel = tepl_panel_new ();
	gtk_orientable_set_orientation (GTK_ORIENTABLE (panel), GTK_ORIENTATION_VERTICAL);

	stack_switcher = GTK_STACK_SWITCHER (gtk_stack_switcher_new ());
	gtk_stack_switcher_set_stack (stack_switcher, panel->priv->stack);

	action_bar = GTK_ACTION_BAR (gtk_action_bar_new ());
	gtk_action_bar_set_center_widget (action_bar, GTK_WIDGET (stack_switcher));
	gtk_action_bar_pack_end (action_bar, create_close_button (panel));

	gtk_grid_attach_next_to (GTK_GRID (panel),
				 GTK_WIDGET (action_bar),
				 GTK_WIDGET (panel->priv->stack),
				 GTK_POS_TOP, 1, 1);

	gtk_widget_show_all (GTK_WIDGET (panel));

	return panel;
}

/**
 * tepl_panel_new_for_left_side_panel:
 *
 * Creates a new #TeplPanel intended to be used as a side panel added on the
 * left side inside a #GtkWindow.
 *
 * It contains:
 * - A #GtkStackSwitcher.
 * - A close button that hides the #TeplPanel when clicked.
 * - A #GtkStack that can be retrieved with tepl_panel_get_stack().
 *
 * Returns: (transfer floating): a new left side #TeplPanel.
 * Since: 5.0
 */
TeplPanel *
tepl_panel_new_for_left_side_panel (void)
{
	TeplPanel *panel;

	panel = new_for_side_panel ();
	gtk_widget_set_margin_start (GTK_WIDGET (panel), 6);

	return panel;
}

/**
 * tepl_panel_get_stack:
 * @panel: a #TeplPanel.
 *
 * Returns: (transfer none): the #GtkStack widget of @panel (a direct child
 * #GtkWidget of @panel).
 * Since: 5.0
 */
GtkStack *
tepl_panel_get_stack (TeplPanel *panel)
{
	g_return_val_if_fail (TEPL_IS_PANEL (panel), NULL);

	return panel->priv->stack;
}

/**
 * tepl_panel_add_component:
 * @panel: a #TeplPanel.
 * @component: the child #GtkWidget to add to the #GtkStack of @panel.
 * @name: the name for @component.
 * @title: a human-readable title for @component.
 * @icon_name: (nullable): the icon name for @component, or %NULL.
 *
 * The equivalent of gtk_stack_add_titled(), with an optional @icon_name to set
 * the “icon-name” #GtkStack child property.
 *
 * Since: 5.0
 */
void
tepl_panel_add_component (TeplPanel   *panel,
			  GtkWidget   *component,
			  const gchar *name,
			  const gchar *title,
			  const gchar *icon_name)
{
	g_return_if_fail (TEPL_IS_PANEL (panel));
	g_return_if_fail (GTK_IS_WIDGET (component));
	g_return_if_fail (name != NULL);
	g_return_if_fail (title != NULL);

	gtk_stack_add_titled (panel->priv->stack, component, name, title);

	if (icon_name != NULL)
	{
		gtk_container_child_set (GTK_CONTAINER (panel->priv->stack),
					 component,
					 "icon-name", icon_name,
					 NULL);
	}
}

/**
 * tepl_panel_provide_active_component_gsetting:
 * @panel: a #TeplPanel.
 * @settings: a #GSettings object.
 * @setting_key: a #GSettings key of type string.
 *
 * Provides a #GSettings key for saving and restoring the
 * #GtkStack:visible-child-name property of the #GtkStack belonging to @panel.
 *
 * This function just stores @settings and @setting_key for further use by
 * tepl_panel_restore_state_from_gsettings() and
 * tepl_panel_save_state_to_gsettings().
 *
 * Note that only one @settings/@setting_key pair is stored by @panel for
 * further use, if you call this function twice on the same @panel, the second
 * call overrides the first one.
 *
 * Since: 5.0
 */
void
tepl_panel_provide_active_component_gsetting (TeplPanel   *panel,
					      GSettings   *settings,
					      const gchar *setting_key)
{
	g_return_if_fail (TEPL_IS_PANEL (panel));
	g_return_if_fail (G_IS_SETTINGS (settings));
	g_return_if_fail (setting_key != NULL);

	g_set_object (&panel->priv->settings, settings);

	g_free (panel->priv->active_component_setting_key);
	panel->priv->active_component_setting_key = g_strdup (setting_key);
}

/**
 * tepl_panel_restore_state_from_gsettings:
 * @panel: a #TeplPanel.
 *
 * Restores the state of @panel according to the provided #GSettings.
 *
 * This function must be called when all components have been added to the
 * #GtkStack of @panel.
 *
 * Since: 5.0
 */
void
tepl_panel_restore_state_from_gsettings (TeplPanel *panel)
{
	gchar *active_component_name;
	GtkWidget *child_widget;

	g_return_if_fail (TEPL_IS_PANEL (panel));

	if (panel->priv->settings == NULL)
	{
		return;
	}

	active_component_name = g_settings_get_string (panel->priv->settings,
						       panel->priv->active_component_setting_key);

	child_widget = gtk_stack_get_child_by_name (panel->priv->stack, active_component_name);

	/* If child_widget is NULL, do nothing, and do not print an error. It
	 * can happen if it's an old GSettings value and the child in the
	 * GtkStack no longer exists after an upgrade of the application.
	 */

	if (child_widget != NULL)
	{
		gtk_stack_set_visible_child (panel->priv->stack, child_widget);
	}

	g_free (active_component_name);
}

/**
 * tepl_panel_save_state_to_gsettings:
 * @panel: a #TeplPanel.
 *
 * Saves the current state of @panel to the provided #GSettings.
 *
 * Since: 5.0
 */
void
tepl_panel_save_state_to_gsettings (TeplPanel *panel)
{
	const gchar *visible_child_name;

	g_return_if_fail (TEPL_IS_PANEL (panel));

	if (panel->priv->settings == NULL)
	{
		return;
	}

	visible_child_name = gtk_stack_get_visible_child_name (panel->priv->stack);
	if (visible_child_name != NULL)
	{
		g_settings_set_string (panel->priv->settings,
				       panel->priv->active_component_setting_key,
				       visible_child_name);
	}
}
