/*
 * This file is part of Amtk, a text editor library.
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

#ifndef AMTK_ACTION_INFO_STORE_H
#define AMTK_ACTION_INFO_STORE_H

#if !defined (AMTK_H_INSIDE) && !defined (AMTK_COMPILATION)
#error "Only <amtk/amtk.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <amtk/amtk-types.h>

G_BEGIN_DECLS

#define AMTK_TYPE_ACTION_INFO_STORE             (amtk_action_info_store_get_type ())
#define AMTK_ACTION_INFO_STORE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), AMTK_TYPE_ACTION_INFO_STORE, AmtkActionInfoStore))
#define AMTK_ACTION_INFO_STORE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), AMTK_TYPE_ACTION_INFO_STORE, AmtkActionInfoStoreClass))
#define AMTK_IS_ACTION_INFO_STORE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AMTK_TYPE_ACTION_INFO_STORE))
#define AMTK_IS_ACTION_INFO_STORE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), AMTK_TYPE_ACTION_INFO_STORE))
#define AMTK_ACTION_INFO_STORE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), AMTK_TYPE_ACTION_INFO_STORE, AmtkActionInfoStoreClass))

typedef struct _AmtkActionInfoStoreClass    AmtkActionInfoStoreClass;
typedef struct _AmtkActionInfoStorePrivate  AmtkActionInfoStorePrivate;

struct _AmtkActionInfoStore
{
	GObject parent;

	AmtkActionInfoStorePrivate *priv;
};

struct _AmtkActionInfoStoreClass
{
	GObjectClass parent_class;

	gpointer padding[12];
};

GType			amtk_action_info_store_get_type			(void) G_GNUC_CONST;

AmtkActionInfoStore *	amtk_action_info_store_new			(GtkApplication *application);

GtkApplication *	amtk_action_info_store_get_application		(AmtkActionInfoStore *store);

void			amtk_action_info_store_add			(AmtkActionInfoStore *store,
									 AmtkActionInfo      *info);

void			amtk_action_info_store_add_entries		(AmtkActionInfoStore       *store,
									 const AmtkActionInfoEntry *entries,
									 gint                       n_entries,
									 const gchar               *translation_domain);

const AmtkActionInfo *	amtk_action_info_store_lookup			(AmtkActionInfoStore *store,
									 const gchar         *action_name);

GtkWidget *		amtk_action_info_store_create_menu_item		(AmtkActionInfoStore *store,
									 const gchar         *action_name);

void			amtk_action_info_store_check_all_used		(AmtkActionInfoStore *store);

G_END_DECLS

#endif /* AMTK_ACTION_INFO_STORE_H */
