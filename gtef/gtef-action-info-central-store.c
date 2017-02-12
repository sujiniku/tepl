/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2017 - Sébastien Wilmet <swilmet@gnome.org>
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

#include "gtef-action-info-central-store.h"
#include "gtef-action-info-store.h"
#include "gtef-action-info.h"

/**
 * SECTION:action-info-central-store
 * @Short_description: Aggregation of all GtefActionInfoStore's
 * @Title: GtefActionInfoCentralStore
 * @See_also: #GtefActionInfoStore
 *
 * #GtefActionInfoCentralStore is a singleton class containing the aggregation
 * of all #GtefActionInfoStore's. Each time a #GtefActionInfo is added to a
 * #GtefActionInfoStore, it is also added to the #GtefActionInfoCentralStore.
 */

struct _GtefActionInfoCentralStorePrivate
{
	GtefActionInfoStore *store;
};

G_DEFINE_TYPE_WITH_PRIVATE (GtefActionInfoCentralStore, gtef_action_info_central_store, G_TYPE_OBJECT)

static void
gtef_action_info_central_store_finalize (GObject *object)
{
	GtefActionInfoCentralStore *central_store = GTEF_ACTION_INFO_CENTRAL_STORE (object);

	g_object_unref (central_store->priv->store);

	G_OBJECT_CLASS (gtef_action_info_central_store_parent_class)->finalize (object);
}

static void
gtef_action_info_central_store_class_init (GtefActionInfoCentralStoreClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = gtef_action_info_central_store_finalize;
}

static void
gtef_action_info_central_store_init (GtefActionInfoCentralStore *central_store)
{
	central_store->priv = gtef_action_info_central_store_get_instance_private (central_store);

	central_store->priv->store = gtef_action_info_store_new (NULL);
}

/**
 * gtef_action_info_central_store_get_instance:
 *
 * Returns: (transfer none): the #GtefActionInfoCentralStore singleton instance.
 * Since: 2.0
 */
GtefActionInfoCentralStore *
gtef_action_info_central_store_get_instance (void)
{
	static GtefActionInfoCentralStore *instance = NULL;

	if (G_UNLIKELY (instance == NULL))
	{
		instance = g_object_new (GTEF_TYPE_ACTION_INFO_CENTRAL_STORE, NULL);
	}

	return instance;
}

void
_gtef_action_info_central_store_add (GtefActionInfoCentralStore *central_store,
				     GtefActionInfo             *info)
{
	const gchar *action_name;

	g_return_if_fail (GTEF_IS_ACTION_INFO_CENTRAL_STORE (central_store));
	g_return_if_fail (info != NULL);

	action_name = gtef_action_info_get_action_name (info);
	g_return_if_fail (action_name != NULL);

	if (gtef_action_info_store_lookup (central_store->priv->store, action_name) != NULL)
	{
		g_warning ("The GtefActionInfoCentralStore already contains a GtefActionInfo "
			   "with the action name “%s”. Libraries must namespace their action names.",
			   action_name);
		return;
	}

	gtef_action_info_store_add (central_store->priv->store, info);
}

/**
 * gtef_action_info_central_store_lookup:
 * @central_store: a #GtefActionInfoCentralStore.
 * @action_name: an action name.
 *
 * Returns: (transfer none): the found #GtefActionInfo, or %NULL.
 * Since: 2.0
 */
const GtefActionInfo *
gtef_action_info_central_store_lookup (GtefActionInfoCentralStore *central_store,
				       const gchar                *action_name)
{
	g_return_val_if_fail (GTEF_IS_ACTION_INFO_CENTRAL_STORE (central_store), NULL);
	g_return_val_if_fail (action_name != NULL, NULL);

	return gtef_action_info_store_lookup (central_store->priv->store, action_name);
}
