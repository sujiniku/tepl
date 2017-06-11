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

#ifndef TEPL_MENU_SHELL_H
#define TEPL_MENU_SHELL_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <tepl/tepl-types.h>

G_BEGIN_DECLS

#define TEPL_TYPE_MENU_SHELL             (tepl_menu_shell_get_type ())
#define TEPL_MENU_SHELL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_MENU_SHELL, TeplMenuShell))
#define TEPL_MENU_SHELL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_MENU_SHELL, TeplMenuShellClass))
#define TEPL_IS_MENU_SHELL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_MENU_SHELL))
#define TEPL_IS_MENU_SHELL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_MENU_SHELL))
#define TEPL_MENU_SHELL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_MENU_SHELL, TeplMenuShellClass))

typedef struct _TeplMenuShellClass    TeplMenuShellClass;
typedef struct _TeplMenuShellPrivate  TeplMenuShellPrivate;

struct _TeplMenuShell
{
	GObject parent;

	TeplMenuShellPrivate *priv;
};

struct _TeplMenuShellClass
{
	GObjectClass parent_class;

	/* Signals */

	void (* menu_item_selected)	(TeplMenuShell *tepl_menu_shell,
					 GtkMenuItem   *menu_item);

	void (* menu_item_deselected)	(TeplMenuShell *tepl_menu_shell,
					 GtkMenuItem   *menu_item);

	gpointer padding[12];
};

GType		tepl_menu_shell_get_type		(void) G_GNUC_CONST;

TeplMenuShell *	tepl_menu_shell_get_from_gtk_menu_shell	(GtkMenuShell *gtk_menu_shell);

GtkMenuShell *	tepl_menu_shell_get_menu_shell		(TeplMenuShell *tepl_menu_shell);

G_END_DECLS

#endif /* TEPL_MENU_SHELL_H */

/* ex:set ts=8 noet: */
