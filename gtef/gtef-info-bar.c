/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "gtef-info-bar.h"

/**
 * SECTION:info-bar
 * @Short_description: Subclass of GtkInfoBar
 * @Title: GtefInfoBar
 *
 * #GtefInfoBar is a subclass of #GtkInfoBar with a vertical action area.
 */

G_DEFINE_TYPE (GtefInfoBar, gtef_info_bar, GTK_TYPE_INFO_BAR)

static void
gtef_info_bar_class_init (GtefInfoBarClass *klass)
{
}

static void
gtef_info_bar_init (GtefInfoBar *info_bar)
{
	GtkWidget *action_area;

	/* Change the buttons orientation to be vertical.
	 * With a small window, if 3 or more buttons are shown horizontally,
	 * there is a ridiculous amount of space for the text. And it can get
	 * worse since the button labels are translatable, in other languages it
	 * can take even more place. If the buttons are packed vertically, there
	 * is no problem.
	 */
	action_area = gtk_info_bar_get_action_area (GTK_INFO_BAR (info_bar));
	if (GTK_IS_ORIENTABLE (action_area))
	{
		gtk_orientable_set_orientation (GTK_ORIENTABLE (action_area),
						GTK_ORIENTATION_VERTICAL);
	}
	else
	{
		g_warning ("Failed to set vertical orientation to the GtkInfoBar action area.");
	}
}

/**
 * gtef_info_bar_new:
 *
 * Returns: a new #GtefInfoBar.
 * Since: 1.0
 */
GtefInfoBar *
gtef_info_bar_new (void)
{
	return g_object_new (GTEF_TYPE_INFO_BAR, NULL);
}

/**
 * gtef_info_bar_create_label:
 *
 * Utility function to create a #GtkLabel suitable for a #GtkInfoBar. The
 * wrapping and alignment is configured. The label is also set as selectable,
 * for example to copy an error message and search an explanation on the web.
 *
 * Returns: (transfer floating): a new #GtkLabel suitable for a #GtkInfoBar.
 * Since: 1.0
 */
GtkLabel *
gtef_info_bar_create_label (void)
{
	GtkLabel *label;

	label = GTK_LABEL (gtk_label_new (NULL));
	gtk_widget_set_halign (GTK_WIDGET (label), GTK_ALIGN_START);
	gtk_label_set_line_wrap (label, TRUE);
	gtk_label_set_line_wrap_mode (label, PANGO_WRAP_WORD_CHAR);

	gtk_label_set_selectable (label, TRUE);

	/* FIXME really needed? We already set the label as selectable. */
	gtk_widget_set_can_focus (GTK_WIDGET (label), TRUE);

	return label;
}
