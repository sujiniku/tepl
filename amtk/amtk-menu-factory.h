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

#ifndef AMTK_MENU_FACTORY_H
#define AMTK_MENU_FACTORY_H

#if !defined (AMTK_H_INSIDE) && !defined (AMTK_COMPILATION)
#error "Only <amtk/amtk.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <amtk/amtk-types.h>

G_BEGIN_DECLS

#define AMTK_TYPE_MENU_FACTORY             (amtk_menu_factory_get_type ())
#define AMTK_MENU_FACTORY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), AMTK_TYPE_MENU_FACTORY, AmtkMenuFactory))
#define AMTK_MENU_FACTORY_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), AMTK_TYPE_MENU_FACTORY, AmtkMenuFactoryClass))
#define AMTK_IS_MENU_FACTORY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AMTK_TYPE_MENU_FACTORY))
#define AMTK_IS_MENU_FACTORY_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), AMTK_TYPE_MENU_FACTORY))
#define AMTK_MENU_FACTORY_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), AMTK_TYPE_MENU_FACTORY, AmtkMenuFactoryClass))

typedef struct _AmtkMenuFactoryClass    AmtkMenuFactoryClass;
typedef struct _AmtkMenuFactoryPrivate  AmtkMenuFactoryPrivate;

struct _AmtkMenuFactory
{
	GObject parent;

	AmtkMenuFactoryPrivate *priv;
};

struct _AmtkMenuFactoryClass
{
	GObjectClass parent_class;
};

GType			amtk_menu_factory_get_type			(void);

AmtkMenuFactory *	amtk_menu_factory_new				(GtkApplication *application);

AmtkMenuFactory *	amtk_menu_factory_new_with_default_application	(void);

GtkApplication *	amtk_menu_factory_get_application		(AmtkMenuFactory *factory);

GtkWidget *		amtk_menu_factory_create_menu_item		(AmtkMenuFactory *factory,
									 const gchar     *action_name);

G_END_DECLS

#endif /* AMTK_MENU_FACTORY_H */
