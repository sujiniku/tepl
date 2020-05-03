/* Copyright 2016-2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_INFO_BAR_H
#define TEPL_INFO_BAR_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <tepl/tepl-macros.h>

G_BEGIN_DECLS

#define TEPL_TYPE_INFO_BAR (tepl_info_bar_get_type ())
_TEPL_EXTERN
G_DECLARE_DERIVABLE_TYPE (TeplInfoBar, tepl_info_bar,
			  TEPL, INFO_BAR,
			  GtkInfoBar)

struct _TeplInfoBarClass
{
	GtkInfoBarClass parent_class;

	gpointer padding[12];
};

_TEPL_EXTERN
TeplInfoBar *		tepl_info_bar_new				(void);

_TEPL_EXTERN
TeplInfoBar *		tepl_info_bar_new_simple			(GtkMessageType  msg_type,
									 const gchar    *primary_msg,
									 const gchar    *secondary_msg);

_TEPL_EXTERN
void			tepl_info_bar_add_icon				(TeplInfoBar *info_bar);

_TEPL_EXTERN
void			tepl_info_bar_add_primary_message		(TeplInfoBar *info_bar,
									 const gchar *primary_msg);

_TEPL_EXTERN
void			tepl_info_bar_add_secondary_message		(TeplInfoBar *info_bar,
									 const gchar *secondary_msg);

_TEPL_EXTERN
void			tepl_info_bar_add_content_widget		(TeplInfoBar *info_bar,
									 GtkWidget   *content);

_TEPL_EXTERN
void			tepl_info_bar_add_close_button			(TeplInfoBar *info_bar);

_TEPL_EXTERN
void			tepl_info_bar_set_buttons_orientation		(TeplInfoBar    *info_bar,
									 GtkOrientation  buttons_orientation);

_TEPL_EXTERN
GtkLabel *		tepl_info_bar_create_label			(void);

G_GNUC_INTERNAL
void			_tepl_info_bar_set_size_request			(GtkInfoBar *info_bar);

G_END_DECLS

#endif /* TEPL_INFO_BAR_H */
