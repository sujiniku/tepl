/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
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

_TEPL_EXTERN
GType		tepl_statusbar_get_type			(void);

_TEPL_EXTERN
TeplStatusbar *	tepl_statusbar_new			(void);

_TEPL_EXTERN
void		tepl_statusbar_show_cursor_position	(TeplStatusbar *statusbar,
							 gint           line,
							 gint           column);

_TEPL_EXTERN
void		tepl_statusbar_hide_cursor_position	(TeplStatusbar *statusbar);

_TEPL_EXTERN
void		tepl_statusbar_set_tab_group		(TeplStatusbar *statusbar,
							 TeplTabGroup  *tab_group);

G_END_DECLS

#endif /* TEPL_STATUSBAR_H */
