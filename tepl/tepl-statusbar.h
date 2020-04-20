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

#ifndef TEPL_STATUSBAR_H
#define TEPL_STATUSBAR_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <tepl/tepl-tab-group.h>

G_BEGIN_DECLS

#define TEPL_TYPE_STATUSBAR             (tepl_statusbar_get_type ())
#define TEPL_STATUSBAR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_STATUSBAR, TeplStatusbar))
#define TEPL_STATUSBAR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_STATUSBAR, TeplStatusbarClass))
#define TEPL_IS_STATUSBAR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_STATUSBAR))
#define TEPL_IS_STATUSBAR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_STATUSBAR))
#define TEPL_STATUSBAR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_STATUSBAR, TeplStatusbarClass))

typedef struct _TeplStatusbar         TeplStatusbar;
typedef struct _TeplStatusbarClass    TeplStatusbarClass;
typedef struct _TeplStatusbarPrivate  TeplStatusbarPrivate;

struct _TeplStatusbar
{
	GtkStatusbar parent;

	TeplStatusbarPrivate *priv;
};

struct _TeplStatusbarClass
{
	GtkStatusbarClass parent_class;

	gpointer padding[12];
};

GType		tepl_statusbar_get_type			(void);

TeplStatusbar *	tepl_statusbar_new			(void);

void		tepl_statusbar_show_cursor_position	(TeplStatusbar *statusbar,
							 gint           line,
							 gint           column);

void		tepl_statusbar_hide_cursor_position	(TeplStatusbar *statusbar);

void		tepl_statusbar_set_tab_group		(TeplStatusbar *statusbar,
							 TeplTabGroup  *tab_group);

G_END_DECLS

#endif /* TEPL_STATUSBAR_H */
