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

#ifndef TEPL_APPLICATION_WINDOW_H
#define TEPL_APPLICATION_WINDOW_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <tepl/tepl-types.h>

G_BEGIN_DECLS

#define TEPL_TYPE_APPLICATION_WINDOW             (tepl_application_window_get_type ())
#define TEPL_APPLICATION_WINDOW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_APPLICATION_WINDOW, TeplApplicationWindow))
#define TEPL_APPLICATION_WINDOW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_APPLICATION_WINDOW, TeplApplicationWindowClass))
#define TEPL_IS_APPLICATION_WINDOW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_APPLICATION_WINDOW))
#define TEPL_IS_APPLICATION_WINDOW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_APPLICATION_WINDOW))
#define TEPL_APPLICATION_WINDOW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_APPLICATION_WINDOW, TeplApplicationWindowClass))

typedef struct _TeplApplicationWindowClass    TeplApplicationWindowClass;
typedef struct _TeplApplicationWindowPrivate  TeplApplicationWindowPrivate;

struct _TeplApplicationWindow
{
	GObject parent;

	TeplApplicationWindowPrivate *priv;
};

struct _TeplApplicationWindowClass
{
	GObjectClass parent_class;

	gpointer padding[12];
};

GType			tepl_application_window_get_type			(void) G_GNUC_CONST;

TeplApplicationWindow *	tepl_application_window_get_from_gtk_application_window	(GtkApplicationWindow *gtk_window);

GtkApplicationWindow *	tepl_application_window_get_application_window		(TeplApplicationWindow *tepl_window);

void			tepl_application_window_set_tab_group			(TeplApplicationWindow *tepl_window,
										 TeplTabGroup          *tab_group);

gboolean		tepl_application_window_is_main_window			(GtkApplicationWindow *gtk_window);

gboolean		tepl_application_window_get_handle_title		(TeplApplicationWindow *tepl_window);

void			tepl_application_window_set_handle_title		(TeplApplicationWindow *tepl_window,
										 gboolean               handle_title);

void			tepl_application_window_open_file			(TeplApplicationWindow *tepl_window,
										 GFile                 *location);

G_END_DECLS

#endif /* TEPL_APPLICATION_WINDOW_H */

/* ex:set ts=8 noet: */
