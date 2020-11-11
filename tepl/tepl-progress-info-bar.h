/* SPDX-FileCopyrightText: 2005 - Paolo Maggi
 * SPDX-FileCopyrightText: 2016, 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_PROGRESS_INFO_BAR_H
#define TEPL_PROGRESS_INFO_BAR_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <tepl/tepl-info-bar.h>

G_BEGIN_DECLS

#define TEPL_TYPE_PROGRESS_INFO_BAR             (tepl_progress_info_bar_get_type ())
#define TEPL_PROGRESS_INFO_BAR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_PROGRESS_INFO_BAR, TeplProgressInfoBar))
#define TEPL_PROGRESS_INFO_BAR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_PROGRESS_INFO_BAR, TeplProgressInfoBarClass))
#define TEPL_IS_PROGRESS_INFO_BAR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_PROGRESS_INFO_BAR))
#define TEPL_IS_PROGRESS_INFO_BAR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_PROGRESS_INFO_BAR))
#define TEPL_PROGRESS_INFO_BAR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_PROGRESS_INFO_BAR, TeplProgressInfoBarClass))

typedef struct _TeplProgressInfoBar         TeplProgressInfoBar;
typedef struct _TeplProgressInfoBarClass    TeplProgressInfoBarClass;
typedef struct _TeplProgressInfoBarPrivate  TeplProgressInfoBarPrivate;

struct _TeplProgressInfoBar
{
	TeplInfoBar parent;

	TeplProgressInfoBarPrivate *priv;
};

struct _TeplProgressInfoBarClass
{
	TeplInfoBarClass parent_class;

	gpointer padding[12];
};

_TEPL_EXTERN
GType			tepl_progress_info_bar_get_type		(void);

_TEPL_EXTERN
TeplProgressInfoBar *	tepl_progress_info_bar_new		(const gchar *icon_name,
								 const gchar *markup,
								 gboolean     has_cancel_button);

_TEPL_EXTERN
void			tepl_progress_info_bar_set_markup	(TeplProgressInfoBar *info_bar,
								 const gchar         *markup);

_TEPL_EXTERN
void			tepl_progress_info_bar_set_text		(TeplProgressInfoBar *info_bar,
								 const gchar         *text);

_TEPL_EXTERN
void			tepl_progress_info_bar_set_fraction	(TeplProgressInfoBar *info_bar,
								 gdouble              fraction);

_TEPL_EXTERN
void			tepl_progress_info_bar_pulse		(TeplProgressInfoBar *info_bar);

G_END_DECLS

#endif /* TEPL_PROGRESS_INFO_BAR_H */
