/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2005 - Paolo Maggi
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

/* Modified version of GeditProgressInfoBar. */

#include "config.h"
#include "gtef-progress-info-bar.h"
#include <glib/gi18n-lib.h>

enum
{
	PROP_0,
	PROP_HAS_CANCEL_BUTTON,
	N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

struct _GtefProgressInfoBar
{
	GtefInfoBar parent_instance;

	GtkLabel *label;
	GtkProgressBar *progress_bar;
};

G_DEFINE_TYPE (GtefProgressInfoBar, _gtef_progress_info_bar, GTEF_TYPE_INFO_BAR)

static void
set_has_cancel_button (GtefProgressInfoBar *info_bar,
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
_gtef_progress_info_bar_set_property (GObject      *object,
				      guint         prop_id,
				      const GValue *value,
				      GParamSpec   *pspec)
{
	GtefProgressInfoBar *bar = GTEF_PROGRESS_INFO_BAR (object);

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
_gtef_progress_info_bar_class_init (GtefProgressInfoBarClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->set_property = _gtef_progress_info_bar_set_property;

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
_gtef_progress_info_bar_init (GtefProgressInfoBar *info_bar)
{
	GtkGrid *vgrid;
	GtkWidget *content_area;

	vgrid = GTK_GRID (gtk_grid_new ());
	gtk_orientable_set_orientation (GTK_ORIENTABLE (vgrid), GTK_ORIENTATION_VERTICAL);
	gtk_grid_set_row_spacing (vgrid, 6);

	info_bar->label = gtef_info_bar_create_label ();
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

GtefProgressInfoBar *
_gtef_progress_info_bar_new (const gchar *markup,
			     gboolean     has_cancel_button)
{
	GtefProgressInfoBar *info_bar;

	g_return_val_if_fail (markup != NULL, NULL);

	info_bar = g_object_new (GTEF_TYPE_PROGRESS_INFO_BAR,
				 "has-cancel-button", has_cancel_button,
				 NULL);

	_gtef_progress_info_bar_set_markup (info_bar, markup);

	return info_bar;
}

void
_gtef_progress_info_bar_set_markup (GtefProgressInfoBar *info_bar,
				    const gchar         *markup)
{
	g_return_if_fail (GTEF_IS_PROGRESS_INFO_BAR (info_bar));
	g_return_if_fail (markup != NULL);

	gtk_label_set_markup (info_bar->label, markup);
}

void
_gtef_progress_info_bar_set_text (GtefProgressInfoBar *info_bar,
				  const gchar         *text)
{
	g_return_if_fail (GTEF_IS_PROGRESS_INFO_BAR (info_bar));
	g_return_if_fail (text != NULL);

	gtk_label_set_text (info_bar->label, text);
}

void
_gtef_progress_info_bar_set_fraction (GtefProgressInfoBar *info_bar,
				      gdouble              fraction)
{
	g_return_if_fail (GTEF_IS_PROGRESS_INFO_BAR (info_bar));

	gtk_progress_bar_set_fraction (info_bar->progress_bar, fraction);
}

void
_gtef_progress_info_bar_pulse (GtefProgressInfoBar *info_bar)
{
	g_return_if_fail (GTEF_IS_PROGRESS_INFO_BAR (info_bar));

	gtk_progress_bar_pulse (info_bar->progress_bar);
}
