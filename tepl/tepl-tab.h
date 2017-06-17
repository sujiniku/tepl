/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2016, 2017 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef TEPL_TAB_H
#define TEPL_TAB_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <tepl/tepl-types.h>

G_BEGIN_DECLS

#define TEPL_TYPE_TAB             (tepl_tab_get_type ())
#define TEPL_TAB(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_TAB, TeplTab))
#define TEPL_TAB_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_TAB, TeplTabClass))
#define TEPL_IS_TAB(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_TAB))
#define TEPL_IS_TAB_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_TAB))
#define TEPL_TAB_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_TAB, TeplTabClass))

typedef struct _TeplTabClass    TeplTabClass;
typedef struct _TeplTabPrivate  TeplTabPrivate;

struct _TeplTab
{
	GtkGrid parent;

	TeplTabPrivate *priv;
};

struct _TeplTabClass
{
	GtkGridClass parent_class;

	gpointer padding[12];
};

GType			tepl_tab_get_type				(void);

TeplTab *		tepl_tab_new					(TeplView *view);

TeplView *		tepl_tab_get_view				(TeplTab *tab);

void			tepl_tab_add_info_bar				(TeplTab    *tab,
									 GtkInfoBar *info_bar);

G_END_DECLS

#endif /* TEPL_TAB_H */
