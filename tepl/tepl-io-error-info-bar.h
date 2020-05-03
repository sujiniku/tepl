/* Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_IO_ERROR_INFO_BAR_H
#define TEPL_IO_ERROR_INFO_BAR_H

#include <gtksourceview/gtksource.h>
#include "tepl-info-bar.h"

G_BEGIN_DECLS

#define TEPL_TYPE_IO_ERROR_INFO_BAR (_tepl_io_error_info_bar_get_type ())
G_DECLARE_DERIVABLE_TYPE (TeplIoErrorInfoBar, _tepl_io_error_info_bar,
			  TEPL, IO_ERROR_INFO_BAR,
			  TeplInfoBar)

struct _TeplIoErrorInfoBarClass
{
	TeplInfoBarClass parent_class;
};

G_GNUC_INTERNAL
TeplIoErrorInfoBar *	_tepl_io_error_info_bar_new				(void);

G_GNUC_INTERNAL
void			_tepl_io_error_info_bar_set_loading_error		(TeplIoErrorInfoBar  *info_bar,
										 GtkSourceFileLoader *loader,
										 const GError        *error);

G_END_DECLS

#endif /* TEPL_IO_ERROR_INFO_BAR_H */
