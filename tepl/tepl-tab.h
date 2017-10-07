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

/**
 * TeplTabClass:
 * @parent_class: The parent class.
 * @pack_view: Virtual function pointer to add the #TeplView in the #TeplTab
 *   container. Called only once at object construction time, when the
 *   #TeplTab:view property is set. By default the #TeplView is added to a
 *   #GtkScrolledWindow and the #GtkScrolledWindow is added to the #TeplTab with
 *   gtk_container_add().
 * @pack_info_bar: Virtual function pointer to add a #GtkInfoBar in the #TeplTab
 *   container. By default the #GtkInfoBar is inserted above the first
 *   non-#GtkInfoBar child widget of #TeplTab (so by default it is inserted
 *   below other #GtkInfoBar's, but above the #GtkScrolledWindow containing the
 *   #TeplView).
 * @close_request: For the #TeplTab::close-request signal.
 */
struct _TeplTabClass
{
	GtkGridClass parent_class;

	/* Vfuncs */

	void	(* pack_view)		(TeplTab  *tab,
					 TeplView *view);

	void	(* pack_info_bar)	(TeplTab    *tab,
					 GtkInfoBar *info_bar);

	/* Signals */

	void	(* close_request)	(TeplTab *tab);

	/*< private >*/
	gpointer padding[12];
};

GType		tepl_tab_get_type		(void);

TeplTab *	tepl_tab_new			(void);

TeplTab *	tepl_tab_new_with_view		(TeplView *view);

TeplView *	tepl_tab_get_view		(TeplTab *tab);

TeplBuffer *	tepl_tab_get_buffer		(TeplTab *tab);

void		tepl_tab_add_info_bar		(TeplTab    *tab,
						 GtkInfoBar *info_bar);

void		tepl_tab_load_file		(TeplTab *tab,
						 GFile   *location);

G_END_DECLS

#endif /* TEPL_TAB_H */
