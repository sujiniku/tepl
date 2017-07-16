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

#include "amtk-menu-factory.h"
#include "amtk-action-info.h"
#include "amtk-action-info-central-store.h"
#include "amtk-menu-item.h"

/**
 * SECTION:amtk-menu-factory
 * @Short_description: A factory that creates #GtkMenuItem's
 * @Title: AmtkMenuFactory
 *
 * #AmtkMenuFactory permits to create #GtkMenuItem's from #AmtkActionInfo's.
 *
 * A #GtkApplication can be associated so that when a menu item is created,
 * gtk_application_set_accels_for_action() is called. See
 * amtk_menu_factory_create_menu_item() for more details.
 */

struct _AmtkMenuFactoryPrivate
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

G_DEFINE_TYPE_WITH_PRIVATE (AmtkMenuFactory, amtk_menu_factory, G_TYPE_OBJECT)

static void
amtk_menu_factory_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
	AmtkMenuFactory *factory = AMTK_MENU_FACTORY (object);

	switch (prop_id)
	{
		case PROP_APPLICATION:
			g_value_set_object (value, amtk_menu_factory_get_application (factory));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
amtk_menu_factory_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
	AmtkMenuFactory *factory = AMTK_MENU_FACTORY (object);

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
amtk_menu_factory_dispose (GObject *object)
{
	AmtkMenuFactory *factory = AMTK_MENU_FACTORY (object);

	g_clear_object (&factory->priv->app);

	G_OBJECT_CLASS (amtk_menu_factory_parent_class)->dispose (object);
}

static void
amtk_menu_factory_class_init (AmtkMenuFactoryClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = amtk_menu_factory_get_property;
	object_class->set_property = amtk_menu_factory_set_property;
	object_class->dispose = amtk_menu_factory_dispose;

	/**
	 * AmtkMenuFactory:application:
	 *
	 * The associated #GtkApplication. #AmtkMenuFactory has a strong
	 * reference to the #GtkApplication (which means that once the menu is
	 * created you should free the #AmtkMenuFactory).
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
amtk_menu_factory_init (AmtkMenuFactory *factory)
{
	factory->priv = amtk_menu_factory_get_instance_private (factory);
}

/**
 * amtk_menu_factory_new:
 * @application: (nullable): a #GtkApplication, or %NULL.
 *
 * Creates a new #AmtkMenuFactory object. Associating a #GtkApplication is
 * optional.
 *
 * Returns: a new #AmtkMenuFactory.
 * Since: 3.0
 */
AmtkMenuFactory *
amtk_menu_factory_new (GtkApplication *application)
{
	g_return_val_if_fail (application == NULL || GTK_IS_APPLICATION (application), NULL);

	return g_object_new (AMTK_TYPE_MENU_FACTORY,
			     "application", application,
			     NULL);
}

/**
 * amtk_menu_factory_new_with_default_application:
 *
 * Calls amtk_menu_factory_new() with g_application_get_default().
 *
 * Returns: a new #AmtkMenuFactory with the default #GtkApplication.
 * Since: 3.0
 */
AmtkMenuFactory *
amtk_menu_factory_new_with_default_application (void)
{
	return amtk_menu_factory_new (GTK_APPLICATION (g_application_get_default ()));
}

/**
 * amtk_menu_factory_get_application:
 * @factory: an #AmtkMenuFactory.
 *
 * Returns: (transfer none): the #AmtkMenuFactory:application.
 * Since: 3.0
 */
GtkApplication *
amtk_menu_factory_get_application (AmtkMenuFactory *factory)
{
	g_return_val_if_fail (AMTK_IS_MENU_FACTORY (factory), NULL);

	return factory->priv->app;
}

/**
 * amtk_menu_factory_create_menu_item:
 * @factory: an #AmtkMenuFactory.
 * @action_name: an action name.
 *
 * Creates a new #GtkMenuItem for @action_name. The #AmtkActionInfoCentralStore
 * must contain an #AmtkActionInfo for @action_name.
 *
 * gtk_actionable_set_action_name() is called on the menu item with
 * @action_name. The label is set with the #GtkMenuItem:use-underline property
 * enabled. The first accelerator is set to the #GtkAccelLabel of the menu item.
 * The icon is set. And the tooltip is set with
 * amtk_menu_item_set_long_description().
 *
 * If the #AmtkMenuFactory:application is non-%NULL, this function also calls
 * gtk_application_set_accels_for_action() with the accelerators returned by
 * amtk_action_info_get_accels() (this will erase previously set accelerators
 * for that action, if any).
 *
 * Returns: (transfer floating): a new #GtkMenuItem for @action_name.
 * Since: 3.0
 */
GtkWidget *
amtk_menu_factory_create_menu_item (AmtkMenuFactory *factory,
				    const gchar     *action_name)
{
	AmtkActionInfoCentralStore *central_store;
	const AmtkActionInfo *action_info;
	GtkMenuItem *menu_item;
	const gchar * const *accels;
	const gchar *icon_name;
	const gchar *tooltip;

	g_return_val_if_fail (AMTK_IS_MENU_FACTORY (factory), NULL);
	g_return_val_if_fail (action_name != NULL, NULL);

	central_store = amtk_action_info_central_store_get_instance ();
	action_info = amtk_action_info_central_store_lookup (central_store, action_name);

	if (action_info == NULL)
	{
		g_warning ("%s(): action name '%s' not found.",
			   G_STRFUNC,
			   action_name);

		return NULL;
	}

	menu_item = GTK_MENU_ITEM (gtk_menu_item_new ());

	gtk_actionable_set_action_name (GTK_ACTIONABLE (menu_item), action_name);

	gtk_menu_item_set_use_underline (menu_item, TRUE);
	gtk_menu_item_set_label (menu_item, amtk_action_info_get_label (action_info));

	/* Set accel before setting icon, because
	 * amtk_menu_item_set_icon_name() adds a GtkBox.
	 */
	accels = amtk_action_info_get_accels (action_info);
	if (accels != NULL && accels[0] != NULL)
	{
		guint accel_key;
		GdkModifierType accel_mods;

		gtk_accelerator_parse (accels[0], &accel_key, &accel_mods);

		if (accel_key != 0 || accel_mods != 0)
		{
			GtkWidget *child;

			child = gtk_bin_get_child (GTK_BIN (menu_item));

			gtk_accel_label_set_accel (GTK_ACCEL_LABEL (child),
						   accel_key,
						   accel_mods);
		}
	}

	icon_name = amtk_action_info_get_icon_name (action_info);
	if (icon_name != NULL)
	{
		amtk_menu_item_set_icon_name (menu_item, icon_name);
	}

	tooltip = amtk_action_info_get_tooltip (action_info);
	if (tooltip != NULL)
	{
		amtk_menu_item_set_long_description (menu_item, tooltip);
	}

	if (factory->priv->app != NULL)
	{
		gtk_application_set_accels_for_action (factory->priv->app,
						       action_name,
						       accels);
	}

	/* FIXME: we are cheating a little here. Maybe the lookup functions
	 * should not return const values. And _amtk_action_info_set_used()
	 * should be public so factory functions can be written in apps.
	 */
	_amtk_action_info_set_used ((AmtkActionInfo *) action_info);

	return GTK_WIDGET (menu_item);
}
