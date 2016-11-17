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
	/* Left: icon. Right: content_vgrid. */
	GtkGrid *content_hgrid;

	/* Contains primary/secondary messages. */
	GtkGrid *content_vgrid;

	guint close_button_added : 1;
};

G_DEFINE_TYPE_WITH_PRIVATE (GtefInfoBar, gtef_info_bar, GTK_TYPE_INFO_BAR)

static void
gtef_info_bar_response (GtkInfoBar *gtk_info_bar,
			gint        response_id)
{
	GtefInfoBar *info_bar = GTEF_INFO_BAR (gtk_info_bar);
	GtefInfoBarPrivate *priv = gtef_info_bar_get_instance_private (info_bar);

	if (response_id == GTK_RESPONSE_CLOSE &&
	    priv->close_button_added)
	{
		gtk_widget_destroy (GTK_WIDGET (info_bar));

		/* No need to chain up, the widget is destroyed. */
		return;
	}

	if (GTK_INFO_BAR_CLASS (gtef_info_bar_parent_class)->response != NULL)
	{
		GTK_INFO_BAR_CLASS (gtef_info_bar_parent_class)->response (gtk_info_bar,
									   response_id);
	}
}

static void
gtef_info_bar_class_init (GtefInfoBarClass *klass)
{
	GtkInfoBarClass *info_bar_class = GTK_INFO_BAR_CLASS (klass);

	info_bar_class->response = gtef_info_bar_response;
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

	/* hgrid */
	priv->content_hgrid = GTK_GRID (gtk_grid_new ());
	gtk_orientable_set_orientation (GTK_ORIENTABLE (priv->content_hgrid),
					GTK_ORIENTATION_HORIZONTAL);
	gtk_grid_set_column_spacing (priv->content_hgrid, 16);
	gtk_widget_show (GTK_WIDGET (priv->content_hgrid));

	content_area = gtk_info_bar_get_content_area (GTK_INFO_BAR (info_bar));
	gtk_container_add (GTK_CONTAINER (content_area),
			   GTK_WIDGET (priv->content_hgrid));

	/* vgrid */
	priv->content_vgrid = GTK_GRID (gtk_grid_new ());
	gtk_orientable_set_orientation (GTK_ORIENTABLE (priv->content_vgrid),
					GTK_ORIENTATION_VERTICAL);
	gtk_grid_set_row_spacing (priv->content_vgrid, 6);
	gtk_widget_show (GTK_WIDGET (priv->content_vgrid));

	gtk_container_add (GTK_CONTAINER (priv->content_hgrid),
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
 * gtef_info_bar_new_simple:
 * @msg_type: the message type.
 * @primary_msg: the primary message.
 * @secondary_msg: (nullable): the secondary message, or %NULL.
 *
 * Creates a new #GtefInfoBar with an icon (depending on @msg_type), a primary
 * message and a secondary message.
 *
 * Returns: a new #GtefInfoBar.
 * Since: 1.2
 */
GtefInfoBar *
gtef_info_bar_new_simple (GtkMessageType  msg_type,
			  const gchar    *primary_msg,
			  const gchar    *secondary_msg)
{
	GtefInfoBar *info_bar;

	g_return_val_if_fail (primary_msg != NULL, NULL);

	info_bar = gtef_info_bar_new ();

	gtk_info_bar_set_message_type (GTK_INFO_BAR (info_bar), msg_type);
	gtef_info_bar_add_icon (info_bar);
	gtef_info_bar_add_primary_message (info_bar, primary_msg);

	if (secondary_msg != NULL)
	{
		gtef_info_bar_add_secondary_message (info_bar, secondary_msg);
	}

	return info_bar;
}

static const gchar *
get_icon_name (GtefInfoBar *info_bar)
{
	GtkMessageType msg_type;

	msg_type = gtk_info_bar_get_message_type (GTK_INFO_BAR (info_bar));

	switch (msg_type)
	{
		case GTK_MESSAGE_INFO:
			return "dialog-information";

		case GTK_MESSAGE_WARNING:
			return "dialog-warning";

		case GTK_MESSAGE_QUESTION:
			return "dialog-question";

		case GTK_MESSAGE_ERROR:
			return "dialog-error";

		case GTK_MESSAGE_OTHER:
		default:
			/* No icon */
			break;
	}

	return NULL;
}

/**
 * gtef_info_bar_add_icon:
 * @info_bar: a #GtefInfoBar.
 *
 * Adds an icon on the left, determined by the message type. So before calling
 * this function, gtk_info_bar_set_message_type() must have been called.
 *
 * The icon is not updated when the message type changes. Another #GtefInfoBar
 * must be created in that case.
 *
 * Since: 1.2
 */
void
gtef_info_bar_add_icon (GtefInfoBar *info_bar)
{
	GtefInfoBarPrivate *priv;
	const gchar *icon_name;
	GtkWidget *image;

	g_return_if_fail (GTEF_IS_INFO_BAR (info_bar));

	priv = gtef_info_bar_get_instance_private (info_bar);

	icon_name = get_icon_name (info_bar);
	if (icon_name == NULL)
	{
		return;
	}

	image = gtk_image_new_from_icon_name (icon_name, GTK_ICON_SIZE_DIALOG);
	gtk_widget_set_valign (image, GTK_ALIGN_START);
	gtk_widget_show (image);

	gtk_grid_attach_next_to (priv->content_hgrid,
				 image,
				 GTK_WIDGET (priv->content_vgrid),
				 GTK_POS_LEFT,
				 1,
				 1);
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
 * gtef_info_bar_add_content_widget:
 * @info_bar: a #GtefInfoBar.
 * @content: a #GtkWidget.
 *
 * Adds @content to @info_bar.
 *
 * #GtefInfoBar has an internal container, to be able to add the icon and add
 * primary or secondary messages. The internal container is added to the content
 * area, as returned by gtk_info_bar_get_content_area(). So if you use a
 * #GtefInfoBar and you need to add a custom #GtkWidget, it is better to use
 * this function instead of adding the #GtkWidget directly to the content area.
 *
 * Since: 1.2
 */
void
gtef_info_bar_add_content_widget (GtefInfoBar *info_bar,
				  GtkWidget   *content)
{
	GtefInfoBarPrivate *priv;

	g_return_if_fail (GTEF_IS_INFO_BAR (info_bar));
	g_return_if_fail (GTK_IS_WIDGET (content));

	priv = gtef_info_bar_get_instance_private (info_bar);

	gtk_container_add (GTK_CONTAINER (priv->content_vgrid), content);
}

/**
 * gtef_info_bar_add_close_button:
 * @info_bar: a #GtefInfoBar.
 *
 * Calls gtk_info_bar_set_show_close_button(), and additionnally closes the
 * @info_bar when the #GtkInfoBar::response signal is received with the
 * @response_id %GTK_RESPONSE_CLOSE.
 *
 * Since: 1.2
 */
void
gtef_info_bar_add_close_button (GtefInfoBar *info_bar)
{
	GtefInfoBarPrivate *priv;

	g_return_if_fail (GTEF_IS_INFO_BAR (info_bar));

	priv = gtef_info_bar_get_instance_private (info_bar);

	gtk_info_bar_set_show_close_button (GTK_INFO_BAR (info_bar), TRUE);

	priv->close_button_added = TRUE;
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
