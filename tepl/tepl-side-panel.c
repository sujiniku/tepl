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
#include "tepl-side-panel.h"
#include <glib/gi18n-lib.h>
#include "tepl-utils.h"

/**
 * SECTION:side-panel
 * @Title: TeplSidePanel
 * @Short_description: Functions to create a side panel
 *
 * Functions to create a side panel.
 *
 * The workflow to create a side panel is as follows:
 * 1. gtk_stack_new()
 * 2. tepl_stack_add_component() multiple times.
 * 3. tepl_side_panel_new()
 * 4. tepl_stack_bind_setting()
 */

static void
close_button_clicked_cb (GtkButton *close_button,
			 GtkWidget *side_panel)
{
	gtk_widget_hide (side_panel);
}

static GtkWidget *
create_close_button (GtkWidget *side_panel)
{
	GtkWidget *close_button;

	close_button = tepl_utils_create_close_button ();
	gtk_widget_set_tooltip_text (close_button, _("Hide panel"));

	g_signal_connect (close_button,
			  "clicked",
			  G_CALLBACK (close_button_clicked_cb),
			  side_panel);

	return close_button;
}

/**
 * tepl_side_panel_new:
 * @stack: a #GtkStack.
 *
 * Creates a new container intended to be used as a side panel. It contains:
 * - A #GtkStackSwitcher.
 * - A close button that hides the side panel when clicked.
 * - The provided @stack.
 *
 * Returns: (transfer floating): a new side panel container.
 * Since: 5.0
 */
GtkWidget *
tepl_side_panel_new (GtkStack *stack)
{
	GtkWidget *vgrid;
	GtkStackSwitcher *stack_switcher;
	GtkActionBar *action_bar;

	g_return_val_if_fail (GTK_IS_STACK (stack), NULL);

	vgrid = gtk_grid_new ();
	gtk_orientable_set_orientation (GTK_ORIENTABLE (vgrid), GTK_ORIENTATION_VERTICAL);
	/* We assume here that it's a *left* side panel. */
	gtk_widget_set_margin_start (vgrid, 6);

	stack_switcher = GTK_STACK_SWITCHER (gtk_stack_switcher_new ());
	gtk_stack_switcher_set_stack (stack_switcher, stack);

	action_bar = GTK_ACTION_BAR (gtk_action_bar_new ());
	gtk_action_bar_set_center_widget (action_bar, GTK_WIDGET (stack_switcher));
	gtk_action_bar_pack_end (action_bar, create_close_button (vgrid));

	gtk_container_add (GTK_CONTAINER (vgrid), GTK_WIDGET (action_bar));
	gtk_widget_show_all (vgrid);

	gtk_container_add (GTK_CONTAINER (vgrid), GTK_WIDGET (stack));

	/* Do not call gtk_widget_show_all() on stack, it's externally-provided. */
	gtk_widget_show (GTK_WIDGET (stack));

	return vgrid;
}
