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

#ifndef TEPL_APPLICATION_H
#define TEPL_APPLICATION_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <amtk/amtk.h>
#include <tepl/tepl-types.h>

G_BEGIN_DECLS

#define TEPL_TYPE_APPLICATION             (tepl_application_get_type ())
#define TEPL_APPLICATION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_APPLICATION, TeplApplication))
#define TEPL_APPLICATION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_APPLICATION, TeplApplicationClass))
#define TEPL_IS_APPLICATION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_APPLICATION))
#define TEPL_IS_APPLICATION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_APPLICATION))
#define TEPL_APPLICATION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_APPLICATION, TeplApplicationClass))

typedef struct _TeplApplicationClass    TeplApplicationClass;
typedef struct _TeplApplicationPrivate  TeplApplicationPrivate;

struct _TeplApplication
{
	GObject parent;

	TeplApplicationPrivate *priv;
};

struct _TeplApplicationClass
{
	GObjectClass parent_class;

	gpointer padding[12];
};

GType			tepl_application_get_type			(void) G_GNUC_CONST;

TeplApplication *	tepl_application_get_from_gtk_application	(GtkApplication *gtk_app);

TeplApplication *	tepl_application_get_default			(void);

GtkApplication *	tepl_application_get_application		(TeplApplication *tepl_app);

AmtkActionInfoStore *	tepl_application_get_app_action_info_store	(TeplApplication *tepl_app);

AmtkActionInfoStore *	tepl_application_get_tepl_action_info_store	(TeplApplication *tepl_app);

GtkApplicationWindow *	tepl_application_get_active_main_window		(TeplApplication *tepl_app);

void			tepl_application_open_simple			(TeplApplication *tepl_app,
									 GFile           *file);

G_END_DECLS

#endif /* TEPL_APPLICATION_H */

/* ex:set ts=8 noet: */
