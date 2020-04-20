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

#include "tepl-stack.h"

/**
 * SECTION:stack
 * @Title: TeplStack
 * @Short_description: #GtkStack functions
 *
 * #GtkStack functions.
 */

/**
 * tepl_stack_add_component:
 * @stack: a #GtkStack.
 * @child: the #GtkWidget to add to the @stack.
 * @name: the name for @child.
 * @title: a human-readable title for @child.
 * @icon_name: the icon name for @child.
 *
 * The same as gtk_stack_add_titled(), but sets in addition the “icon-name”
 * child property.
 *
 * Since: 5.0
 */
void
tepl_stack_add_component (GtkStack    *stack,
			  GtkWidget   *child,
			  const gchar *name,
			  const gchar *title,
			  const gchar *icon_name)
{
	g_return_if_fail (icon_name != NULL);

	gtk_stack_add_titled (stack, child, name, title);

	gtk_container_child_set (GTK_CONTAINER (stack),
				 child,
				 "icon-name", icon_name,
				 NULL);
}

static gboolean
bind_get_mapping_cb (GValue   *to_property_value,
		     GVariant *from_setting_variant,
		     gpointer  user_data)
{
	GtkStack *stack = GTK_STACK (user_data);
	const gchar *child_name;
	GtkWidget *child_widget;

	child_name = g_variant_get_string (from_setting_variant, NULL);
	child_widget = gtk_stack_get_child_by_name (stack, child_name);

	/* If child_widget is NULL, do nothing, and do not print an error. It
	 * can happen if it's an old GSettings value and the child in the
	 * GtkStack no longer exists after an upgrade of the application.
	 */

	if (child_widget != NULL)
	{
		g_value_set_string (to_property_value, child_name);
	}

	return TRUE;
}

/**
 * tepl_stack_bind_setting:
 * @stack: a #GtkStack.
 * @settings: a #GSettings object.
 * @setting_key: the #GSettings key of type string.
 *
 * Binds the provided #GSettings key with the #GtkStack:visible-child-name
 * property of @stack.
 *
 * This function must be called when all #GtkWidget children have been added to
 * @stack, to initially restore the state from #GSettings and then to update the
 * #GSettings key when the visible child changes.
 *
 * Since: 5.0
 */
void
tepl_stack_bind_setting (GtkStack    *stack,
			 GSettings   *settings,
			 const gchar *setting_key)
{
	g_return_if_fail (GTK_IS_STACK (stack));
	g_return_if_fail (G_IS_SETTINGS (settings));
	g_return_if_fail (setting_key != NULL);

	/* G_SETTINGS_BIND_GET_NO_CHANGES is used because an application can
	 * have several windows with the same GtkStack/panel class, bound to the
	 * same GSettings key. But the visible child widget can be different on
	 * each window. On application exit, the GSettings key is set for the
	 * last closed window.
	 */
	g_settings_bind_with_mapping (settings, setting_key,
				      stack, "visible-child-name",
				      G_SETTINGS_BIND_SET |
				      G_SETTINGS_BIND_GET |
				      G_SETTINGS_BIND_GET_NO_CHANGES |
				      G_SETTINGS_BIND_NO_SENSITIVITY,
				      bind_get_mapping_cb,
				      NULL,
				      stack, NULL);
}
