/* SPDX-FileCopyrightText: 2005 - Paolo Maggi
 * SPDX-FileCopyrightText: 2016 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

/* Modified version of GeditProgressInfoBar. */

#include "config.h"
#include "tepl-progress-info-bar.h"
#include <glib/gi18n-lib.h>

enum
{
	PROP_0,
	PROP_HAS_CANCEL_BUTTON,
	N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

struct _TeplProgressInfoBar
{
	TeplInfoBar parent_instance;

	GtkLabel *label;
	GtkProgressBar *progress_bar;
};

G_DEFINE_TYPE (TeplProgressInfoBar, _tepl_progress_info_bar, TEPL_TYPE_INFO_BAR)

static void
set_has_cancel_button (TeplProgressInfoBar *info_bar,
		       gboolean             has_cancel_button)
{
	if (has_cancel_button)
	{
		gtk_info_bar_add_button (GTK_INFO_BAR (info_bar),
					 _("_Cancel"),
					 GTK_RESPONSE_CANCEL);
	}
}

static void
_tepl_progress_info_bar_set_property (GObject      *object,
				      guint         prop_id,
				      const GValue *value,
				      GParamSpec   *pspec)
{
	TeplProgressInfoBar *bar = TEPL_PROGRESS_INFO_BAR (object);

	switch (prop_id)
	{
		case PROP_HAS_CANCEL_BUTTON:
			set_has_cancel_button (bar, g_value_get_boolean (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
_tepl_progress_info_bar_class_init (TeplProgressInfoBarClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->set_property = _tepl_progress_info_bar_set_property;

	properties[PROP_HAS_CANCEL_BUTTON] =
		g_param_spec_boolean ("has-cancel-button",
				      "Has Cancel Button",
				      "",
				      TRUE,
				      G_PARAM_WRITABLE |
				      G_PARAM_CONSTRUCT_ONLY |
				      G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
_tepl_progress_info_bar_init (TeplProgressInfoBar *info_bar)
{
	GtkGrid *vgrid;
	GtkWidget *content_area;

	vgrid = GTK_GRID (gtk_grid_new ());
	gtk_orientable_set_orientation (GTK_ORIENTABLE (vgrid), GTK_ORIENTATION_VERTICAL);
	gtk_grid_set_row_spacing (vgrid, 6);

	info_bar->label = tepl_info_bar_create_label ();
	gtk_container_add (GTK_CONTAINER (vgrid),
			   GTK_WIDGET (info_bar->label));

	info_bar->progress_bar = GTK_PROGRESS_BAR (gtk_progress_bar_new ());
	gtk_widget_set_hexpand (GTK_WIDGET (info_bar->progress_bar), TRUE);
	gtk_container_add (GTK_CONTAINER (vgrid),
			   GTK_WIDGET (info_bar->progress_bar));

	content_area = gtk_info_bar_get_content_area (GTK_INFO_BAR (info_bar));
	gtk_container_add (GTK_CONTAINER (content_area),
			   GTK_WIDGET (vgrid));

	gtk_widget_show_all (GTK_WIDGET (vgrid));
}

TeplProgressInfoBar *
_tepl_progress_info_bar_new (const gchar *markup,
			     gboolean     has_cancel_button)
{
	TeplProgressInfoBar *info_bar;

	g_return_val_if_fail (markup != NULL, NULL);

	info_bar = g_object_new (TEPL_TYPE_PROGRESS_INFO_BAR,
				 "has-cancel-button", has_cancel_button,
				 NULL);

	_tepl_progress_info_bar_set_markup (info_bar, markup);

	return info_bar;
}

void
_tepl_progress_info_bar_set_markup (TeplProgressInfoBar *info_bar,
				    const gchar         *markup)
{
	g_return_if_fail (TEPL_IS_PROGRESS_INFO_BAR (info_bar));
	g_return_if_fail (markup != NULL);

	gtk_label_set_markup (info_bar->label, markup);
}

void
_tepl_progress_info_bar_set_text (TeplProgressInfoBar *info_bar,
				  const gchar         *text)
{
	g_return_if_fail (TEPL_IS_PROGRESS_INFO_BAR (info_bar));
	g_return_if_fail (text != NULL);

	gtk_label_set_text (info_bar->label, text);
}

void
_tepl_progress_info_bar_set_fraction (TeplProgressInfoBar *info_bar,
				      gdouble              fraction)
{
	g_return_if_fail (TEPL_IS_PROGRESS_INFO_BAR (info_bar));

	gtk_progress_bar_set_fraction (info_bar->progress_bar, fraction);
}

void
_tepl_progress_info_bar_pulse (TeplProgressInfoBar *info_bar)
{
	g_return_if_fail (TEPL_IS_PROGRESS_INFO_BAR (info_bar));

	gtk_progress_bar_pulse (info_bar->progress_bar);
}
