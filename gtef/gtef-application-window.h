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

#ifndef GTEF_APPLICATION_WINDOW_H
#define GTEF_APPLICATION_WINDOW_H

#if !defined (GTEF_H_INSIDE) && !defined (GTEF_COMPILATION)
#error "Only <gtef/gtef.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <gtef/gtef-types.h>

G_BEGIN_DECLS

#define GTEF_TYPE_APPLICATION_WINDOW             (gtef_application_window_get_type ())
#define GTEF_APPLICATION_WINDOW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTEF_TYPE_APPLICATION_WINDOW, GtefApplicationWindow))
#define GTEF_APPLICATION_WINDOW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GTEF_TYPE_APPLICATION_WINDOW, GtefApplicationWindowClass))
#define GTEF_IS_APPLICATION_WINDOW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTEF_TYPE_APPLICATION_WINDOW))
#define GTEF_IS_APPLICATION_WINDOW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GTEF_TYPE_APPLICATION_WINDOW))
#define GTEF_APPLICATION_WINDOW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GTEF_TYPE_APPLICATION_WINDOW, GtefApplicationWindowClass))

typedef struct _GtefApplicationWindowClass    GtefApplicationWindowClass;
typedef struct _GtefApplicationWindowPrivate  GtefApplicationWindowPrivate;

struct _GtefApplicationWindow
{
	GObject parent;

	GtefApplicationWindowPrivate *priv;
};

struct _GtefApplicationWindowClass
{
	GObjectClass parent_class;

	gpointer padding[12];
};

GType			gtef_application_window_get_type			(void) G_GNUC_CONST;

GtefApplicationWindow *	gtef_application_window_get_from_gtk_application_window	(GtkApplicationWindow *gtk_window);

GtkApplicationWindow *	gtef_application_window_get_application_window		(GtefApplicationWindow *gtef_window);

GtkStatusbar *		gtef_application_window_get_statusbar			(GtefApplicationWindow *gtef_window);

void			gtef_application_window_set_statusbar			(GtefApplicationWindow *gtef_window,
										 GtkStatusbar          *statusbar);

void			gtef_application_window_connect_menu_to_statusbar	(GtefApplicationWindow *gtef_window,
										 GtefMenuShell         *gtef_menu_shell);

void			gtef_application_window_connect_recent_chooser_menu_to_statusbar
										(GtefApplicationWindow *gtef_window,
										 GtkRecentChooserMenu  *menu);

GtkWidget *		gtef_application_window_create_open_recent_menu_item	(GtefApplicationWindow *gtef_window);

G_END_DECLS

#endif /* GTEF_APPLICATION_WINDOW_H */

/* ex:set ts=8 noet: */
