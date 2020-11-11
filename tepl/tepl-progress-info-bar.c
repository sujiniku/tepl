/* SPDX-FileCopyrightText: 2005 - Paolo Maggi
 * SPDX-FileCopyrightText: 2016, 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

/* Inspired by GeditProgressInfoBar. */

#include "config.h"
#include "tepl-progress-info-bar.h"
#include <glib/gi18n-lib.h>

struct _TeplProgressInfoBarPrivate
{
	GtkLabel *label;
	GtkProgressBar *progress_bar;
	guint has_cancel_button : 1;
};

enum
{
	PROP_0,
	PROP_HAS_CANCEL_BUTTON,
	N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

G_DEFINE_TYPE_WITH_PRIVATE (TeplProgressInfoBar, tepl_progress_info_bar, TEPL_TYPE_INFO_BAR)

static void
set_has_cancel_button (TeplProgressInfoBar *info_bar,
		       gboolean             has_cancel_button)
{
	info_bar->priv->has_cancel_button = has_cancel_button != FALSE;

	if (has_cancel_button)
	{
		gtk_info_bar_add_button (GTK_INFO_BAR (info_bar),
					 _("_Cancel"),
					 GTK_RESPONSE_CANCEL);
	}
}

static void
tepl_progress_info_bar_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
	TeplProgressInfoBar *info_bar = TEPL_PROGRESS_INFO_BAR (object);

	switch (prop_id)
	{
		case PROP_HAS_CANCEL_BUTTON:
			g_value_set_boolean (value, info_bar->priv->has_cancel_button);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_progress_info_bar_set_property (GObject      *object,
				     guint         prop_id,
				     const GValue *value,
				     GParamSpec   *pspec)
{
	TeplProgressInfoBar *info_bar = TEPL_PROGRESS_INFO_BAR (object);

	switch (prop_id)
	{
		case PROP_HAS_CANCEL_BUTTON:
			set_has_cancel_button (info_bar, g_value_get_boolean (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_progress_info_bar_dispose (GObject *object)
{
	TeplProgressInfoBar *info_bar = TEPL_PROGRESS_INFO_BAR (object);

	info_bar->priv->label = NULL;
	info_bar->priv->progress_bar = NULL;

	G_OBJECT_CLASS (tepl_progress_info_bar_parent_class)->dispose (object);
}

static void
tepl_progress_info_bar_class_init (TeplProgressInfoBarClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = tepl_progress_info_bar_get_property;
	object_class->set_property = tepl_progress_info_bar_set_property;
	object_class->dispose = tepl_progress_info_bar_dispose;

	properties[PROP_HAS_CANCEL_BUTTON] =
		g_param_spec_boolean ("has-cancel-button",
				      "has-cancel-button",
				      "",
				      TRUE,
				      G_PARAM_READWRITE |
				      G_PARAM_CONSTRUCT_ONLY |
				      G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
tepl_progress_info_bar_init (TeplProgressInfoBar *info_bar)
{
	info_bar->priv = tepl_progress_info_bar_get_instance_private (info_bar);

	info_bar->priv->label = tepl_info_bar_create_label ();
	gtk_widget_show (GTK_WIDGET (info_bar->priv->label));
	tepl_info_bar_add_content_widget (TEPL_INFO_BAR (info_bar),
					  GTK_WIDGET (info_bar->priv->label),
					  TEPL_INFO_BAR_LOCATION_ALONGSIDE_ICON);

	info_bar->priv->progress_bar = GTK_PROGRESS_BAR (gtk_progress_bar_new ());
	gtk_widget_set_hexpand (GTK_WIDGET (info_bar->priv->progress_bar), TRUE);
	gtk_widget_show (GTK_WIDGET (info_bar->priv->progress_bar));
	tepl_info_bar_add_content_widget (TEPL_INFO_BAR (info_bar),
					  GTK_WIDGET (info_bar->priv->progress_bar),
					  TEPL_INFO_BAR_LOCATION_BELOW_ICON);
}

/**
 * tepl_progress_info_bar_new:
 * @icon_name: (nullable):
 * @markup: (nullable):
 * @has_cancel_button:
 *
 * Returns: a new #TeplProgressInfoBar.
 * Since: 6.0
 */
TeplProgressInfoBar *
tepl_progress_info_bar_new (const gchar *icon_name,
			    const gchar *markup,
			    gboolean     has_cancel_button)
{
	TeplProgressInfoBar *info_bar;

	info_bar = g_object_new (TEPL_TYPE_PROGRESS_INFO_BAR,
				 "icon-name", icon_name,
				 "has-cancel-button", has_cancel_button,
				 NULL);

	if (markup != NULL)
	{
		tepl_progress_info_bar_set_markup (info_bar, markup);
	}

	return info_bar;
}

void
tepl_progress_info_bar_set_markup (TeplProgressInfoBar *info_bar,
				   const gchar         *markup)
{
	g_return_if_fail (TEPL_IS_PROGRESS_INFO_BAR (info_bar));
	g_return_if_fail (markup != NULL);

	gtk_label_set_markup (info_bar->priv->label, markup);
}

void
tepl_progress_info_bar_set_text (TeplProgressInfoBar *info_bar,
				 const gchar         *text)
{
	g_return_if_fail (TEPL_IS_PROGRESS_INFO_BAR (info_bar));
	g_return_if_fail (text != NULL);

	gtk_label_set_text (info_bar->priv->label, text);
}

void
tepl_progress_info_bar_set_fraction (TeplProgressInfoBar *info_bar,
				     gdouble              fraction)
{
	g_return_if_fail (TEPL_IS_PROGRESS_INFO_BAR (info_bar));

	gtk_progress_bar_set_fraction (info_bar->priv->progress_bar, fraction);
}

void
tepl_progress_info_bar_pulse (TeplProgressInfoBar *info_bar)
{
	g_return_if_fail (TEPL_IS_PROGRESS_INFO_BAR (info_bar));

	gtk_progress_bar_pulse (info_bar->priv->progress_bar);
}
