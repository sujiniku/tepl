/* Copyright 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_PANEL_H
#define TEPL_PANEL_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <tepl/tepl-macros.h>

G_BEGIN_DECLS

#define TEPL_TYPE_PANEL             (tepl_panel_get_type ())
#define TEPL_PANEL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_PANEL, TeplPanel))
#define TEPL_PANEL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_PANEL, TeplPanelClass))
#define TEPL_IS_PANEL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_PANEL))
#define TEPL_IS_PANEL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_PANEL))
#define TEPL_PANEL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_PANEL, TeplPanelClass))

typedef struct _TeplPanel         TeplPanel;
typedef struct _TeplPanelClass    TeplPanelClass;
typedef struct _TeplPanelPrivate  TeplPanelPrivate;

struct _TeplPanel
{
	GtkGrid parent;

	TeplPanelPrivate *priv;
};

struct _TeplPanelClass
{
	GtkGridClass parent_class;

	gpointer padding[12];
};

_TEPL_EXTERN
GType		tepl_panel_get_type				(void);

_TEPL_EXTERN
TeplPanel *	tepl_panel_new					(void);

_TEPL_EXTERN
TeplPanel *	tepl_panel_new_for_left_side_panel		(void);

_TEPL_EXTERN
GtkStack *	tepl_panel_get_stack				(TeplPanel *panel);

_TEPL_EXTERN
void		tepl_panel_add_component			(TeplPanel   *panel,
								 GtkWidget   *component,
								 const gchar *name,
								 const gchar *title,
								 const gchar *icon_name);

_TEPL_EXTERN
void		tepl_panel_provide_active_component_gsetting	(TeplPanel   *panel,
								 GSettings   *settings,
								 const gchar *setting_key);

_TEPL_EXTERN
void		tepl_panel_restore_state_from_gsettings		(TeplPanel *panel);

_TEPL_EXTERN
void		tepl_panel_save_state_to_gsettings 		(TeplPanel *panel);

G_END_DECLS

#endif /* TEPL_PANEL_H */
