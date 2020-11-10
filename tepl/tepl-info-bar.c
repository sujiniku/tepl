/* SPDX-FileCopyrightText: 2016-2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "tepl-info-bar.h"

/**
 * SECTION:info-bar
 * @Title: TeplInfoBar
 * @Short_description: Subclass of #GtkInfoBar
 *
 * #TeplInfoBar is a subclass of #GtkInfoBar with functions to ease the creation
 * of info bars.
 */

/* To put later in the class description / the goal:
 *
 * The content area of a #TeplInfoBar contains a vertical container containing:
 * - First, an horizontal container containing:
 *   - A place for an optional icon.
 *   - A vertical container that can contain: primary/secondary messages plus
 *     additional widgets, in the order that they are added. So the widgets
 *     added here are <emphasis>on the right</emphasis> of the icon.
 * - Possible additional widgets, in the order that they are added. So the
 *   widgets added here are <emphasis>under</emphasis> the icon (if there is an
 *   icon).
 */

struct _TeplInfoBarPrivate
{
	GtkImage *icon;

	/* Can contain primary/secondary messages plus additional widgets. */
	GtkGrid *content_vgrid;

	guint icon_from_message_type : 1;
	guint handle_close_response : 1;
};

enum
{
	PROP_0,
	PROP_ICON_FROM_MESSAGE_TYPE,
	PROP_HANDLE_CLOSE_RESPONSE,
	N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

G_DEFINE_TYPE_WITH_PRIVATE (TeplInfoBar, tepl_info_bar, GTK_TYPE_INFO_BAR)

static void
tepl_info_bar_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
	TeplInfoBar *info_bar = TEPL_INFO_BAR (object);

	switch (prop_id)
	{
		case PROP_ICON_FROM_MESSAGE_TYPE:
			g_value_set_boolean (value, tepl_info_bar_get_icon_from_message_type (info_bar));
			break;

		case PROP_HANDLE_CLOSE_RESPONSE:
			g_value_set_boolean (value, tepl_info_bar_get_handle_close_response (info_bar));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_info_bar_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
	TeplInfoBar *info_bar = TEPL_INFO_BAR (object);

	switch (prop_id)
	{
		case PROP_ICON_FROM_MESSAGE_TYPE:
			tepl_info_bar_set_icon_from_message_type (info_bar, g_value_get_boolean (value));
			break;

		case PROP_HANDLE_CLOSE_RESPONSE:
			tepl_info_bar_set_handle_close_response (info_bar, g_value_get_boolean (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_info_bar_response (GtkInfoBar *gtk_info_bar,
			gint        response_id)
{
	TeplInfoBar *info_bar = TEPL_INFO_BAR (gtk_info_bar);

	if (response_id == GTK_RESPONSE_CLOSE &&
	    info_bar->priv->handle_close_response)
	{
		gtk_widget_destroy (GTK_WIDGET (info_bar));

		/* No need to chain up, the widget is destroyed. */
		return;
	}

	if (GTK_INFO_BAR_CLASS (tepl_info_bar_parent_class)->response != NULL)
	{
		GTK_INFO_BAR_CLASS (tepl_info_bar_parent_class)->response (gtk_info_bar,
									   response_id);
	}
}

static void
tepl_info_bar_class_init (TeplInfoBarClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkInfoBarClass *info_bar_class = GTK_INFO_BAR_CLASS (klass);

	object_class->get_property = tepl_info_bar_get_property;
	object_class->set_property = tepl_info_bar_set_property;

	info_bar_class->response = tepl_info_bar_response;

	/**
	 * TeplInfoBar:icon-from-message-type:
	 *
	 * If this property is %TRUE, then an icon is shown on the left, based
	 * on the value of the #GtkInfoBar:message-type property. For
	 * %GTK_MESSAGE_OTHER no icon is shown.
	 *
	 * Since: 6.0
	 */
	properties[PROP_ICON_FROM_MESSAGE_TYPE] =
		g_param_spec_boolean ("icon-from-message-type",
				      "icon-from-message-type",
				      "",
				      FALSE,
				      G_PARAM_READWRITE |
				      G_PARAM_CONSTRUCT |
				      G_PARAM_STATIC_STRINGS);

	/**
	 * TeplInfoBar:handle-close-response:
	 *
	 * If this property is %TRUE, then the #TeplInfoBar is destroyed with
	 * gtk_widget_destroy() when the #GtkInfoBar::response signal is
	 * received with the @response_id %GTK_RESPONSE_CLOSE.
	 *
	 * Since: 6.0
	 */
	properties[PROP_HANDLE_CLOSE_RESPONSE] =
		g_param_spec_boolean ("handle-close-response",
				      "handle-close-response",
				      "",
				      FALSE,
				      G_PARAM_READWRITE |
				      G_PARAM_CONSTRUCT |
				      G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
tepl_info_bar_init (TeplInfoBar *info_bar)
{
	GtkGrid *content_hgrid;
	GtkWidget *content_area;

	info_bar->priv = tepl_info_bar_get_instance_private (info_bar);

	/* info_bar config */
	_tepl_info_bar_set_size_request (GTK_INFO_BAR (info_bar));
	tepl_info_bar_set_buttons_orientation (GTK_INFO_BAR (info_bar), GTK_ORIENTATION_VERTICAL);

	/* Icon */
	info_bar->priv->icon = GTK_IMAGE (gtk_image_new ());
	gtk_widget_set_valign (GTK_WIDGET (info_bar->priv->icon), GTK_ALIGN_START);
	gtk_widget_set_no_show_all (GTK_WIDGET (info_bar->priv->icon), TRUE);

	/* priv->content_vgrid: can contain primary/secondary messages plus
	 * additional widgets.
	 */
	info_bar->priv->content_vgrid = GTK_GRID (gtk_grid_new ());
	gtk_orientable_set_orientation (GTK_ORIENTABLE (info_bar->priv->content_vgrid), GTK_ORIENTATION_VERTICAL);
	gtk_grid_set_row_spacing (info_bar->priv->content_vgrid, 6);
	gtk_widget_show (GTK_WIDGET (info_bar->priv->content_vgrid));

	/* content_hgrid: icon on the left, priv->content_vgrid on the right. */
	content_hgrid = GTK_GRID (gtk_grid_new ());
	gtk_orientable_set_orientation (GTK_ORIENTABLE (content_hgrid), GTK_ORIENTATION_HORIZONTAL);
	gtk_grid_set_column_spacing (content_hgrid, 16);

	gtk_container_add (GTK_CONTAINER (content_hgrid), GTK_WIDGET (info_bar->priv->icon));
	gtk_container_add (GTK_CONTAINER (content_hgrid), GTK_WIDGET (info_bar->priv->content_vgrid));
	gtk_widget_show (GTK_WIDGET (content_hgrid));

	/* content_area */
	content_area = gtk_info_bar_get_content_area (GTK_INFO_BAR (info_bar));
	gtk_container_add (GTK_CONTAINER (content_area), GTK_WIDGET (content_hgrid));
}

/**
 * tepl_info_bar_new:
 *
 * Returns: a new #TeplInfoBar.
 * Since: 1.0
 */
TeplInfoBar *
tepl_info_bar_new (void)
{
	return g_object_new (TEPL_TYPE_INFO_BAR, NULL);
}

/**
 * tepl_info_bar_new_simple:
 * @msg_type: the message type.
 * @primary_msg: the primary message.
 * @secondary_msg: (nullable): the secondary message, or %NULL.
 *
 * Creates a new #TeplInfoBar with an icon (depending on @msg_type), a primary
 * message and a secondary message.
 *
 * Returns: a new #TeplInfoBar.
 * Since: 2.0
 */
TeplInfoBar *
tepl_info_bar_new_simple (GtkMessageType  msg_type,
			  const gchar    *primary_msg,
			  const gchar    *secondary_msg)
{
	TeplInfoBar *info_bar;

	g_return_val_if_fail (primary_msg != NULL, NULL);

	info_bar = tepl_info_bar_new ();

	gtk_info_bar_set_message_type (GTK_INFO_BAR (info_bar), msg_type);
	tepl_info_bar_set_icon_from_message_type (info_bar, TRUE);
	tepl_info_bar_add_primary_message (info_bar, primary_msg);

	if (secondary_msg != NULL)
	{
		tepl_info_bar_add_secondary_message (info_bar, secondary_msg);
	}

	return info_bar;
}

static const gchar *
get_icon_name_for_message_type (TeplInfoBar *info_bar)
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

static void
update_icon_state (TeplInfoBar *info_bar)
{
	const gchar *icon_name;

	icon_name = get_icon_name_for_message_type (info_bar);

	if (info_bar->priv->icon_from_message_type &&
	    icon_name != NULL)
	{
		gtk_image_set_from_icon_name (info_bar->priv->icon, icon_name, GTK_ICON_SIZE_DIALOG);
		gtk_widget_show (GTK_WIDGET (info_bar->priv->icon));
	}
	else
	{
		gtk_widget_hide (GTK_WIDGET (info_bar->priv->icon));
	}
}

/**
 * tepl_info_bar_get_icon_from_message_type:
 * @info_bar: a #TeplInfoBar.
 *
 * Returns: the value of the #TeplInfoBar:icon-from-message-type property.
 * Since: 6.0
 */
gboolean
tepl_info_bar_get_icon_from_message_type (TeplInfoBar *info_bar)
{
	g_return_val_if_fail (TEPL_IS_INFO_BAR (info_bar), FALSE);

	return info_bar->priv->icon_from_message_type;
}

/**
 * tepl_info_bar_set_icon_from_message_type:
 * @info_bar: a #TeplInfoBar.
 * @icon_from_message_type: the new value.
 *
 * Sets a new value to the #TeplInfoBar:icon-from-message-type property.
 *
 * Since: 6.0
 */
void
tepl_info_bar_set_icon_from_message_type (TeplInfoBar *info_bar,
					  gboolean     icon_from_message_type)
{
	g_return_if_fail (TEPL_IS_INFO_BAR (info_bar));

	icon_from_message_type = icon_from_message_type != FALSE;

	if (info_bar->priv->icon_from_message_type != icon_from_message_type)
	{
		info_bar->priv->icon_from_message_type = icon_from_message_type;
		update_icon_state (info_bar);
		g_object_notify_by_pspec (G_OBJECT (info_bar), properties[PROP_ICON_FROM_MESSAGE_TYPE]);
	}
}

/**
 * tepl_info_bar_add_primary_message:
 * @info_bar: a #TeplInfoBar.
 * @primary_msg: a primary message.
 *
 * Adds a primary message.
 * Since: 2.0
 */
void
tepl_info_bar_add_primary_message (TeplInfoBar *info_bar,
				   const gchar *primary_msg)
{
	gchar *primary_msg_escaped;
	gchar *primary_markup;
	GtkLabel *primary_label;

	g_return_if_fail (TEPL_IS_INFO_BAR (info_bar));
	g_return_if_fail (primary_msg != NULL);

	primary_msg_escaped = g_markup_escape_text (primary_msg, -1);
	primary_markup = g_strdup_printf ("<b>%s</b>", primary_msg_escaped);
	primary_label = tepl_info_bar_create_label ();
	gtk_label_set_markup (primary_label, primary_markup);
	g_free (primary_markup);
	g_free (primary_msg_escaped);

	gtk_widget_show (GTK_WIDGET (primary_label));
	gtk_container_add (GTK_CONTAINER (info_bar->priv->content_vgrid),
			   GTK_WIDGET (primary_label));
}

/**
 * tepl_info_bar_add_secondary_message:
 * @info_bar: a #TeplInfoBar.
 * @secondary_msg: a secondary message.
 *
 * Adds a secondary message.
 * Since: 2.0
 */
void
tepl_info_bar_add_secondary_message (TeplInfoBar *info_bar,
				     const gchar *secondary_msg)
{
	gchar *secondary_msg_escaped;
	gchar *secondary_markup;
	GtkLabel *secondary_label;

	g_return_if_fail (TEPL_IS_INFO_BAR (info_bar));
	g_return_if_fail (secondary_msg != NULL);

	secondary_msg_escaped = g_markup_escape_text (secondary_msg, -1);
	secondary_markup = g_strdup_printf ("<small>%s</small>", secondary_msg_escaped);
	secondary_label = tepl_info_bar_create_label ();
	gtk_label_set_markup (secondary_label, secondary_markup);
	g_free (secondary_markup);
	g_free (secondary_msg_escaped);

	gtk_widget_show (GTK_WIDGET (secondary_label));
	gtk_container_add (GTK_CONTAINER (info_bar->priv->content_vgrid),
			   GTK_WIDGET (secondary_label));
}

/**
 * tepl_info_bar_add_content_widget:
 * @info_bar: a #TeplInfoBar.
 * @content: a #GtkWidget.
 *
 * Adds @content to @info_bar.
 *
 * #TeplInfoBar has an internal container, to be able to add the icon and add
 * primary or secondary messages. The internal container is added to the content
 * area, as returned by gtk_info_bar_get_content_area(). So if you use a
 * #TeplInfoBar and you need to add a custom #GtkWidget, it is better to use
 * this function instead of adding the #GtkWidget directly to the content area.
 *
 * Since: 2.0
 */
void
tepl_info_bar_add_content_widget (TeplInfoBar *info_bar,
				  GtkWidget   *content)
{
	g_return_if_fail (TEPL_IS_INFO_BAR (info_bar));
	g_return_if_fail (GTK_IS_WIDGET (content));

	gtk_container_add (GTK_CONTAINER (info_bar->priv->content_vgrid), content);
}

/**
 * tepl_info_bar_get_handle_close_response:
 * @info_bar: a #TeplInfoBar.
 *
 * Returns: the value of the #TeplInfoBar:handle-close-response property.
 * Since: 6.0
 */
gboolean
tepl_info_bar_get_handle_close_response (TeplInfoBar *info_bar)
{
	g_return_val_if_fail (TEPL_IS_INFO_BAR (info_bar), FALSE);

	return info_bar->priv->handle_close_response;
}

/**
 * tepl_info_bar_set_handle_close_response:
 * @info_bar: a #TeplInfoBar.
 * @handle_close_response: the new value.
 *
 * Sets a new value to the #TeplInfoBar:handle-close-response property.
 *
 * Since: 6.0
 */
void
tepl_info_bar_set_handle_close_response (TeplInfoBar *info_bar,
					 gboolean     handle_close_response)
{
	g_return_if_fail (TEPL_IS_INFO_BAR (info_bar));

	handle_close_response = handle_close_response != FALSE;

	if (info_bar->priv->handle_close_response != handle_close_response)
	{
		info_bar->priv->handle_close_response = handle_close_response;
		g_object_notify_by_pspec (G_OBJECT (info_bar), properties[PROP_HANDLE_CLOSE_RESPONSE]);
	}
}

/**
 * tepl_info_bar_setup_close_button:
 * @info_bar: a #TeplInfoBar.
 *
 * Convenience function to set the #GtkInfoBar:show-close-button and
 * #TeplInfoBar:handle-close-response properties to %TRUE.
 *
 * Since: 6.0
 */
void
tepl_info_bar_setup_close_button (TeplInfoBar *info_bar)
{
	g_return_if_fail (TEPL_IS_INFO_BAR (info_bar));

	gtk_info_bar_set_show_close_button (GTK_INFO_BAR (info_bar), TRUE);
	tepl_info_bar_set_handle_close_response (info_bar, TRUE);
}

/**
 * tepl_info_bar_set_buttons_orientation:
 * @info_bar: a #GtkInfoBar.
 * @buttons_orientation: the desired orientation.
 *
 * Sets the desired orientation (horizontal or vertical) for the action area as
 * returned by gtk_info_bar_get_action_area(). The action area is where the
 * buttons are placed.
 *
 * The default value for a #TeplInfoBar is %GTK_ORIENTATION_VERTICAL. The reason
 * is because with a small #GtkWindow, if 3 or more buttons are shown
 * horizontally, there is not enough space for the text. And it can be worse
 * when the button labels are translated to another language. When the buttons
 * are packed vertically, there is usually no problem. A vertical action area
 * also follows the original design of #GtkInfoBar.
 *
 * Since: 6.0
 */
void
tepl_info_bar_set_buttons_orientation (GtkInfoBar     *info_bar,
				       GtkOrientation  buttons_orientation)
{
	GtkWidget *action_area;

	g_return_if_fail (GTK_IS_INFO_BAR (info_bar));

	action_area = gtk_info_bar_get_action_area (info_bar);
	if (GTK_IS_ORIENTABLE (action_area))
	{
		gtk_orientable_set_orientation (GTK_ORIENTABLE (action_area),
						buttons_orientation);
	}
	else
	{
		g_warning ("Failed to set the orientation for the GtkInfoBar action area.");
	}
}

/**
 * tepl_info_bar_create_label:
 *
 * Utility function to create a #GtkLabel suitable for a #GtkInfoBar. The
 * wrapping and alignment is configured. The label is also set as selectable,
 * for example to copy an error message and search an explanation on the web.
 *
 * Returns: (transfer floating): a new #GtkLabel suitable for a #GtkInfoBar.
 * Since: 1.0
 */
GtkLabel *
tepl_info_bar_create_label (void)
{
	GtkLabel *label;

	label = GTK_LABEL (gtk_label_new (NULL));
	gtk_widget_set_halign (GTK_WIDGET (label), GTK_ALIGN_START);
	gtk_label_set_xalign (label, 0.0);
	gtk_label_set_line_wrap (label, TRUE);
	gtk_label_set_line_wrap_mode (label, PANGO_WRAP_WORD_CHAR);
	gtk_label_set_selectable (label, TRUE);

	/* Since the wrapping is enabled, we need to set a minimum width.
	 *
	 * If a minimum width is not set, adding an info bar to a container
	 * (e.g. a TeplTab) can make the GtkWindow height to grow. Because
	 * without a minimum width (and without ellipsization), when the user
	 * resizes the window (e.g. reducing the width) the widgets inside the
	 * window must be able to be drawn. When the info bar must be drawn with
	 * a width of e.g. 20 pixels, it takes a huge height because of the text
	 * wrapping. So by setting a minimum width to the label, the maximum
	 * height that the info bar can take is limited, so in most cases the
	 * GtkWindow current height is sufficient to draw the info bar with its
	 * maximum height.
	 *
	 * See:
	 * https://wiki.gnome.org/HowDoI/Labels
	 *
	 * There is also a safety net in tepl_tab_add_info_bar() which calls
	 * gtk_widget_set_size_request() on the GtkInfoBar, to set a minimum
	 * width.
	 */
	gtk_label_set_width_chars (label, 30);

	return label;
}

void
_tepl_info_bar_set_size_request (GtkInfoBar *info_bar)
{
	gint min_width;
	gint min_height;

	g_return_if_fail (GTK_IS_INFO_BAR (info_bar));

	gtk_widget_get_size_request (GTK_WIDGET (info_bar), &min_width, &min_height);

	/* If min_width != -1, gtk_widget_set_size_request() has already been
	 * called, so don't change the value.
	 */
	if (min_width == -1)
	{
		/* Safety net to avoid in most cases the GtkWindow height to
		 * grow.
		 *
		 * The gtk_label_set_width_chars() call in
		 * tepl_info_bar_create_label() fixes the problem at the root,
		 * but we cannot enforce all GtkLabel of @info_bar to be created
		 * with tepl_info_bar_create_label(), so a safety net is better.
		 */
		gtk_widget_set_size_request (GTK_WIDGET (info_bar), 300, min_height);
	}
}
