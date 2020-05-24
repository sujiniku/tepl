/* SPDX-FileCopyrightText: 2016-2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_TAB_H
#define TEPL_TAB_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <tepl/tepl-buffer.h>
#include <tepl/tepl-view.h>
#include <tepl/tepl-goto-line-bar.h>

G_BEGIN_DECLS

#define TEPL_TYPE_TAB             (tepl_tab_get_type ())
#define TEPL_TAB(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_TAB, TeplTab))
#define TEPL_TAB_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_TAB, TeplTabClass))
#define TEPL_IS_TAB(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_TAB))
#define TEPL_IS_TAB_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_TAB))
#define TEPL_TAB_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_TAB, TeplTabClass))

typedef struct _TeplTab         TeplTab;
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
 *   container. By default the #GtkInfoBar is inserted just above the
 *   #GtkScrolledWindow containing the #TeplView.
 * @pack_goto_line_bar: Virtual function pointer to add a #TeplGotoLineBar in
 *   the #TeplTab container. By default the #TeplGotoLineBar is added at the
 *   bottom.
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

	void	(* pack_goto_line_bar)	(TeplTab         *tab,
					 TeplGotoLineBar *goto_line_bar);

	/* Signals */

	void	(* close_request)	(TeplTab *tab);

	/*< private >*/
	gpointer padding[12];
};

_TEPL_EXTERN
GType		tepl_tab_get_type		(void);

_TEPL_EXTERN
TeplTab *	tepl_tab_new			(void);

_TEPL_EXTERN
TeplTab *	tepl_tab_new_with_view		(TeplView *view);

_TEPL_EXTERN
TeplView *	tepl_tab_get_view		(TeplTab *tab);

_TEPL_EXTERN
TeplBuffer *	tepl_tab_get_buffer		(TeplTab *tab);

_TEPL_EXTERN
TeplGotoLineBar *tepl_tab_get_goto_line_bar	(TeplTab *tab);

_TEPL_EXTERN
void		tepl_tab_add_info_bar		(TeplTab    *tab,
						 GtkInfoBar *info_bar);

G_END_DECLS

#endif /* TEPL_TAB_H */
