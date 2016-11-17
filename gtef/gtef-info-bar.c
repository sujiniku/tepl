/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * Gtef is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * Gtef is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "gtef-info-bar.h"

/**
 * SECTION:info-bar
 * @Short_description: Subclass of GtkInfoBar
 * @Title: GtefInfoBar
 *
 * #GtefInfoBar is a subclass of #GtkInfoBar with a vertical action area and
 * functions to ease the creation of info bars.
 */

typedef struct _GtefInfoBarPrivate GtefInfoBarPrivate;

struct _GtefInfoBarPrivate
{
	GtkGrid *content_vgrid;
};

G_DEFINE_TYPE_WITH_PRIVATE (GtefInfoBar, gtef_info_bar, GTK_TYPE_INFO_BAR)

static void
gtef_info_bar_class_init (GtefInfoBarClass *klass)
{
}

static void
gtef_info_bar_init (GtefInfoBar *info_bar)
{
	GtefInfoBarPrivate *priv;
	GtkWidget *action_area;
	GtkWidget *content_area;

	priv = gtef_info_bar_get_instance_private (info_bar);

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

	priv->content_vgrid = GTK_GRID (gtk_grid_new ());
	gtk_orientable_set_orientation (GTK_ORIENTABLE (priv->content_vgrid),
					GTK_ORIENTATION_VERTICAL);
	gtk_grid_set_row_spacing (priv->content_vgrid, 6);
	gtk_widget_show (GTK_WIDGET (priv->content_vgrid));

	content_area = gtk_info_bar_get_content_area (GTK_INFO_BAR (info_bar));
	gtk_container_add (GTK_CONTAINER (content_area),
			   GTK_WIDGET (priv->content_vgrid));
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
 * gtef_info_bar_add_primary_message:
 * @info_bar: a #GtefInfoBar.
 * @primary_msg: a primary message.
 *
 * Adds a primary message.
 * Since: 1.2
 */
void
gtef_info_bar_add_primary_message (GtefInfoBar *info_bar,
				   const gchar *primary_msg)
{
	GtefInfoBarPrivate *priv;
	gchar *primary_msg_escaped;
	gchar *primary_markup;
	GtkLabel *primary_label;

	g_return_if_fail (GTEF_IS_INFO_BAR (info_bar));
	g_return_if_fail (primary_msg != NULL);

	priv = gtef_info_bar_get_instance_private (info_bar);

	primary_msg_escaped = g_markup_escape_text (primary_msg, -1);
	primary_markup = g_strdup_printf ("<b>%s</b>", primary_msg_escaped);
	primary_label = gtef_info_bar_create_label ();
	gtk_label_set_markup (primary_label, primary_markup);
	g_free (primary_markup);
	g_free (primary_msg_escaped);

	gtk_widget_show (GTK_WIDGET (primary_label));
	gtk_container_add (GTK_CONTAINER (priv->content_vgrid),
			   GTK_WIDGET (primary_label));
}

/**
 * gtef_info_bar_add_secondary_message:
 * @info_bar: a #GtefInfoBar.
 * @secondary_msg: a secondary message.
 *
 * Adds a secondary message.
 * Since: 1.2
 */
void
gtef_info_bar_add_secondary_message (GtefInfoBar *info_bar,
				     const gchar *secondary_msg)
{
	GtefInfoBarPrivate *priv;
	gchar *secondary_msg_escaped;
	gchar *secondary_markup;
	GtkLabel *secondary_label;

	g_return_if_fail (GTEF_IS_INFO_BAR (info_bar));
	g_return_if_fail (secondary_msg != NULL);

	priv = gtef_info_bar_get_instance_private (info_bar);

	secondary_msg_escaped = g_markup_escape_text (secondary_msg, -1);
	secondary_markup = g_strdup_printf ("<small>%s</small>", secondary_msg_escaped);
	secondary_label = gtef_info_bar_create_label ();
	gtk_label_set_markup (secondary_label, secondary_markup);
	g_free (secondary_markup);
	g_free (secondary_msg_escaped);

	gtk_widget_show (GTK_WIDGET (secondary_label));
	gtk_container_add (GTK_CONTAINER (priv->content_vgrid),
			   GTK_WIDGET (secondary_label));
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

	return label;
}
