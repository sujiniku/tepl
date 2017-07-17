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

#ifndef AMTK_FACTORY_MENU_H
#define AMTK_FACTORY_MENU_H

#if !defined (AMTK_H_INSIDE) && !defined (AMTK_COMPILATION)
#error "Only <amtk/amtk.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <amtk/amtk-types.h>

G_BEGIN_DECLS

#define AMTK_TYPE_FACTORY_MENU             (amtk_factory_menu_get_type ())
#define AMTK_FACTORY_MENU(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), AMTK_TYPE_FACTORY_MENU, AmtkFactoryMenu))
#define AMTK_FACTORY_MENU_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), AMTK_TYPE_FACTORY_MENU, AmtkFactoryMenuClass))
#define AMTK_IS_FACTORY_MENU(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AMTK_TYPE_FACTORY_MENU))
#define AMTK_IS_FACTORY_MENU_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), AMTK_TYPE_FACTORY_MENU))
#define AMTK_FACTORY_MENU_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), AMTK_TYPE_FACTORY_MENU, AmtkFactoryMenuClass))

typedef struct _AmtkFactoryMenuClass    AmtkFactoryMenuClass;
typedef struct _AmtkFactoryMenuPrivate  AmtkFactoryMenuPrivate;

struct _AmtkFactoryMenu
{
	GObject parent;

	AmtkFactoryMenuPrivate *priv;
};

struct _AmtkFactoryMenuClass
{
	GObjectClass parent_class;
};

GType			amtk_factory_menu_get_type			(void);

AmtkFactoryMenu *	amtk_factory_menu_new				(GtkApplication *application);

AmtkFactoryMenu *	amtk_factory_menu_new_with_default_application	(void);

GtkApplication *	amtk_factory_menu_get_application		(AmtkFactoryMenu *factory);

GtkWidget *		amtk_factory_menu_create_menu_item		(AmtkFactoryMenu *factory,
									 const gchar     *action_name);

G_END_DECLS

#endif /* AMTK_FACTORY_MENU_H */
