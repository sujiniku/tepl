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

#ifndef TEPL_ACTION_INFO_STORE_H
#define TEPL_ACTION_INFO_STORE_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <tepl/tepl-types.h>

G_BEGIN_DECLS

#define TEPL_TYPE_ACTION_INFO_STORE             (tepl_action_info_store_get_type ())
#define TEPL_ACTION_INFO_STORE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_ACTION_INFO_STORE, TeplActionInfoStore))
#define TEPL_ACTION_INFO_STORE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_ACTION_INFO_STORE, TeplActionInfoStoreClass))
#define TEPL_IS_ACTION_INFO_STORE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_ACTION_INFO_STORE))
#define TEPL_IS_ACTION_INFO_STORE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_ACTION_INFO_STORE))
#define TEPL_ACTION_INFO_STORE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_ACTION_INFO_STORE, TeplActionInfoStoreClass))

typedef struct _TeplActionInfoStoreClass    TeplActionInfoStoreClass;
typedef struct _TeplActionInfoStorePrivate  TeplActionInfoStorePrivate;

struct _TeplActionInfoStore
{
	GObject parent;

	TeplActionInfoStorePrivate *priv;
};

struct _TeplActionInfoStoreClass
{
	GObjectClass parent_class;

	gpointer padding[12];
};

GType			tepl_action_info_store_get_type			(void) G_GNUC_CONST;

TeplActionInfoStore *	tepl_action_info_store_new			(GtkApplication *application);

GtkApplication *	tepl_action_info_store_get_application		(TeplActionInfoStore *store);

void			tepl_action_info_store_add			(TeplActionInfoStore *store,
									 TeplActionInfo      *info);

void			tepl_action_info_store_add_entries		(TeplActionInfoStore       *store,
									 const TeplActionInfoEntry *entries,
									 gint                       n_entries,
									 const gchar               *translation_domain);

const TeplActionInfo *	tepl_action_info_store_lookup			(TeplActionInfoStore *store,
									 const gchar         *action_name);

GtkWidget *		tepl_action_info_store_create_menu_item		(TeplActionInfoStore *store,
									 const gchar         *action_name);

void			tepl_action_info_store_check_all_used		(TeplActionInfoStore *store);

G_END_DECLS

#endif /* TEPL_ACTION_INFO_STORE_H */
