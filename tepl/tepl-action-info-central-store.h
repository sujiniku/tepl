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

#ifndef TEPL_ACTION_INFO_CENTRAL_STORE_H
#define TEPL_ACTION_INFO_CENTRAL_STORE_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <glib-object.h>
#include <tepl/tepl-types.h>

G_BEGIN_DECLS

#define TEPL_TYPE_ACTION_INFO_CENTRAL_STORE             (tepl_action_info_central_store_get_type ())
#define TEPL_ACTION_INFO_CENTRAL_STORE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_ACTION_INFO_CENTRAL_STORE, TeplActionInfoCentralStore))
#define TEPL_ACTION_INFO_CENTRAL_STORE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_ACTION_INFO_CENTRAL_STORE, TeplActionInfoCentralStoreClass))
#define TEPL_IS_ACTION_INFO_CENTRAL_STORE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_ACTION_INFO_CENTRAL_STORE))
#define TEPL_IS_ACTION_INFO_CENTRAL_STORE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_ACTION_INFO_CENTRAL_STORE))
#define TEPL_ACTION_INFO_CENTRAL_STORE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_ACTION_INFO_CENTRAL_STORE, TeplActionInfoCentralStoreClass))

typedef struct _TeplActionInfoCentralStoreClass    TeplActionInfoCentralStoreClass;
typedef struct _TeplActionInfoCentralStorePrivate  TeplActionInfoCentralStorePrivate;

struct _TeplActionInfoCentralStore
{
	GObject parent;

	TeplActionInfoCentralStorePrivate *priv;
};

struct _TeplActionInfoCentralStoreClass
{
	GObjectClass parent_class;

	gpointer padding[12];
};

GType		tepl_action_info_central_store_get_type		(void) G_GNUC_CONST;

TeplActionInfoCentralStore *
		tepl_action_info_central_store_get_instance	(void);

const TeplActionInfo *
		tepl_action_info_central_store_lookup		(TeplActionInfoCentralStore *central_store,
								 const gchar                *action_name);

G_GNUC_INTERNAL
void		_tepl_action_info_central_store_add		(TeplActionInfoCentralStore *central_store,
								 TeplActionInfo             *info);

G_END_DECLS

#endif /* TEPL_ACTION_INFO_CENTRAL_STORE_H */
