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

#ifndef GTEF_ACTION_INFO_STORE_H
#define GTEF_ACTION_INFO_STORE_H

#if !defined (GTEF_H_INSIDE) && !defined (GTEF_COMPILATION)
#error "Only <gtef/gtef.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <gtef/gtef-types.h>

G_BEGIN_DECLS

#define GTEF_TYPE_ACTION_INFO_STORE             (gtef_action_info_store_get_type ())
#define GTEF_ACTION_INFO_STORE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTEF_TYPE_ACTION_INFO_STORE, GtefActionInfoStore))
#define GTEF_ACTION_INFO_STORE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GTEF_TYPE_ACTION_INFO_STORE, GtefActionInfoStoreClass))
#define GTEF_IS_ACTION_INFO_STORE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTEF_TYPE_ACTION_INFO_STORE))
#define GTEF_IS_ACTION_INFO_STORE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GTEF_TYPE_ACTION_INFO_STORE))
#define GTEF_ACTION_INFO_STORE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GTEF_TYPE_ACTION_INFO_STORE, GtefActionInfoStoreClass))

typedef struct _GtefActionInfoStoreClass    GtefActionInfoStoreClass;
typedef struct _GtefActionInfoStorePrivate  GtefActionInfoStorePrivate;

struct _GtefActionInfoStore
{
	GObject parent;

	GtefActionInfoStorePrivate *priv;
};

struct _GtefActionInfoStoreClass
{
	GObjectClass parent_class;
};

GType			gtef_action_info_store_get_type			(void) G_GNUC_CONST;

GtefActionInfoStore *	gtef_action_info_store_new			(GtkApplication *application);

GtkApplication *	gtef_action_info_store_get_application		(GtefActionInfoStore *store);

void			gtef_action_info_store_add			(GtefActionInfoStore  *store,
									 const GtefActionInfo *info);

void			gtef_action_info_store_add_entries		(GtefActionInfoStore       *store,
									 const GtefActionInfoEntry *entries,
									 gint                       n_entries,
									 const gchar               *translation_domain);

const GtefActionInfo *	gtef_action_info_store_lookup			(GtefActionInfoStore *store,
									 const gchar         *action_name);

GtkWidget *		gtef_action_info_store_create_menu_item		(GtefActionInfoStore *store,
									 const gchar         *action_name);

G_END_DECLS

#endif /* GTEF_ACTION_INFO_STORE_H */
