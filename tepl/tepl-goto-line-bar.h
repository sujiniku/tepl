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

#ifndef TEPL_GOTO_LINE_BAR_H
#define TEPL_GOTO_LINE_BAR_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TEPL_TYPE_GOTO_LINE_BAR             (tepl_goto_line_bar_get_type ())
#define TEPL_GOTO_LINE_BAR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_GOTO_LINE_BAR, TeplGotoLineBar))
#define TEPL_GOTO_LINE_BAR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_GOTO_LINE_BAR, TeplGotoLineBarClass))
#define TEPL_IS_GOTO_LINE_BAR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_GOTO_LINE_BAR))
#define TEPL_IS_GOTO_LINE_BAR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_GOTO_LINE_BAR))
#define TEPL_GOTO_LINE_BAR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_GOTO_LINE_BAR, TeplGotoLineBarClass))

typedef struct _TeplGotoLineBar         TeplGotoLineBar;
typedef struct _TeplGotoLineBarClass    TeplGotoLineBarClass;
typedef struct _TeplGotoLineBarPrivate  TeplGotoLineBarPrivate;

struct _TeplGotoLineBar
{
	GtkGrid parent;

	TeplGotoLineBarPrivate *priv;
};

struct _TeplGotoLineBarClass
{
	GtkGridClass parent_class;

	gpointer padding[12];
};

GType			tepl_goto_line_bar_get_type	(void);

TeplGotoLineBar *	tepl_goto_line_bar_new		(void);

G_END_DECLS

#endif /* TEPL_GOTO_LINE_BAR_H */
