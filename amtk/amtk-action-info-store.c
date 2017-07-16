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

/**
 * SECTION:amtk-action-info-store
 * @Short_description: A store of #AmtkActionInfo's
 * @Title: AmtkActionInfoStore
 * @See_also: #AmtkActionInfo, #AmtkActionInfoCentralStore
 *
 * #AmtkActionInfoStore contains a set of #AmtkActionInfo's. It is add-only, an
 * #AmtkActionInfo cannot be removed.
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
	/* Key: owned gchar*: action name.
	 * Value: owned AmtkActionInfo.
	 */
	GHashTable *hash_table;
};

G_DEFINE_TYPE_WITH_PRIVATE (AmtkActionInfoStore, amtk_action_info_store, G_TYPE_OBJECT)

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

	object_class->finalize = amtk_action_info_store_finalize;
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
 *
 * Returns: a new #AmtkActionInfoStore.
 * Since: 3.0
 */
AmtkActionInfoStore *
amtk_action_info_store_new (void)
{
	return g_object_new (AMTK_TYPE_ACTION_INFO_STORE, NULL);
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

static void
check_used_cb (gpointer key,
	       gpointer value,
	       gpointer user_data)
{
	const gchar *action_name = key;
	const AmtkActionInfo *action_info = value;

	if (!amtk_action_info_has_been_used (action_info))
	{
		g_warning ("AmtkActionInfo with action_name='%s' has not been used.",
			   action_name);
	}
}

/**
 * amtk_action_info_store_check_all_used:
 * @store: an #AmtkActionInfoStore.
 *
 * Checks for each #AmtkActionInfo of @store that it has been used by a factory
 * to create a #GtkWidget (typically a menu or toolbar item). If an
 * #AmtkActionInfo has not been used, a warning is printed and might indicate
 * dead code.
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
