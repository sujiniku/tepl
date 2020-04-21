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

#include "tepl-style-scheme-chooser-widget.h"
#include <gtksourceview/gtksource.h>

struct _TeplStyleSchemeChooserWidgetPrivate
{
	gint something;
};

enum
{
	PROP_0,
	PROP_STYLE_SCHEME,
	N_PROPERTIES
};

static void gtk_source_style_scheme_chooser_interface_init (gpointer g_iface,
							    gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE (TeplStyleSchemeChooserWidget,
			 tepl_style_scheme_chooser_widget,
			 GTK_TYPE_BIN,
			 G_ADD_PRIVATE (TeplStyleSchemeChooserWidget)
			 G_IMPLEMENT_INTERFACE (GTK_SOURCE_TYPE_STYLE_SCHEME_CHOOSER,
						gtk_source_style_scheme_chooser_interface_init))

static void
tepl_style_scheme_chooser_widget_get_property (GObject    *object,
                                               guint       prop_id,
                                               GValue     *value,
                                               GParamSpec *pspec)
{
	GtkSourceStyleSchemeChooser *chooser = GTK_SOURCE_STYLE_SCHEME_CHOOSER (object);

	switch (prop_id)
	{
		case PROP_STYLE_SCHEME:
			g_value_set_object (value, gtk_source_style_scheme_chooser_get_style_scheme (chooser));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_style_scheme_chooser_widget_set_property (GObject      *object,
                                               guint         prop_id,
                                               const GValue *value,
                                               GParamSpec   *pspec)
{
	GtkSourceStyleSchemeChooser *chooser = GTK_SOURCE_STYLE_SCHEME_CHOOSER (object);

	switch (prop_id)
	{
		case PROP_STYLE_SCHEME:
			gtk_source_style_scheme_chooser_set_style_scheme (chooser, g_value_get_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_style_scheme_chooser_widget_dispose (GObject *object)
{

	G_OBJECT_CLASS (tepl_style_scheme_chooser_widget_parent_class)->dispose (object);
}

static void
tepl_style_scheme_chooser_widget_class_init (TeplStyleSchemeChooserWidgetClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = tepl_style_scheme_chooser_widget_get_property;
	object_class->set_property = tepl_style_scheme_chooser_widget_set_property;
	object_class->dispose = tepl_style_scheme_chooser_widget_dispose;

	g_object_class_override_property (object_class, PROP_STYLE_SCHEME, "style-scheme");
}

static GtkSourceStyleScheme *
tepl_style_scheme_chooser_widget_get_style_scheme (GtkSourceStyleSchemeChooser *chooser)
{
	return NULL;
}

static void
tepl_style_scheme_chooser_widget_set_style_scheme (GtkSourceStyleSchemeChooser *chooser,
						   GtkSourceStyleScheme        *scheme)
{
}

static void
gtk_source_style_scheme_chooser_interface_init (gpointer g_iface,
						gpointer iface_data)
{
	GtkSourceStyleSchemeChooserInterface *interface = g_iface;

	interface->get_style_scheme = tepl_style_scheme_chooser_widget_get_style_scheme;
	interface->set_style_scheme = tepl_style_scheme_chooser_widget_set_style_scheme;
}

static void
tepl_style_scheme_chooser_widget_init (TeplStyleSchemeChooserWidget *chooser)
{
	chooser->priv = tepl_style_scheme_chooser_widget_get_instance_private (chooser);
}

TeplStyleSchemeChooserWidget *
tepl_style_scheme_chooser_widget_new (void)
{
	return g_object_new (TEPL_TYPE_STYLE_SCHEME_CHOOSER_WIDGET, NULL);
}
