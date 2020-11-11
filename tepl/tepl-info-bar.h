/* SPDX-FileCopyrightText: 2016-2020 - Sébastien Wilmet <swilmet@gnome.org>
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

#define TEPL_TYPE_INFO_BAR             (tepl_info_bar_get_type ())
#define TEPL_INFO_BAR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_INFO_BAR, TeplInfoBar))
#define TEPL_INFO_BAR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_INFO_BAR, TeplInfoBarClass))
#define TEPL_IS_INFO_BAR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_INFO_BAR))
#define TEPL_IS_INFO_BAR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_INFO_BAR))
#define TEPL_INFO_BAR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_INFO_BAR, TeplInfoBarClass))

typedef struct _TeplInfoBar         TeplInfoBar;
typedef struct _TeplInfoBarClass    TeplInfoBarClass;
typedef struct _TeplInfoBarPrivate  TeplInfoBarPrivate;

struct _TeplInfoBar
{
	GtkInfoBar parent;

	TeplInfoBarPrivate *priv;
};

struct _TeplInfoBarClass
{
	GtkInfoBarClass parent_class;

	gpointer padding[12];
};

/**
 * TeplInfoBarLocation:
 * @TEPL_INFO_BAR_LOCATION_ALONGSIDE_ICON: on the right side of the icon.
 * @TEPL_INFO_BAR_LOCATION_BELOW_ICON: below the icon.
 *
 * Location inside the content area.
 *
 * The content area of a #TeplInfoBar contains a vertical container containing:
 * - First, an horizontal container containing:
 *   - A place for an optional icon.
 *   - The %TEPL_INFO_BAR_LOCATION_ALONGSIDE_ICON location, which is a vertical
 *     container that can contain: primary/secondary messages plus additional
 *     widgets, in the order that they are added.
 * - The %TEPL_INFO_BAR_LOCATION_BELOW_ICON location, which can contain
 *   additional widgets, in the order that they are added. So the widgets added
 *   here are under the icon and under the
 *   %TEPL_INFO_BAR_LOCATION_ALONGSIDE_ICON location.
 *
 * Since: 6.0
 */
typedef enum
{
	TEPL_INFO_BAR_LOCATION_ALONGSIDE_ICON,
	TEPL_INFO_BAR_LOCATION_BELOW_ICON
} TeplInfoBarLocation;

_TEPL_EXTERN
GType			tepl_info_bar_get_type				(void);

_TEPL_EXTERN
TeplInfoBar *		tepl_info_bar_new				(void);

_TEPL_EXTERN
TeplInfoBar *		tepl_info_bar_new_simple			(GtkMessageType  msg_type,
									 const gchar    *primary_msg,
									 const gchar    *secondary_msg);

_TEPL_EXTERN
gboolean		tepl_info_bar_get_icon_from_message_type	(TeplInfoBar *info_bar);

_TEPL_EXTERN
void			tepl_info_bar_set_icon_from_message_type	(TeplInfoBar *info_bar,
									 gboolean     icon_from_message_type);

_TEPL_EXTERN
const gchar *		tepl_info_bar_get_icon_name			(TeplInfoBar *info_bar);

_TEPL_EXTERN
void			tepl_info_bar_set_icon_name			(TeplInfoBar *info_bar,
									 const gchar *icon_name);

_TEPL_EXTERN
void			tepl_info_bar_add_primary_message		(TeplInfoBar *info_bar,
									 const gchar *primary_msg);

_TEPL_EXTERN
void			tepl_info_bar_add_secondary_message		(TeplInfoBar *info_bar,
									 const gchar *secondary_msg);

_TEPL_EXTERN
void			tepl_info_bar_add_content_widget		(TeplInfoBar         *info_bar,
									 GtkWidget           *widget,
									 TeplInfoBarLocation  location);

_TEPL_EXTERN
gboolean		tepl_info_bar_get_handle_close_response		(TeplInfoBar *info_bar);

_TEPL_EXTERN
void			tepl_info_bar_set_handle_close_response		(TeplInfoBar *info_bar,
									 gboolean     handle_close_response);

_TEPL_EXTERN
void			tepl_info_bar_setup_close_button		(TeplInfoBar *info_bar);

_TEPL_EXTERN
void			tepl_info_bar_set_buttons_orientation		(GtkInfoBar     *info_bar,
									 GtkOrientation  buttons_orientation);

_TEPL_EXTERN
GtkLabel *		tepl_info_bar_create_label			(void);

G_GNUC_INTERNAL
void			_tepl_info_bar_set_size_request			(GtkInfoBar *info_bar);

G_END_DECLS

#endif /* TEPL_INFO_BAR_H */
