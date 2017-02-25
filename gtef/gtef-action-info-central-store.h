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

#ifndef GTEF_ACTION_INFO_CENTRAL_STORE_H
#define GTEF_ACTION_INFO_CENTRAL_STORE_H

#if !defined (GTEF_H_INSIDE) && !defined (GTEF_COMPILATION)
#error "Only <gtef/gtef.h> can be included directly."
#endif

#include <glib-object.h>
#include <gtef/gtef-types.h>

G_BEGIN_DECLS

#define GTEF_TYPE_ACTION_INFO_CENTRAL_STORE             (gtef_action_info_central_store_get_type ())
#define GTEF_ACTION_INFO_CENTRAL_STORE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTEF_TYPE_ACTION_INFO_CENTRAL_STORE, GtefActionInfoCentralStore))
#define GTEF_ACTION_INFO_CENTRAL_STORE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GTEF_TYPE_ACTION_INFO_CENTRAL_STORE, GtefActionInfoCentralStoreClass))
#define GTEF_IS_ACTION_INFO_CENTRAL_STORE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTEF_TYPE_ACTION_INFO_CENTRAL_STORE))
#define GTEF_IS_ACTION_INFO_CENTRAL_STORE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GTEF_TYPE_ACTION_INFO_CENTRAL_STORE))
#define GTEF_ACTION_INFO_CENTRAL_STORE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GTEF_TYPE_ACTION_INFO_CENTRAL_STORE, GtefActionInfoCentralStoreClass))

typedef struct _GtefActionInfoCentralStoreClass    GtefActionInfoCentralStoreClass;
typedef struct _GtefActionInfoCentralStorePrivate  GtefActionInfoCentralStorePrivate;

struct _GtefActionInfoCentralStore
{
	GObject parent;

	GtefActionInfoCentralStorePrivate *priv;
};

struct _GtefActionInfoCentralStoreClass
{
	GObjectClass parent_class;

	gpointer padding[12];
};

GType		gtef_action_info_central_store_get_type		(void) G_GNUC_CONST;

GtefActionInfoCentralStore *
		gtef_action_info_central_store_get_instance	(void);

const GtefActionInfo *
		gtef_action_info_central_store_lookup		(GtefActionInfoCentralStore *central_store,
								 const gchar                *action_name);

G_GNUC_INTERNAL
void		_gtef_action_info_central_store_add		(GtefActionInfoCentralStore *central_store,
								 GtefActionInfo             *info);

G_END_DECLS

#endif /* GTEF_ACTION_INFO_CENTRAL_STORE_H */
