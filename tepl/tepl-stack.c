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
