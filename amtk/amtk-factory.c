/*
 * This file is part of Amtk - Actions, Menus and Toolbars Kit
 *
 * Copyright 2017 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * Amtk is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * Amtk is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "amtk-factory.h"

/**
 * SECTION:amtk-factory
 * @Short_description: Factory base class
 * @Title: AmtkFactory
 *
 * #AmtkFactory is a base class to create #GtkWidget's from #AmtkActionInfo's.
 *
 * A #GtkApplication can be associated so that when a widget is created,
 * gtk_application_set_accels_for_action() is called.
 *
 * Once the widgets are created, an #AmtkFactory should be freed because it has
 * a strong reference to the #GtkApplication.
 */

struct _AmtkFactoryPrivate
{
	GtkApplication *app;
};

enum
{
	PROP_0,
	PROP_APPLICATION,
	N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

G_DEFINE_TYPE_WITH_PRIVATE (AmtkFactory, amtk_factory, G_TYPE_OBJECT)

static void
amtk_factory_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
	AmtkFactory *factory = AMTK_FACTORY (object);

	switch (prop_id)
	{
		case PROP_APPLICATION:
			g_value_set_object (value, amtk_factory_get_application (factory));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
amtk_factory_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
	AmtkFactory *factory = AMTK_FACTORY (object);

	switch (prop_id)
	{
		case PROP_APPLICATION:
			g_assert (factory->priv->app == NULL);
			factory->priv->app = g_value_dup_object (value);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
amtk_factory_dispose (GObject *object)
{
	AmtkFactory *factory = AMTK_FACTORY (object);

	g_clear_object (&factory->priv->app);

	G_OBJECT_CLASS (amtk_factory_parent_class)->dispose (object);
}

static void
amtk_factory_class_init (AmtkFactoryClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = amtk_factory_get_property;
	object_class->set_property = amtk_factory_set_property;
	object_class->dispose = amtk_factory_dispose;

	/**
	 * AmtkFactory:application:
	 *
	 * The associated #GtkApplication. #AmtkFactory has a strong reference
	 * to the #GtkApplication (which means that once the widgets are created
	 * you should free the #AmtkFactory).
	 *
	 * Since: 3.0
	 */
	properties[PROP_APPLICATION] =
		g_param_spec_object ("application",
				     "GtkApplication",
				     "",
				     GTK_TYPE_APPLICATION,
				     G_PARAM_READWRITE |
				     G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
amtk_factory_init (AmtkFactory *factory)
{
	factory->priv = amtk_factory_get_instance_private (factory);
}

/**
 * amtk_factory_get_application:
 * @factory: an #AmtkFactory.
 *
 * Returns: (transfer none): the #AmtkFactory:application.
 * Since: 3.0
 */
GtkApplication *
amtk_factory_get_application (AmtkFactory *factory)
{
	g_return_val_if_fail (AMTK_IS_FACTORY (factory), NULL);

	return factory->priv->app;
}
