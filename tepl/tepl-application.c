/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2017 - Sébastien Wilmet <swilmet@gnome.org>
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

#include "config.h"
#include "tepl-application.h"
#include <glib/gi18n-lib.h>
#include "tepl-action-info.h"
#include "tepl-action-info-store.h"

/**
 * SECTION:application
 * @Short_description: An extension of GtkApplication
 * @Title: TeplApplication
 * @See_also: #TeplActionInfoStore
 *
 * #TeplApplication extends the #GtkApplication class.
 *
 * Note that #TeplApplication extends the #GtkApplication class but without
 * subclassing it, because several libraries might want to extend
 * #GtkApplication and an application needs to be able to use all those
 * extensions at the same time.
 */

struct _TeplApplicationPrivate
{
	GtkApplication *gtk_app;
	TeplActionInfoStore *app_action_info_store;
	TeplActionInfoStore *tepl_action_info_store;
};

enum
{
	PROP_0,
	PROP_APPLICATION,
	N_PROPERTIES
};

#define TEPL_APPLICATION_KEY "tepl-application-key"

static GParamSpec *properties[N_PROPERTIES];

G_DEFINE_TYPE_WITH_PRIVATE (TeplApplication, tepl_application, G_TYPE_OBJECT)

static void
init_app_action_info_store (TeplApplication *tepl_app)
{
	g_return_if_fail (tepl_app->priv->app_action_info_store == NULL);
	g_assert (tepl_app->priv->gtk_app != NULL);

	tepl_app->priv->app_action_info_store = tepl_action_info_store_new (tepl_app->priv->gtk_app);
}

static void
init_tepl_action_info_store (TeplApplication *tepl_app)
{
	const TeplActionInfoEntry entries[] =
	{
		/* When adding an item to this array, do not forget to update
		 * the documentation of
		 * tepl_application_get_tepl_action_info_store().
		 */

		/* action, icon, label, accel, tooltip */

		{ "win.tepl-select-all", "edit-select-all", N_("Select _All"), "<Control>a",
		  N_("Select all the text") },
	};

	g_return_if_fail (tepl_app->priv->tepl_action_info_store == NULL);
	g_assert (tepl_app->priv->gtk_app != NULL);

	tepl_app->priv->tepl_action_info_store = tepl_action_info_store_new (tepl_app->priv->gtk_app);

	tepl_action_info_store_add_entries (tepl_app->priv->tepl_action_info_store,
					    entries,
					    G_N_ELEMENTS (entries),
					    GETTEXT_PACKAGE);
}

static void
tepl_application_get_property (GObject    *object,
			       guint       prop_id,
			       GValue     *value,
			       GParamSpec *pspec)
{
	TeplApplication *tepl_app = TEPL_APPLICATION (object);

	switch (prop_id)
	{
		case PROP_APPLICATION:
			g_value_set_object (value, tepl_application_get_application (tepl_app));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_application_set_property (GObject      *object,
			       guint         prop_id,
			       const GValue *value,
			       GParamSpec   *pspec)
{
	TeplApplication *tepl_app = TEPL_APPLICATION (object);

	switch (prop_id)
	{
		case PROP_APPLICATION:
			g_assert (tepl_app->priv->gtk_app == NULL);
			tepl_app->priv->gtk_app = g_value_get_object (value);

			init_app_action_info_store (tepl_app);
			init_tepl_action_info_store (tepl_app);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_application_dispose (GObject *object)
{
	TeplApplication *tepl_app = TEPL_APPLICATION (object);

	tepl_app->priv->gtk_app = NULL;
	g_clear_object (&tepl_app->priv->app_action_info_store);
	g_clear_object (&tepl_app->priv->tepl_action_info_store);

	G_OBJECT_CLASS (tepl_application_parent_class)->dispose (object);
}

static void
tepl_application_class_init (TeplApplicationClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = tepl_application_get_property;
	object_class->set_property = tepl_application_set_property;
	object_class->dispose = tepl_application_dispose;

	/**
	 * TeplApplication:application:
	 *
	 * The #GtkApplication.
	 *
	 * Since: 2.0
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
tepl_application_init (TeplApplication *tepl_app)
{
	tepl_app->priv = tepl_application_get_instance_private (tepl_app);
}

/**
 * tepl_application_get_from_gtk_application:
 * @gtk_app: a #GtkApplication.
 *
 * Returns the #TeplApplication of @gtk_app. The returned object is guaranteed
 * to be the same for the lifetime of @gtk_app.
 *
 * Returns: (transfer none): the #TeplApplication of @gtk_app.
 * Since: 2.0
 */
TeplApplication *
tepl_application_get_from_gtk_application (GtkApplication *gtk_app)
{
	TeplApplication *tepl_app;

	g_return_val_if_fail (GTK_IS_APPLICATION (gtk_app), NULL);

	tepl_app = g_object_get_data (G_OBJECT (gtk_app), TEPL_APPLICATION_KEY);

	if (tepl_app == NULL)
	{
		tepl_app = g_object_new (TEPL_TYPE_APPLICATION,
					 "application", gtk_app,
					 NULL);

		g_object_set_data_full (G_OBJECT (gtk_app),
					TEPL_APPLICATION_KEY,
					tepl_app,
					g_object_unref);
	}

	g_return_val_if_fail (TEPL_IS_APPLICATION (tepl_app), NULL);
	return tepl_app;
}

/**
 * tepl_application_get_default:
 *
 * Convenience function that calls g_application_get_default() followed by
 * tepl_application_get_from_gtk_application(). The object returned by
 * g_application_get_default() must be a #GtkApplication.
 *
 * Returns: (transfer none): the default #TeplApplication.
 * Since: 2.0
 */
TeplApplication *
tepl_application_get_default (void)
{
	GApplication *g_app;

	g_app = g_application_get_default ();
	g_return_val_if_fail (GTK_IS_APPLICATION (g_app), NULL);

	return tepl_application_get_from_gtk_application (GTK_APPLICATION (g_app));
}

/**
 * tepl_application_get_application:
 * @tepl_app: a #TeplApplication.
 *
 * Returns: (transfer none): the #GtkApplication of @tepl_app.
 * Since: 2.0
 */
GtkApplication *
tepl_application_get_application (TeplApplication *tepl_app)
{
	g_return_val_if_fail (TEPL_IS_APPLICATION (tepl_app), NULL);

	return tepl_app->priv->gtk_app;
}

/**
 * tepl_application_get_app_action_info_store:
 * @tepl_app: a #TeplApplication.
 *
 * Returns an initially empty #TeplActionInfoStore reserved for the
 * application-specific actions. Libraries should not add #TeplActionInfo's to
 * this store. Libraries should provide their own store if they want to share
 * #TeplActionInfo's.
 *
 * Returns: (transfer none): the #TeplActionInfoStore reserved for the
 * application.
 * Since: 2.0
 */
TeplActionInfoStore *
tepl_application_get_app_action_info_store (TeplApplication *tepl_app)
{
	g_return_val_if_fail (TEPL_IS_APPLICATION (tepl_app), NULL);

	return tepl_app->priv->app_action_info_store;
}

/**
 * tepl_application_get_tepl_action_info_store:
 * @tepl_app: a #TeplApplication.
 *
 * The returned #TeplActionInfoStore contains #TeplActionInfo's for the
 * following actions:
 * - `"win.tepl-select-all"`
 *
 * To know what the #GAction's do, see the class description of
 * #TeplApplicationWindow.
 *
 * Returns: (transfer none): the #TeplActionInfoStore of the Tepl library.
 * Since: 3.0
 */
TeplActionInfoStore *
tepl_application_get_tepl_action_info_store (TeplApplication *tepl_app)
{
	g_return_val_if_fail (TEPL_IS_APPLICATION (tepl_app), NULL);

	return tepl_app->priv->tepl_action_info_store;
}

/**
 * tepl_application_open_simple:
 * @tepl_app: a #TeplApplication.
 * @file: a #GFile.
 *
 * Calls g_application_open() with a single file and an empty hint.
 *
 * Since: 2.0
 */
void
tepl_application_open_simple (TeplApplication *tepl_app,
			      GFile           *file)
{
	GFile *files[1];

	g_return_if_fail (TEPL_IS_APPLICATION (tepl_app));
	g_return_if_fail (G_IS_FILE (file));

	files[0] = file;
	g_application_open (G_APPLICATION (tepl_app->priv->gtk_app), files, 1, "");
}

/* ex:set ts=8 noet: */
