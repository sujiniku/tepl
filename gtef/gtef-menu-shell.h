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

#ifndef GTEF_MENU_SHELL_H
#define GTEF_MENU_SHELL_H

#if !defined (GTEF_H_INSIDE) && !defined (GTEF_COMPILATION)
#error "Only <gtef/gtef.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <gtef/gtef-types.h>

G_BEGIN_DECLS

#define GTEF_TYPE_MENU_SHELL             (gtef_menu_shell_get_type ())
#define GTEF_MENU_SHELL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTEF_TYPE_MENU_SHELL, GtefMenuShell))
#define GTEF_MENU_SHELL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GTEF_TYPE_MENU_SHELL, GtefMenuShellClass))
#define GTEF_IS_MENU_SHELL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTEF_TYPE_MENU_SHELL))
#define GTEF_IS_MENU_SHELL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GTEF_TYPE_MENU_SHELL))
#define GTEF_MENU_SHELL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GTEF_TYPE_MENU_SHELL, GtefMenuShellClass))

typedef struct _GtefMenuShellClass    GtefMenuShellClass;
typedef struct _GtefMenuShellPrivate  GtefMenuShellPrivate;

struct _GtefMenuShell
{
	GObject parent;

	GtefMenuShellPrivate *priv;
};

struct _GtefMenuShellClass
{
	GObjectClass parent_class;

	/* Signals */

	void (* menu_item_selected)	(GtefMenuShell *gtef_menu_shell,
					 GtkMenuItem   *menu_item);

	void (* menu_item_deselected)	(GtefMenuShell *gtef_menu_shell,
					 GtkMenuItem   *menu_item);

	gpointer padding[12];
};

GType		gtef_menu_shell_get_type		(void) G_GNUC_CONST;

GtefMenuShell *	gtef_menu_shell_get_from_gtk_menu_shell	(GtkMenuShell *gtk_menu_shell);

GtkMenuShell *	gtef_menu_shell_get_menu_shell		(GtefMenuShell *gtef_menu_shell);

G_END_DECLS

#endif /* GTEF_MENU_SHELL_H */

/* ex:set ts=8 noet: */
