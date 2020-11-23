/* SPDX-FileCopyrightText: 2017 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "config.h"
#include "tepl-tab-label.h"
#include <glib/gi18n-lib.h>
#include "tepl-buffer.h"
#include "tepl-file.h"
#include "tepl-signal-group.h"
#include "tepl-utils.h"

/**
 * SECTION:tab-label
 * @Short_description: A #TeplTab label, to use with #GtkNotebook
 * @Title: TeplTabLabel
 *
 * #TeplTabLabel is the label/title of a #TeplTab, suitable for #GtkNotebook.
 *
 * A #TeplTabLabel contains:
 * - a #GtkLabel with the #TeplBuffer:tepl-short-title.
 * - a close button, when clicked the #TeplTab #TeplTab::close-request signal is
 *   emitted.
 * - a customizable tooltip, by default it shows the full #TeplFile:location.
 */

struct _TeplTabLabelPrivate
{
	/* Weak ref */
	TeplTab *tab;

	TeplSignalGroup *buffer_signal_group;
	TeplSignalGroup *file_signal_group;

	GtkLabel *label;
};

enum
{
	PROP_0,
	PROP_TAB,
	N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

#define MAX_LABEL_CHARS_LENGTH (42)

G_DEFINE_TYPE_WITH_PRIVATE (TeplTabLabel, tepl_tab_label, GTK_TYPE_GRID)

static void
update_label (TeplTabLabel *tab_label)
{
	TeplBuffer *buffer;
	gchar *short_title;
	gchar *truncated_short_title;

	if (tab_label->priv->tab == NULL)
	{
		return;
	}

	buffer = tepl_tab_get_buffer (tab_label->priv->tab);
	short_title = tepl_buffer_get_short_title (buffer);

	/* A GtkNotebook tab label doesn't support well an ellipsizing GtkLabel.
	 * So, ellipsize ourself.
	 */
	truncated_short_title = tepl_utils_str_middle_truncate (short_title, MAX_LABEL_CHARS_LENGTH);

	gtk_label_set_text (tab_label->priv->label, truncated_short_title);

	g_free (truncated_short_title);
	g_free (short_title);
}

static void
buffer_short_title_notify_cb (TeplBuffer   *buffer,
			      GParamSpec   *pspec,
			      TeplTabLabel *tab_label)
{
	update_label (tab_label);
}

static void
file_location_notify_cb (TeplFile     *file,
			 GParamSpec   *pspec,
			 TeplTabLabel *tab_label)
{
	tepl_tab_label_update_tooltip (tab_label);
}

static void
buffer_changed (TeplTabLabel *tab_label)
{
	TeplBuffer *buffer;
	TeplFile *file;

	tepl_signal_group_clear (&tab_label->priv->buffer_signal_group);
	tepl_signal_group_clear (&tab_label->priv->file_signal_group);

	if (tab_label->priv->tab == NULL)
	{
		return;
	}

	/* Buffer */

	buffer = tepl_tab_get_buffer (tab_label->priv->tab);
	tab_label->priv->buffer_signal_group = tepl_signal_group_new (G_OBJECT (buffer));

	tepl_signal_group_add (tab_label->priv->buffer_signal_group,
			       g_signal_connect (buffer,
						 "notify::tepl-short-title",
						 G_CALLBACK (buffer_short_title_notify_cb),
						 tab_label));

	update_label (tab_label);

	/* File */

	file = tepl_buffer_get_file (buffer);
	tab_label->priv->file_signal_group = tepl_signal_group_new (G_OBJECT (file));

	tepl_signal_group_add (tab_label->priv->file_signal_group,
			       g_signal_connect (file,
						 "notify::location",
						 G_CALLBACK (file_location_notify_cb),
						 tab_label));

	tepl_tab_label_update_tooltip (tab_label);
}

static void
buffer_notify_cb (GtkTextView  *view,
		  GParamSpec   *pspec,
		  TeplTabLabel *tab_label)
{
	buffer_changed (tab_label);
}

static void
set_tab (TeplTabLabel *tab_label,
	 TeplTab      *tab)
{
	TeplView *view;

	if (tab == NULL)
	{
		return;
	}

	g_return_if_fail (TEPL_IS_TAB (tab));

	g_assert (tab_label->priv->tab == NULL);
	g_set_weak_pointer (&tab_label->priv->tab, tab);

	view = tepl_tab_get_view (tab);
	g_signal_connect_object (view,
				 "notify::buffer",
				 G_CALLBACK (buffer_notify_cb),
				 tab_label,
				 0);

	buffer_changed (tab_label);
}

static void
tepl_tab_label_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
	TeplTabLabel *tab_label = TEPL_TAB_LABEL (object);

	switch (prop_id)
	{
		case PROP_TAB:
			g_value_set_object (value, tepl_tab_label_get_tab (tab_label));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_tab_label_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
	TeplTabLabel *tab_label = TEPL_TAB_LABEL (object);

	switch (prop_id)
	{
		case PROP_TAB:
			set_tab (tab_label, g_value_get_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_tab_label_dispose (GObject *object)
{
	TeplTabLabel *tab_label = TEPL_TAB_LABEL (object);

	g_clear_weak_pointer (&tab_label->priv->tab);
	tepl_signal_group_clear (&tab_label->priv->buffer_signal_group);
	tepl_signal_group_clear (&tab_label->priv->file_signal_group);

	G_OBJECT_CLASS (tepl_tab_label_parent_class)->dispose (object);
}

static gchar *
tepl_tab_label_get_tooltip_markup_default (TeplTabLabel *tab_label)
{
	TeplBuffer *buffer;
	TeplFile *file;
	GFile *location;
	gchar *parse_name;
	gchar *parse_name_with_tilde;
	gchar *tooltip_markup;

	if (tab_label->priv->tab == NULL)
	{
		return NULL;
	}

	buffer = tepl_tab_get_buffer (tab_label->priv->tab);
	file = tepl_buffer_get_file (buffer);
	location = tepl_file_get_location (file);

	if (location == NULL)
	{
		return NULL;
	}

	parse_name = g_file_get_parse_name (location);
	parse_name_with_tilde = tepl_utils_replace_home_dir_with_tilde (parse_name);

	tooltip_markup = g_markup_printf_escaped ("<b>%s</b> %s",
						  /* Translators: location of a file. */
						  _("Location:"),
						  parse_name_with_tilde);

	g_free (parse_name_with_tilde);
	g_free (parse_name);

	return tooltip_markup;
}

static void
tepl_tab_label_class_init (TeplTabLabelClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = tepl_tab_label_get_property;
	object_class->set_property = tepl_tab_label_set_property;
	object_class->dispose = tepl_tab_label_dispose;

	klass->get_tooltip_markup = tepl_tab_label_get_tooltip_markup_default;

	/**
	 * TeplTabLabel:tab:
	 *
	 * The associated #TeplTab. #TeplTabLabel has a weak reference to the
	 * #TeplTab.
	 *
	 * Since: 3.0
	 */
	properties[PROP_TAB] =
		g_param_spec_object ("tab",
				     "tab",
				     "",
				     TEPL_TYPE_TAB,
				     G_PARAM_READWRITE |
				     G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
close_button_clicked_cb (GtkButton    *close_button,
			 TeplTabLabel *tab_label)
{
	if (tab_label->priv->tab != NULL)
	{
		g_signal_emit_by_name (tab_label->priv->tab, "close-request");
	}
}

static void
tepl_tab_label_init (TeplTabLabel *tab_label)
{
	GtkWidget *close_button;

	tab_label->priv = tepl_tab_label_get_instance_private (tab_label);

	/* Label */

	tab_label->priv->label = GTK_LABEL (gtk_label_new (NULL));
	gtk_widget_set_vexpand (GTK_WIDGET (tab_label->priv->label), TRUE);

	gtk_widget_show (GTK_WIDGET (tab_label->priv->label));
	gtk_container_add (GTK_CONTAINER (tab_label),
			   GTK_WIDGET (tab_label->priv->label));

	/* Close button */

	close_button = tepl_utils_create_close_button ();
	gtk_widget_set_tooltip_text (close_button, _("Close file"));

	g_signal_connect (close_button,
			  "clicked",
			  G_CALLBACK (close_button_clicked_cb),
			  tab_label);

	gtk_widget_show (close_button);
	gtk_container_add (GTK_CONTAINER (tab_label), close_button);
}

/**
 * tepl_tab_label_new:
 * @tab: a #TeplTab.
 *
 * Returns: (transfer floating): a new #TeplTabLabel.
 * Since: 3.0
 */
GtkWidget *
tepl_tab_label_new (TeplTab *tab)
{
	g_return_val_if_fail (TEPL_IS_TAB (tab), NULL);

	return g_object_new (TEPL_TYPE_TAB_LABEL,
			     "tab", tab,
			     NULL);
}

/**
 * tepl_tab_label_get_tab:
 * @tab_label: a #TeplTabLabel.
 *
 * Returns: (transfer none) (nullable): the #TeplTabLabel:tab.
 * Since: 3.0
 */
TeplTab *
tepl_tab_label_get_tab (TeplTabLabel *tab_label)
{
	g_return_val_if_fail (TEPL_IS_TAB_LABEL (tab_label), NULL);

	return tab_label->priv->tab;
}

/**
 * tepl_tab_label_update_tooltip:
 * @tab_label: a #TeplTabLabel.
 *
 * Asks #TeplTabLabel to update its tooltip. The ::get_tooltip_markup virtual
 * function is called and the result is set with
 * gtk_widget_set_tooltip_markup().
 *
 * Since: 3.0
 */
void
tepl_tab_label_update_tooltip (TeplTabLabel *tab_label)
{
	gchar *tooltip_markup;

	g_return_if_fail (TEPL_IS_TAB_LABEL (tab_label));

	tooltip_markup = TEPL_TAB_LABEL_GET_CLASS (tab_label)->get_tooltip_markup (tab_label);
	gtk_widget_set_tooltip_markup (GTK_WIDGET (tab_label), tooltip_markup);
	g_free (tooltip_markup);
}
