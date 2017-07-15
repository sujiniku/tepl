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

#include "amtk-action-info-store.h"
#include "amtk-action-info.h"
#include "amtk-action-info-central-store.h"
#include "amtk-menu-item.h"

/**
 * SECTION:amtk-action-info-store
 * @Short_description: A store of #AmtkActionInfo's
 * @Title: AmtkActionInfoStore
 * @See_also: #AmtkActionInfo, #AmtkActionInfoCentralStore
 *
 * #AmtkActionInfoStore contains a set of #AmtkActionInfo's.
 *
 * #AmtkActionInfoStore is add-only, a #AmtkActionInfo cannot be removed. If
 * needed, the remove operation will be added in the future.
 *
 * A #GtkApplication can be associated so that when a widget is created,
 * gtk_application_set_accels_for_action() is called. See
 * amtk_action_info_store_create_menu_item() for more details. Note that this
 * happens on widget creation, not when adding an #AmtkActionInfo to the store,
 * so that the accelerator is bound to the application only if the
 * #AmtkActionInfo is actually used.
 *
 * #AmtkActionInfoStore is designed so that libraries can provide their own
 * store, to share action information (with translations) and possibly the
 * #GAction implementations as well.
 *
 * A library #AmtkActionInfoStore must namespace the action names to not have
 * conflicts when an #AmtkActionInfo is added to the
 * #AmtkActionInfoCentralStore. Examples of namespaced action names:
 * `"win.amtk-save"` or `"app.amtk-quit"`.
 */

struct _AmtkActionInfoStorePrivate
{
	/* Weak ref, because usually GtkApplication owns (indirectly) a
	 * AmtkActionInfoStore.
	 */
	GtkApplication *app;

	/* Key: owned gchar*: action name.
	 * Value: owned AmtkActionInfo.
	 */
	GHashTable *hash_table;
};

enum
{
	PROP_0,
	PROP_APPLICATION,
	N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

G_DEFINE_TYPE_WITH_PRIVATE (AmtkActionInfoStore, amtk_action_info_store, G_TYPE_OBJECT)

static void
set_application (AmtkActionInfoStore *store,
		 GtkApplication      *app)
{
	g_return_if_fail (app == NULL || GTK_IS_APPLICATION (app));

	g_assert (store->priv->app == NULL);

	if (app == NULL)
	{
		return;
	}

	store->priv->app = app;
	g_object_add_weak_pointer (G_OBJECT (store->priv->app),
				   (gpointer *) &store->priv->app);

	g_object_notify_by_pspec (G_OBJECT (store), properties[PROP_APPLICATION]);
}

static void
amtk_action_info_store_get_property (GObject    *object,
				     guint       prop_id,
				     GValue     *value,
				     GParamSpec *pspec)
{
	AmtkActionInfoStore *store = AMTK_ACTION_INFO_STORE (object);

	switch (prop_id)
	{
		case PROP_APPLICATION:
			g_value_set_object (value, amtk_action_info_store_get_application (store));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
amtk_action_info_store_set_property (GObject      *object,
				     guint         prop_id,
				     const GValue *value,
				     GParamSpec   *pspec)
{
	AmtkActionInfoStore *store = AMTK_ACTION_INFO_STORE (object);

	switch (prop_id)
	{
		case PROP_APPLICATION:
			set_application (store, g_value_get_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
amtk_action_info_store_dispose (GObject *object)
{
	AmtkActionInfoStore *store = AMTK_ACTION_INFO_STORE (object);

	if (store->priv->app != NULL)
	{
		g_object_remove_weak_pointer (G_OBJECT (store->priv->app),
					      (gpointer *) &store->priv->app);
		store->priv->app = NULL;
	}

	G_OBJECT_CLASS (amtk_action_info_store_parent_class)->dispose (object);
}

static void
amtk_action_info_store_finalize (GObject *object)
{
	AmtkActionInfoStore *store = AMTK_ACTION_INFO_STORE (object);

	g_hash_table_unref (store->priv->hash_table);

	G_OBJECT_CLASS (amtk_action_info_store_parent_class)->finalize (object);
}

static void
amtk_action_info_store_class_init (AmtkActionInfoStoreClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = amtk_action_info_store_get_property;
	object_class->set_property = amtk_action_info_store_set_property;
	object_class->dispose = amtk_action_info_store_dispose;
	object_class->finalize = amtk_action_info_store_finalize;

	/**
	 * AmtkActionInfoStore:application:
	 *
	 * The associated #GtkApplication. #AmtkActionInfoStore has a weak
	 * reference to the #GtkApplication.
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
amtk_action_info_store_init (AmtkActionInfoStore *store)
{
	store->priv = amtk_action_info_store_get_instance_private (store);

	store->priv->hash_table = g_hash_table_new_full (g_str_hash,
							 g_str_equal,
							 g_free,
							 (GDestroyNotify) amtk_action_info_unref);
}

/**
 * amtk_action_info_store_new:
 * @application: (nullable): a #GtkApplication, or %NULL.
 *
 * Creates a new #AmtkActionInfoStore object. Associating a #GtkApplication is
 * optional.
 *
 * Returns: a new #AmtkActionInfoStore.
 * Since: 2.0
 */
AmtkActionInfoStore *
amtk_action_info_store_new (GtkApplication *application)
{
	g_return_val_if_fail (application == NULL || GTK_IS_APPLICATION (application), NULL);

	return g_object_new (AMTK_TYPE_ACTION_INFO_STORE,
			     "application", application,
			     NULL);
}

/**
 * amtk_action_info_store_get_application:
 * @store: an #AmtkActionInfoStore.
 *
 * Returns: (transfer none) (nullable): the associated #GtkApplication, or
 * %NULL.
 */
GtkApplication *
amtk_action_info_store_get_application (AmtkActionInfoStore *store)
{
	g_return_val_if_fail (AMTK_IS_ACTION_INFO_STORE (store), NULL);

	return store->priv->app;
}

/**
 * amtk_action_info_store_add:
 * @store: an #AmtkActionInfoStore.
 * @info: an #AmtkActionInfo.
 *
 * Inserts @info into @store and into the #AmtkActionInfoCentralStore. Both the
 * @store and central store must <emphasis>not</emphasis> already contain an
 * #AmtkActionInfo with the same action name. The stores take their own
 * reference on @info.
 *
 * Since: 2.0
 */
void
amtk_action_info_store_add (AmtkActionInfoStore *store,
			    AmtkActionInfo      *info)
{
	const gchar *action_name;
	AmtkActionInfoCentralStore *central_store;

	g_return_if_fail (AMTK_IS_ACTION_INFO_STORE (store));
	g_return_if_fail (info != NULL);

	action_name = amtk_action_info_get_action_name (info);
	g_return_if_fail (action_name != NULL);

	if (g_hash_table_lookup (store->priv->hash_table, action_name) != NULL)
	{
		g_warning ("%s(): the AmtkActionInfoStore already contains an AmtkActionInfo "
			   "with the action name “%s”.",
			   G_STRFUNC,
			   action_name);
		return;
	}

	g_hash_table_insert (store->priv->hash_table,
			     g_strdup (action_name),
			     amtk_action_info_ref (info));

	central_store = amtk_action_info_central_store_get_instance ();
	_amtk_action_info_central_store_add (central_store, info);
}

/**
 * amtk_action_info_store_add_entries:
 * @store: an #AmtkActionInfoStore.
 * @entries: (array length=n_entries) (element-type AmtkActionInfoEntry): a
 * pointer to the first item in an array of #AmtkActionInfoEntry structs.
 * @n_entries: the length of @entries, or -1 if @entries is %NULL-terminated.
 * @translation_domain: (nullable): a gettext domain, or %NULL.
 *
 * Calls amtk_action_info_store_add() for each entry.
 *
 * If @translation_domain is not %NULL, g_dgettext() is used to translate the
 * @label and @tooltip of each entry before setting them to the #AmtkActionInfo.
 *
 * An API similar to g_action_map_add_action_entries().
 *
 * Since: 2.0
 */
void
amtk_action_info_store_add_entries (AmtkActionInfoStore       *store,
				    const AmtkActionInfoEntry *entries,
				    gint                       n_entries,
				    const gchar               *translation_domain)
{
	gint i;

	g_return_if_fail (AMTK_IS_ACTION_INFO_STORE (store));
	g_return_if_fail (n_entries >= -1);
	g_return_if_fail (entries != NULL || n_entries == 0);

	for (i = 0; n_entries == -1 ? entries[i].action_name != NULL : i < n_entries; i++)
	{
		AmtkActionInfo *info;

		info = amtk_action_info_new_from_entry (&entries[i], translation_domain);
		amtk_action_info_store_add (store, info);
		amtk_action_info_unref (info);
	}
}

/**
 * amtk_action_info_store_lookup:
 * @store: an #AmtkActionInfoStore.
 * @action_name: an action name.
 *
 * Returns: (transfer none): the found #AmtkActionInfo, or %NULL.
 * Since: 2.0
 */
const AmtkActionInfo *
amtk_action_info_store_lookup (AmtkActionInfoStore *store,
			       const gchar         *action_name)
{
	g_return_val_if_fail (AMTK_IS_ACTION_INFO_STORE (store), NULL);
	g_return_val_if_fail (action_name != NULL, NULL);

	return g_hash_table_lookup (store->priv->hash_table, action_name);
}

/**
 * amtk_action_info_store_create_menu_item:
 * @store: an #AmtkActionInfoStore.
 * @action_name: an action name.
 *
 * Creates a new #GtkMenuItem for @action_name. The @store must contain an
 * #AmtkActionInfo for @action_name.
 *
 * gtk_actionable_set_action_name() is called on the menu item with
 * @action_name. The label is set with the #GtkMenuItem:use-underline property
 * enabled. The first accelerator is set to the #GtkAccelLabel of the menu item.
 * The icon is set. And the tooltip is set with
 * amtk_menu_item_set_long_description().
 *
 * If #AmtkActionInfoStore:application is non-%NULL, this function also calls
 * gtk_application_set_accels_for_action() with the accelerators returned by
 * amtk_action_info_get_accels() (this will erase previously set accelerators
 * for that action, if any).
 *
 * Returns: (transfer floating): a new #GtkMenuItem for @action_name.
 * Since: 2.0
 */
GtkWidget *
amtk_action_info_store_create_menu_item (AmtkActionInfoStore *store,
					 const gchar         *action_name)
{
	GtkMenuItem *menu_item;
	AmtkActionInfo *action_info;
	const gchar * const *accels;
	const gchar *icon_name;
	const gchar *tooltip;

	g_return_val_if_fail (AMTK_IS_ACTION_INFO_STORE (store), NULL);
	g_return_val_if_fail (action_name != NULL, NULL);

	action_info = g_hash_table_lookup (store->priv->hash_table, action_name);

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

	if (store->priv->app != NULL)
	{
		gtk_application_set_accels_for_action (store->priv->app,
						       action_name,
						       accels);
	}

	_amtk_action_info_set_used (action_info);

	return GTK_WIDGET (menu_item);
}

static void
check_used_cb (gpointer key,
	       gpointer value,
	       gpointer user_data)
{
	const gchar *action_name = key;
	const AmtkActionInfo *action_info = value;

	if (!_amtk_action_info_get_used (action_info))
	{
		g_warning ("AmtkActionInfo with action_name='%s' has not been used.",
			   action_name);
	}
}

/**
 * amtk_action_info_store_check_all_used:
 * @store: an #AmtkActionInfoStore.
 *
 * Checks that all #AmtkActionInfo's of @store have been used by
 * amtk_action_info_store_create_menu_item(). If not, a warning is printed and
 * might indicate dead code.
 *
 * You probably want to call this function on the application store. But it can
 * also be useful for a store provided by a library, to easily see which actions
 * you don't use.
 *
 * Since: 2.0
 */
void
amtk_action_info_store_check_all_used (AmtkActionInfoStore *store)
{
	g_return_if_fail (AMTK_IS_ACTION_INFO_STORE (store));

	g_hash_table_foreach (store->priv->hash_table,
			      check_used_cb,
			      NULL);
}
