/* SPDX-FileCopyrightText: 2017 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_TAB_LABEL_H
#define TEPL_TAB_LABEL_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <tepl/tepl-tab.h>

G_BEGIN_DECLS

#define TEPL_TYPE_TAB_LABEL             (tepl_tab_label_get_type ())
#define TEPL_TAB_LABEL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_TAB_LABEL, TeplTabLabel))
#define TEPL_TAB_LABEL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_TAB_LABEL, TeplTabLabelClass))
#define TEPL_IS_TAB_LABEL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_TAB_LABEL))
#define TEPL_IS_TAB_LABEL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_TAB_LABEL))
#define TEPL_TAB_LABEL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_TAB_LABEL, TeplTabLabelClass))

typedef struct _TeplTabLabel         TeplTabLabel;
typedef struct _TeplTabLabelClass    TeplTabLabelClass;
typedef struct _TeplTabLabelPrivate  TeplTabLabelPrivate;

struct _TeplTabLabel
{
	GtkGrid parent;

	TeplTabLabelPrivate *priv;
};

/**
 * TeplTabLabelClass:
 * @parent_class: The parent class.
 * @get_tooltip_markup: Virtual function pointer to create the tooltip markup
 *   string. %NULL must be returned if no tooltip is wanted. The result is
 *   intended to be used as an argument to gtk_widget_set_tooltip_markup(). The
 *   default implementation returns the full #TeplFile:location if non-%NULL, or
 *   %NULL otherwise. The return value must be freed with g_free() when no
 *   longer needed.
 */
struct _TeplTabLabelClass
{
	GtkGridClass parent_class;

	gchar *	(* get_tooltip_markup)	(TeplTabLabel *tab_label);

	/*< private >*/
	gpointer padding[12];
};

_TEPL_EXTERN
GType		tepl_tab_label_get_type		(void);

_TEPL_EXTERN
GtkWidget *	tepl_tab_label_new		(TeplTab *tab);

_TEPL_EXTERN
TeplTab *	tepl_tab_label_get_tab		(TeplTabLabel *tab_label);

_TEPL_EXTERN
void		tepl_tab_label_update_tooltip	(TeplTabLabel *tab_label);

G_END_DECLS

#endif /* TEPL_TAB_LABEL_H */
