/* Copyright 2017 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_APPLICATION_H
#define TEPL_APPLICATION_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <amtk/amtk.h>
#include <tepl/tepl-macros.h>

G_BEGIN_DECLS

#define TEPL_TYPE_APPLICATION             (tepl_application_get_type ())
#define TEPL_APPLICATION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_APPLICATION, TeplApplication))
#define TEPL_APPLICATION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_APPLICATION, TeplApplicationClass))
#define TEPL_IS_APPLICATION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_APPLICATION))
#define TEPL_IS_APPLICATION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_APPLICATION))
#define TEPL_APPLICATION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_APPLICATION, TeplApplicationClass))

typedef struct _TeplApplication         TeplApplication;
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

_TEPL_EXTERN
GType			tepl_application_get_type			(void) G_GNUC_CONST;

_TEPL_EXTERN
TeplApplication *	tepl_application_get_from_gtk_application	(GtkApplication *gtk_app);

_TEPL_EXTERN
TeplApplication *	tepl_application_get_default			(void);

_TEPL_EXTERN
GtkApplication *	tepl_application_get_application		(TeplApplication *tepl_app);

_TEPL_EXTERN
AmtkActionInfoStore *	tepl_application_get_app_action_info_store	(TeplApplication *tepl_app);

_TEPL_EXTERN
AmtkActionInfoStore *	tepl_application_get_tepl_action_info_store	(TeplApplication *tepl_app);

_TEPL_EXTERN
GtkApplicationWindow *	tepl_application_get_active_main_window		(TeplApplication *tepl_app);

_TEPL_EXTERN
void			tepl_application_open_simple			(TeplApplication *tepl_app,
									 GFile           *file);

_TEPL_EXTERN
void			tepl_application_handle_activate		(TeplApplication *tepl_app);

_TEPL_EXTERN
void			tepl_application_handle_open			(TeplApplication *tepl_app);

_TEPL_EXTERN
void			tepl_application_handle_metadata		(TeplApplication *tepl_app);

G_END_DECLS

#endif /* TEPL_APPLICATION_H */
