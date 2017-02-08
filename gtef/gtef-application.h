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

#ifndef GTEF_APPLICATION_H
#define GTEF_APPLICATION_H

#include <gtk/gtk.h>
#include <gtef/gtef-types.h>

G_BEGIN_DECLS

#define GTEF_TYPE_APPLICATION             (gtef_application_get_type ())
#define GTEF_APPLICATION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTEF_TYPE_APPLICATION, GtefApplication))
#define GTEF_APPLICATION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GTEF_TYPE_APPLICATION, GtefApplicationClass))
#define GTEF_IS_APPLICATION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTEF_TYPE_APPLICATION))
#define GTEF_IS_APPLICATION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GTEF_TYPE_APPLICATION))
#define GTEF_APPLICATION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GTEF_TYPE_APPLICATION, GtefApplicationClass))

typedef struct _GtefApplication         GtefApplication;
typedef struct _GtefApplicationClass    GtefApplicationClass;
typedef struct _GtefApplicationPrivate  GtefApplicationPrivate;

struct _GtefApplication
{
	GObject parent;

	GtefApplicationPrivate *priv;
};

struct _GtefApplicationClass
{
	GObjectClass parent_class;
};

G_GNUC_INTERNAL
GType			gtef_application_get_type			(void) G_GNUC_CONST;

G_GNUC_INTERNAL
GtefApplication *	gtef_application_get_from_gtk_application	(GtkApplication *gtk_app);

G_GNUC_INTERNAL
GtkApplication *	gtef_application_get_application		(GtefApplication *gtef_app);

G_GNUC_INTERNAL
GtefActionInfoStore *	gtef_application_get_action_info_store		(GtefApplication *gtef_app);

G_END_DECLS

#endif /* GTEF_APPLICATION_H */

/* ex:set ts=8 noet: */
