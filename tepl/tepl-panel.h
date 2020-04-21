/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2020 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef TEPL_PANEL_H
#define TEPL_PANEL_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtk/gtk.h>

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

GType		tepl_panel_get_type	(void);

TeplPanel *	tepl_panel_new		(void);

G_END_DECLS

#endif /* TEPL_PANEL_H */
