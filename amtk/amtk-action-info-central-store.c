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

#include "amtk-action-info-central-store.h"
#include "amtk-action-info.h"

/**
 * SECTION:amtk-action-info-central-store
 * @Short_description: Aggregation of all #AmtkActionInfoStore's
 * @Title: AmtkActionInfoCentralStore
 * @See_also: #AmtkActionInfoStore
 *
 * #AmtkActionInfoCentralStore is a singleton class containing the aggregation
 * of all #AmtkActionInfoStore's. Each time an #AmtkActionInfo is added to an
 * #AmtkActionInfoStore, it is also added to the #AmtkActionInfoCentralStore.
 */

/* API design:
 *
 * Why both AmtkActionInfoStore and AmtkActionInfoCentralStore are needed?
 *
 * Advantage of AmtkActionInfoStore:
 * - amtk_action_info_store_check_all_used()
 *
 * Advantages of AmtkActionInfoCentralStore:
 * - The central store checks if there are no duplicated action names
 *   (globally).
 * - Permits to write AmtkFactory functions that don't require an
 *   AmtkActionInfoStore parameter.
 *
 * By having the two classes, we have the best of both worlds. We should not be
 * afraid to create a lot of classes, and see things in big, even if it's a bit
 * Java-ish.
 */

struct _AmtkActionInfoCentralStorePrivate
{
	/* Key: owned gchar*: action name.
	 * Value: owned AmtkActionInfo.
	 */
	GHashTable *hash_table;
};

static AmtkActionInfoCentralStore *singleton = NULL;

G_DEFINE_TYPE_WITH_PRIVATE (AmtkActionInfoCentralStore, amtk_action_info_central_store, G_TYPE_OBJECT)

static void
amtk_action_info_central_store_finalize (GObject *object)
{
	AmtkActionInfoCentralStore *central_store = AMTK_ACTION_INFO_CENTRAL_STORE (object);

	g_hash_table_unref (central_store->priv->hash_table);

	if (singleton == central_store)
	{
		singleton = NULL;
	}

	G_OBJECT_CLASS (amtk_action_info_central_store_parent_class)->finalize (object);
}

static void
amtk_action_info_central_store_class_init (AmtkActionInfoCentralStoreClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = amtk_action_info_central_store_finalize;
}

static void
amtk_action_info_central_store_init (AmtkActionInfoCentralStore *central_store)
{
	central_store->priv = amtk_action_info_central_store_get_instance_private (central_store);

	central_store->priv->hash_table = g_hash_table_new_full (g_str_hash,
								 g_str_equal,
								 g_free,
								 (GDestroyNotify) amtk_action_info_unref);
}

/**
 * amtk_action_info_central_store_get_singleton:
 *
 * Returns: (transfer none): the #AmtkActionInfoCentralStore singleton instance.
 * Since: 3.0
 */
AmtkActionInfoCentralStore *
amtk_action_info_central_store_get_singleton (void)
{
	if (G_UNLIKELY (singleton == NULL))
	{
		singleton = g_object_new (AMTK_TYPE_ACTION_INFO_CENTRAL_STORE, NULL);
	}

	return singleton;
}

void
_amtk_action_info_central_store_unref_singleton (void)
{
	if (singleton != NULL)
	{
		g_object_unref (singleton);
	}

	/* singleton is not set to NULL here, it is set to NULL in
	 * amtk_action_info_central_store_finalize() (i.e. when we are sure that
	 * the ref count reaches 0).
	 */
}

void
_amtk_action_info_central_store_add (AmtkActionInfoCentralStore *central_store,
				     AmtkActionInfo             *info)
{
	const gchar *action_name;

	g_return_if_fail (AMTK_IS_ACTION_INFO_CENTRAL_STORE (central_store));
	g_return_if_fail (info != NULL);

	action_name = amtk_action_info_get_action_name (info);
	g_return_if_fail (action_name != NULL);

	if (g_hash_table_lookup (central_store->priv->hash_table, action_name) != NULL)
	{
		g_warning ("The AmtkActionInfoCentralStore already contains an AmtkActionInfo "
			   "with the action name “%s”. Libraries must namespace their action names.",
			   action_name);
		return;
	}

	g_hash_table_insert (central_store->priv->hash_table,
			     g_strdup (action_name),
			     amtk_action_info_ref (info));
}

/**
 * amtk_action_info_central_store_lookup:
 * @central_store: the #AmtkActionInfoCentralStore.
 * @action_name: an action name.
 *
 * Returns: (transfer none): the found #AmtkActionInfo, or %NULL.
 * Since: 2.0
 */
AmtkActionInfo *
amtk_action_info_central_store_lookup (AmtkActionInfoCentralStore *central_store,
				       const gchar                *action_name)
{
	g_return_val_if_fail (AMTK_IS_ACTION_INFO_CENTRAL_STORE (central_store), NULL);
	g_return_val_if_fail (action_name != NULL, NULL);

	return g_hash_table_lookup (central_store->priv->hash_table, action_name);
}
