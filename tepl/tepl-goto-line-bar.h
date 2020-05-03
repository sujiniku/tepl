/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_GOTO_LINE_BAR_H
#define TEPL_GOTO_LINE_BAR_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <tepl/tepl-view.h>

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

_TEPL_EXTERN
GType			tepl_goto_line_bar_get_type			(void);

_TEPL_EXTERN
TeplGotoLineBar *	tepl_goto_line_bar_new				(void);

_TEPL_EXTERN
void			tepl_goto_line_bar_set_view			(TeplGotoLineBar *bar,
									 TeplView        *view);

_TEPL_EXTERN
void			tepl_goto_line_bar_grab_focus_to_entry		(TeplGotoLineBar *bar);

G_GNUC_INTERNAL
void			_tepl_goto_line_bar_bind_to_gaction_state	(TeplGotoLineBar *bar,
									 GAction         *action);

G_END_DECLS

#endif /* TEPL_GOTO_LINE_BAR_H */
