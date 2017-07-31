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
#include "amtk-enum-types.h"

/**
 * SECTION:amtk-factory
 * @Short_description: Factory base class
 * @Title: AmtkFactory
 *
 * #AmtkFactory is a base class to create menu or toolbar items (or anything
 * else) from #AmtkActionInfo's. A factory function accesses an #AmtkActionInfo
 * from the #AmtkActionInfoCentralStore.
 *
 * A #GtkApplication can be associated so that factory functions can call
 * gtk_application_set_accels_for_action() with the accelerators returned by
 * amtk_action_info_get_accels() (this erases previously set accelerators for
 * that action, if any). Note that gtk_application_set_accels_for_action() is
 * called by factory functions and not by amtk_action_info_store_add(), so that
 * libraries can provide their own store and the accelerators are set to the
 * #GtkApplication only if an #AmtkActionInfo is actually used.
 *
 * #AmtkFactoryFlags permits to control how a factory function creates the
 * object, to ignore some steps. Factory functions should be declined in two
 * variants: a simple form which takes the value of the
 * #AmtkFactory:default-flags property, and the same function with the `_full`
 * suffix which takes an #AmtkFactoryFlags argument and ignores the
 * #AmtkFactory:default-flags. See for example
 * amtk_factory_menu_create_menu_item() and
 * amtk_factory_menu_create_menu_item_full().
 *
 * Once the objects are created, an #AmtkFactory should be freed because it has
 * a strong reference to the #GtkApplication. TODO: change it to a weak ref
 * instead so that this paragraph can be removed.
 */

struct _AmtkFactoryPrivate
{
	GtkApplication *app;
	AmtkFactoryFlags default_flags;
};

enum
{
	PROP_0,
	PROP_APPLICATION,
	PROP_DEFAULT_FLAGS,
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

		case PROP_DEFAULT_FLAGS:
			g_value_set_flags (value, amtk_factory_get_default_flags (factory));
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

		case PROP_DEFAULT_FLAGS:
			amtk_factory_set_default_flags (factory, g_value_get_flags (value));
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

	/**
	 * AmtkFactory:default-flags:
	 *
	 * The default #AmtkFactoryFlags.
	 *
	 * Since: 3.0
	 */
	properties[PROP_DEFAULT_FLAGS] =
		g_param_spec_flags ("default-flags",
				    "Default flags",
				    "",
				    AMTK_TYPE_FACTORY_FLAGS,
				    AMTK_FACTORY_FLAGS_NONE,
				    G_PARAM_READWRITE |
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
 * Returns: (transfer none) (nullable): the #AmtkFactory:application.
 * Since: 3.0
 */
GtkApplication *
amtk_factory_get_application (AmtkFactory *factory)
{
	g_return_val_if_fail (AMTK_IS_FACTORY (factory), NULL);

	return factory->priv->app;
}

/**
 * amtk_factory_get_default_flags:
 * @factory: an #AmtkFactory.
 *
 * Returns: the #AmtkFactory:default-flags.
 * Since: 3.0
 */
AmtkFactoryFlags
amtk_factory_get_default_flags (AmtkFactory *factory)
{
	g_return_val_if_fail (AMTK_IS_FACTORY (factory), AMTK_FACTORY_FLAGS_NONE);

	return factory->priv->default_flags;
}

/**
 * amtk_factory_set_default_flags:
 * @factory: an #AmtkFactory.
 * @default_flags: the new value.
 *
 * Sets the #AmtkFactory:default-flags property.
 *
 * Since: 3.0
 */
void
amtk_factory_set_default_flags (AmtkFactory      *factory,
				AmtkFactoryFlags  default_flags)
{
	g_return_if_fail (AMTK_IS_FACTORY (factory));

	if (factory->priv->default_flags != default_flags)
	{
		factory->priv->default_flags = default_flags;
		g_object_notify_by_pspec (G_OBJECT (factory), properties[PROP_DEFAULT_FLAGS]);
	}
}
