/* SPDX-FileCopyrightText: 2016 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_IO_ERROR_INFO_BAR_H
#define TEPL_IO_ERROR_INFO_BAR_H

#include <gtksourceview/gtksource.h>
#include "tepl-info-bar.h"

G_BEGIN_DECLS

#define TEPL_TYPE_IO_ERROR_INFO_BAR             (_tepl_io_error_info_bar_get_type ())
#define TEPL_IO_ERROR_INFO_BAR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_IO_ERROR_INFO_BAR, TeplIoErrorInfoBar))
#define TEPL_IO_ERROR_INFO_BAR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_IO_ERROR_INFO_BAR, TeplIoErrorInfoBarClass))
#define TEPL_IS_IO_ERROR_INFO_BAR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_IO_ERROR_INFO_BAR))
#define TEPL_IS_IO_ERROR_INFO_BAR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_IO_ERROR_INFO_BAR))
#define TEPL_IO_ERROR_INFO_BAR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_IO_ERROR_INFO_BAR, TeplIoErrorInfoBarClass))

typedef struct _TeplIoErrorInfoBar         TeplIoErrorInfoBar;
typedef struct _TeplIoErrorInfoBarClass    TeplIoErrorInfoBarClass;
typedef struct _TeplIoErrorInfoBarPrivate  TeplIoErrorInfoBarPrivate;

struct _TeplIoErrorInfoBar
{
	TeplInfoBar parent;

	TeplIoErrorInfoBarPrivate *priv;
};

struct _TeplIoErrorInfoBarClass
{
	TeplInfoBarClass parent_class;
};

G_GNUC_INTERNAL
GType			_tepl_io_error_info_bar_get_type			(void);

G_GNUC_INTERNAL
TeplIoErrorInfoBar *	_tepl_io_error_info_bar_new				(void);

G_GNUC_INTERNAL
void			_tepl_io_error_info_bar_set_loading_error		(TeplIoErrorInfoBar  *info_bar,
										 GtkSourceFileLoader *loader,
										 const GError        *error);

G_END_DECLS

#endif /* TEPL_IO_ERROR_INFO_BAR_H */
